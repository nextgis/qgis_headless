# QGIS Headless Rendering

Headless rendering library using QGIS

## Installation to Virtualenv

1. Install dependecies:

   * `QGIS` >= 3.44
   * `libqgis-dev`
   * `qt5-image-formats-plugins` (usually installed via dependencies)
   * `build-essential`
   * `cmake`

2. Create virtualenv, activate it and install the package there:

    ``` shell
    $ python3 -m venv env
    $ source env/bin/activate
    $ pip install --editable ./[dev,tests]
    ```

3. Run the tests:

    ``` shell
    $ python -m pytest qgis_headless/test/
    ```

## Running Python Tests in Docker

Ubuntu Jammy (for example):

``` shell
$ docker build -t qgis_headless:resolute -f Dockerfile.resolute .
$ docker run --rm qgis_headless:resolute ./pytest-runner
```

## Developer Notes

### Process shutdown

Call `qgis_headless.deinit()` while the application's initialization thread is
still alive. Native layers, cached style layers, CRS values and render requests
retain the QGIS application until their destructors finish. If any remain alive,
`deinit()` defers cleanup until the last resource is released; there is no need
to force Python garbage collection. Repeated calls to `deinit()` are harmless.
Do not reinitialize QGIS while resources from a previous initialization remain
alive.

Some embedded Python hosts, including uWSGI, run Python `atexit` callbacks after
C++ thread-local destructors. At that point QGIS cleanup is unsafe, so `deinit()`
does nothing and leaves the remaining process resources to the operating system.
Other Python `atexit` callbacks still run normally. If cleanup must flush settings
or other persistent state, call `deinit()` explicitly before process teardown.

### Code quality

This project uses `pre-commit` to enforce code quality standards.

First, make sure `pre-commit` is installed:

```shell
$ pip install pre-commit
```

Then, enable it in the project by running:

``` shell
$ pre-commit install
```
