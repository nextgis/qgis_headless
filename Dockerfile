FROM ubuntu:20.04

# This will make apt-get install without question
ARG DEBIAN_FRONTEND=noninteractive
ENV TZ 'UTC'

RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone && apt-get update \
  && apt-get install -y \
    build-essential bash cmake libqgis-dev

COPY . /root/qgis_headless/

WORKDIR /root/qgis_headless/build

RUN cmake .. -DCMAKE_INSTALL_PREFIX=install && \
    cmake --build . --target install --config Release
