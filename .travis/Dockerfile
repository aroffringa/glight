FROM ubuntu:18.04

RUN apt-get update && \
    apt-get install -y \
    g++ \
    cmake \
    pkg-config \
    libboost-filesystem-dev \
    libboost-system-dev \
    libboost-test-dev \
    libgtkmm-3.0-dev \
    libaubio-dev \
    libflac++-dev \
    libasound2-dev \
    libola-dev libprotobuf-dev \
    libxml2-dev

ADD . /src
WORKDIR /src

RUN mkdir /build && cd /build && cmake ../src && make -j2 && make install && make check
