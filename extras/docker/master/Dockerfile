# Pull base image.
FROM ubuntu:16.04

RUN apt-get update && apt-get install -y \
            software-properties-common \
            ssh \
            python3 \
            python3-dev \
            python \
            python-all-dev \
            python-pip \
            git \
            wget \
            nano \
            build-essential \
            clang-format \
            autoconf \
            cmake \
            sudo \
            libltdl-dev \
            libreadline6-dev \
            libncurses5-dev \
            libgsl-dev \
            libpcre3\
            libpcre3-dev \
            python-scipy \
            openmpi-bin \
            libopenmpi-dev \
            python-mpi4py \
            python-tk \
            python-matplotlib \
            vera++ \
            pep8 \
            libpcre3 \
            libpcre3-dev \
            jq \
            openmpi-bin \
            libopenmpi-dev \
            python3-nose && \

    apt-get autoremove

# Install pip and Cython.
WORKDIR /tmp
RUN wget https://bootstrap.pypa.io/get-pip.py  && \
    python get-pip.py && \
    pip install --upgrade pip && \
    pip install --no-cache-dir --upgrade \
            setuptools \
            path.py \
            cython==0.23.4 \
            statsmodels \
            pandas \
            jupyter


# Install jupyter notebook
#RUN python3 get-pip.py
#RUN pip3 install --upgrade pip
#RUN pip3 install statsmodels
#RUN pip3 install jupyter

# Install current MUSIC version  (/usr/local/lib)
WORKDIR /tmp
RUN git clone https://github.com/INCF/MUSIC.git music
WORKDIR /tmp/music
WORKDIR /tmp/music
RUN chmod +x autogen.sh
RUN ./autogen.sh && \
    chmod +x configure && \
    ./configure && make && make install

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