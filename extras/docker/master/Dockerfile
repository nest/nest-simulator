# Pull base image.
FROM ubuntu:18.04

MAINTAINER "Steffen Graber" <s.graber@fz-juelich.de>

ENV TERM=xterm \
    TZ=Europe/Berlin \
    DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
        software-properties-common \
        tzdata \
        locales \
        ssh \
        curl \
        wget \
        git \
        unzip \
        build-essential \
        autoconf \
        cmake \
        libltdl-dev \
        libreadline6-dev \
        libncurses5-dev \
        libgsl-dev \
        openmpi-bin \
        libopenmpi-dev \
        vera++ \
        pep8 \
        libpcre3 \
        libpcre3-dev \
        jq \
        libmusic1v5 \
        music-bin \
        libmusic-dev \
        python-dev \
        python3-dev \
        python3-setuptools \
        python3-path \
        python3-statsmodels \
        python3-jupyter-core \
        python3-mpi4py \
        python3-numpy \
        python3-pandas \
        python3-scipy \
        jupyter \
        jupyter-notebook \
        cython3  && \
        apt-get autoremove -y && \
        rm -rf /var/lib/apt/lists/* && \
        locale-gen en_US.UTF-8

ENV LANG='en_US.UTF-8' LANGUAGE='en_US:en' LC_ALL='en_US.UTF-8'

RUN cd /tmp && \
    git clone https://github.com/INCF/libneurosim.git libneurosim && \
    cd /tmp/libneurosim && \
    chmod +x autogen.sh && \
    ./autogen.sh && \
    chmod +x configure && \
    ./configure && make &&  make install &&\
    rm -rf /tmp/*

WORKDIR /home/nest/
