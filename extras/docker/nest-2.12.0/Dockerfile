FROM nest/docker-master:latest

ARG WITH_MPI
ARG WITH_GSL
ARG WITH_MUSIC
ARG WITH_LIBNEUROSIM

# add user 'nest'
RUN adduser --disabled-login --gecos 'NEST' --home /home/nest nest && \
    adduser nest sudo

# Install NEST 2.14.0
RUN mkdir data && \
    chown -R nest:nest /home/nest

WORKDIR /home/nest

USER nest

RUN wget https://github.com/nest/nest-simulator/releases/download/v2.12.0/nest-2.12.0.tar.gz && \
    tar -xvzf nest-2.12.0.tar.gz && \
    mkdir /home/nest/nest-build && \
    mkdir /home/nest/nest-install

WORKDIR /home/nest/nest-build

#ENV PATH=/home/nest/miniconda/bin:${PATH}
#RUN /bin/bash -c "source activate py3"

# Install NEST 2.12.0

RUN cmake -DCMAKE_INSTALL_PREFIX:PATH=/home/nest/nest-install \
    -Dwith-python=2 \
    -Dwith-mpi:BOOL=$WITH_MPI \
    -Dwith-gsl:BOOL=$WITH_GSL /usr/local/lib \
    -Dwith-libneurosim:BOOL=$WITH_LIBNEUROSIM /usr/local/lib \
    -Dwith-music=$WITH_MUSIC /usr/local/lib ../nest-2.12.0 && \
    make && make install && \
    echo '. /home/nest/nest-install/bin/nest_vars.sh' >> /home/nest/.bashrc

# RUN su nest -c 'make installcheck'

WORKDIR /home/nest/

USER root
RUN rm -rf /home/nest/nest-build && rm /home/nest/nest-2.12.0.tar.gz

WORKDIR /home/nest/data
EXPOSE 8080
COPY ./entrypoint.sh /home/nest/
RUN chown nest:nest /home/nest/entrypoint.sh && \
    chmod +x /home/nest/entrypoint.sh
ENTRYPOINT ["/home/nest/entrypoint.sh"]
