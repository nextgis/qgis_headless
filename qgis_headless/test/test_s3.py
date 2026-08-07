from __future__ import annotations

import gc
import re
import threading
from contextlib import contextmanager
from dataclasses import dataclass
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from typing import Dict, Iterator, List, Optional, Type
from urllib.parse import urlsplit

import pytest
from osgeo import gdal

from qgis_headless import Layer

_RANGE_PATTERN = re.compile(r"bytes=(\d+)-(\d*)")


@dataclass(frozen=True)
class _HttpRequest:
    method: str
    path: str
    range_header: Optional[str]


class _RequestRecorder:
    def __init__(self) -> None:
        self._requests: List[_HttpRequest] = []
        self._lock = threading.Lock()

    def append(self, request: _HttpRequest) -> None:
        with self._lock:
            self._requests.append(request)

    def snapshot(self) -> List[_HttpRequest]:
        with self._lock:
            return list(self._requests)


@dataclass(frozen=True)
class _MockS3:
    endpoint: str
    object_path: str
    recorder: _RequestRecorder


def _create_request_handler(
    object_path: str,
    object_data: bytes,
    recorder: _RequestRecorder,
) -> Type[BaseHTTPRequestHandler]:
    class ObjectRequestHandler(BaseHTTPRequestHandler):
        protocol_version = "HTTP/1.1"

        def do_HEAD(self) -> None:
            self._serve_object(include_body=False)

        def do_GET(self) -> None:
            self._serve_object(include_body=True)

        def log_message(self, format_string: str, *args: object) -> None:
            # Suppress standard HTTP server logging during tests.
            return

        def _serve_object(self, include_body: bool) -> None:
            request_path = urlsplit(self.path).path
            range_header = self.headers.get("Range")

            recorder.append(
                _HttpRequest(
                    method=self.command,
                    path=request_path,
                    range_header=range_header,
                )
            )

            if request_path != object_path:
                self.send_error(404)
                return

            file_size = len(object_data)
            start_offset = 0
            end_offset = file_size - 1
            status_code = 200

            if range_header is not None:
                match = _RANGE_PATTERN.fullmatch(range_header)
                if match is None:
                    self._send_range_error(file_size)
                    return

                start_offset = int(match.group(1))
                if match.group(2):
                    end_offset = int(match.group(2))

                end_offset = min(end_offset, file_size - 1)

                if start_offset >= file_size or start_offset > end_offset:
                    self._send_range_error(file_size)
                    return

                status_code = 206

            response_data = object_data[start_offset : end_offset + 1]

            self.send_response(status_code)
            self.send_header("Accept-Ranges", "bytes")
            self.send_header("Content-Type", "image/tiff")
            self.send_header("Content-Length", str(len(response_data)))
            self.send_header("ETag", '"test-cog-etag"')
            self.send_header(
                "Last-Modified",
                "Wed, 01 Jan 2025 00:00:00 GMT",
            )

            if status_code == 206:
                self.send_header(
                    "Content-Range",
                    (f"bytes {start_offset}-{end_offset}/{file_size}"),
                )

            self.send_header("Connection", "close")
            self.end_headers()

            if include_body:
                self.wfile.write(response_data)

        def _send_range_error(self, file_size: int) -> None:
            self.send_response(416)
            self.send_header(
                "Content-Range",
                f"bytes */{file_size}",
            )
            self.send_header("Content-Length", "0")
            self.send_header("Connection", "close")
            self.end_headers()

    return ObjectRequestHandler


