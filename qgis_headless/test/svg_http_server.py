from http.server import HTTPServer, SimpleHTTPRequestHandler


class SvgRequestHandler(SimpleHTTPRequestHandler):
    def log_message(self, format, *args):
        # Keep the renderer's stdout/stderr assertions independent of HTTP logs.
        pass


if __name__ == "__main__":
    with HTTPServer(("127.0.0.1", 0), SvgRequestHandler) as server:
        print(server.server_port, flush=True)
        server.serve_forever()
