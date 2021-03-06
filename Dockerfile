FROM ubuntu:20.04

# This will make apt-get install without question
ARG DEBIAN_FRONTEND=noninteractive
ENV TZ 'UTC'

RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone && apt-get update \
  && apt-get install -y \
    build-essential bash cmake libqgis-dev

COPY . /root/qgis_headless/

WORKDIR /root/qgis_headless/build

RUN /bin/bash -c "echo \"export QT_QPA_PLATFORM='offscreen'\" | tee /etc/profile.d/QT_QPA.sh && \
    source /etc/profile && \
    cmake .. -DCMAKE_INSTALL_PREFIX=install -DBUILD_SHARED_LIBS=ON -DENABLE_TESTS=ON && \
    cmake --build . --target install --config Release && \
    ctest --config Release"
