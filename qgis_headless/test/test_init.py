import os
import sys
from contextlib import contextmanager
from subprocess import PIPE, Popen
from time import perf_counter

import psutil
import pytest

CODE = """
print("PYTHON");
input(); import qgis_headless; print("IMPORT");
input(); qgis_headless.init([]); print("INIT");
input()"""


def timer():
    return perf_counter() - timer_frozen_for


@contextmanager
def unfreeze():
    global timer_frozen_at, timer_frozen_for

    assert timer_frozen_at is not None
    timer_frozen_for += perf_counter() - timer_frozen_at
    timer_frozen_at = None

    try:
        yield
    finally:
        assert timer_frozen_at is None
        timer_frozen_at = perf_counter()


timer_frozen_at = perf_counter()
timer_frozen_for = 0.0


@pytest.mark.benchmark(group="init", timer=timer)
@pytest.mark.parametrize(
    "providers",
    [
        pytest.param(True, id="all_providers"),
        pytest.param(False, id="no_providers"),
    ],
)
def test_init(benchmark, providers):
    reports = dict[str, dict]()

    def _measure():
        proc = Popen(
            [sys.executable, "-c", CODE],
            stdin=PIPE,
            stdout=PIPE,
            text=True,
            env=dict(os.environ, QGIS_PROVIDER_FILE=".*" if providers else "$^"),
        )

        assert proc.stdin is not None and proc.stdout is not None

        def _step(
            name: str | None = None,
            write: bool = True,
            pstdin=proc.stdin,
            pstdout=proc.stdout,
        ):
            if write:
                pstdin.write((name or "") + "\n")
                pstdin.flush()

            if name is not None:
                while not (line := pstdout.readline().strip()):
                    pass
                assert line == name
            else:
                proc.wait(0.1)
                assert proc.returncode == 0

        def _report(name: str, pid=proc.pid):
            proc_util = psutil.Process(pid)
            reports[name] = dict(
                rss=round(proc_util.memory_info().rss / (1 << 20), 1),
                threads=proc_util.num_threads(),
                cpu_time=round(1000 * sum(proc_util.cpu_times()), 1),
            )

        _step("PYTHON", write=False)

        _step("IMPORT")
        _report("IMPORT")

        with unfreeze():
            _step("INIT")

        _report("INIT")
        _step(None)

    benchmark.pedantic(_measure, warmup_rounds=5, rounds=10)
    print(repr(reports), file=sys.stderr)