@pytest.fixture()
def mock_s3() -> Iterator[_MockS3]:
    cog_path = Path(__file__).parent / "data" / "raster" / "rounds.tif"
    object_data = cog_path.read_bytes()

    object_path = "/test-bucket/cache_test.tif"
    recorder = _RequestRecorder()
    handler_class = _create_request_handler(
        object_path=object_path,
        object_data=object_data,
        recorder=recorder,
    )

    server = ThreadingHTTPServer(
        ("127.0.0.1", 0),
        handler_class,
    )
    server_thread = threading.Thread(
        target=server.serve_forever,
        daemon=True,
    )
    server_thread.start()

    try:
        port = server.server_address[1]
        yield _MockS3(
            endpoint=f"127.0.0.1:{port}",
            object_path=object_path,
            recorder=recorder,
        )
    finally:
        server.shutdown()
        server.server_close()
        server_thread.join()


@contextmanager
def _gdal_config(
    options: Dict[str, str],
) -> Iterator[None]:
    previous_values = {option_name: gdal.GetConfigOption(option_name) for option_name in options}

    try:
        for option_name, option_value in options.items():
            gdal.SetConfigOption(option_name, option_value)

        yield
    finally:
        for option_name, option_value in previous_values.items():
            gdal.SetConfigOption(option_name, option_value)


def _open_and_release_layer(path: str) -> None:
    layer = Layer.from_gdal(path)
    assert layer is not None

    del layer
    gc.collect()


def _header_requests(
    requests: List[_HttpRequest],
    object_path: str,
) -> List[_HttpRequest]:
    return [
        request
        for request in requests
        if request.method == "GET"
        and request.path == object_path
        and (request.range_header is None or request.range_header.startswith("bytes=0-"))
    ]


def _format_requests(requests: List[_HttpRequest]) -> str:
    return "\n".join(
        (f"{request.method} {request.path} Range={request.range_header!r}") for request in requests
    )


def test_vsis3_reuses_cog_header_cache(mock_s3: _MockS3) -> None:
    vsi_path = "/vsis3/test-bucket/cache_test.tif"

    options = {
        "AWS_S3_ENDPOINT": mock_s3.endpoint,
        "AWS_HTTPS": "NO",
        "AWS_VIRTUAL_HOSTING": "FALSE",
        "AWS_NO_SIGN_REQUEST": "YES",
        "GDAL_DISABLE_READDIR_ON_OPEN": "EMPTY_DIR",
        "GDAL_HTTP_MERGE_CONSECUTIVE_RANGES": "YES",
        "GDAL_PAM_ENABLED": "NO",
        "GDAL_INGESTED_BYTES_AT_OPEN": "32768",
        "CPL_VSIL_CURL_ALLOWED_EXTENSIONS": ".tif,.TIF,.tiff",
        "CPL_VSIL_CURL_CACHE_SIZE": "200000000",
        # Disable the per-handle cache to isolate the global curl cache.
        "VSI_CACHE": "FALSE",
    }

    with _gdal_config(options):
        gdal.VSICurlClearCache()

        _open_and_release_layer(vsi_path)

        requests_after_first_open = mock_s3.recorder.snapshot()
        first_header_requests = _header_requests(
            requests_after_first_open,
            mock_s3.object_path,
        )

        assert first_header_requests, (
            "The first layer opening did not request the COG header.\n"
            f"{_format_requests(requests_after_first_open)}"
        )

        _open_and_release_layer(vsi_path)

        requests_after_second_open = mock_s3.recorder.snapshot()
        second_open_requests = requests_after_second_open[len(requests_after_first_open) :]
        repeated_header_requests = _header_requests(
            second_open_requests,
            mock_s3.object_path,
        )

        assert not repeated_header_requests, (
            "The second layer opening downloaded the COG header again.\n"
            f"{_format_requests(second_open_requests)}"
        )

        # Verify that the test can actually detect a cache miss.
        gdal.VSICurlClearCache()
        before_third_open = len(mock_s3.recorder.snapshot())

        _open_and_release_layer(vsi_path)

        third_open_requests = mock_s3.recorder.snapshot()[before_third_open:]

        assert _header_requests(
            third_open_requests,
            mock_s3.object_path,
        ), (
            "Opening after VSICurlClearCache() did not fetch the header.\n"
            f"{_format_requests(third_open_requests)}"
        )
