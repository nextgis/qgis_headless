# QGIS Headless Rendering

Headless rendering library using QGIS

## Installation to Virtualenv

1. Install dependecies:

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
$ docker build -t qgis_headless:jammy -f Dockerfile.jammy .
$ docker run --rm qgis_headless:jammy ./pytest-runner
```

## Developer Notes

This project uses `pre-commit` to enforce code quality standards.

First, make sure `pre-commit` is installed:

```shell
$ pip install pre-commit
```

Then, enable it in the project by running:

``` shell
$ pre-commit install
```
