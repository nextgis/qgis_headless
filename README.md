# QGIS Headless Rendering

Headless rendering library using QGIS

## Installation to Virtualenv

1. Install dependecies: 

   * `libqgis-dev`
   * `qt5-image-formats-plugins` (usually installed via dependencies)
   * `build-essential`
   * `cmake`

2. Create virtualenv, activate it and install the package there:

    ```
    $ python3 -m venv env
    $ source env/bin/activate
    $ pip install --editable ./[tests]
    ```

3. Run the tests:

    ```
    $ python -m pytest qgis_headless/test/
    ```


## Running Python Tests in Docker

Ubuntu Focal Fossa:

    $ docker build -t qgis_headless:focal -f Dockerfile.focal .
    $ docker run --rm qgis_headless:focal ./pytest-runner

Ubuntu Bionic Beaver:

    $ docker build -t qgis_headless:bionic -f Dockerfile.bionic .
    $ docker run --rm qgis_headless:bionic ./pytest-runner

