# Pull base image.
FROM ubuntu:16.04

RUN apt-get update && apt-get install -y \
    software-properties-common \
    ssh \
    curl \
    wget \
    locales \
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
    libmusic-dev && \
    apt-get autoremove -y

RUN locale-gen en_US.UTF-8
ENV LANG='en_US.UTF-8' LANGUAGE='en_US:en' LC_ALL='en_US.UTF-8'

# Install miniconda and create Python 3 environment
RUN wget https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh && \
bash ./Miniconda3-latest-Linux-x86_64.sh -b -p /home/nest/miniconda  && \
echo ". /home/nest/miniconda/bin:$PATH" >> /home/nest/.bashrc
ENV PATH=/home/nest/miniconda/bin:${PATH}
RUN conda create --name py3 python=3 && \
    conda update -y conda && \
    /bin/bash -c "source activate py3" && \
    conda install -y setuptools path.py cython statsmodels jupyter mpi4py pandas numpy

# Install libneurosim (/usr/local/lib)
WORKDIR /tmp
RUN git clone https://github.com/INCF/libneurosim.git libneurosim
WORKDIR /tmp/libneurosim
RUN chmod +x autogen.sh
RUN ./autogen.sh && \
    chmod +x configure && \
    ./configure && make &&  make install

EXPOSE 8080
RUN apt-get autoremove && rm -rf /tmp/*

WORKDIR /home/nest/
