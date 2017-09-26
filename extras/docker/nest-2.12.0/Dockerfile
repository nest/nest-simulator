FROM nest/docker-master:latest

ARG WITH_MPI
ARG WITH_GSL
ARG WITH_MUSIC
ARG WITH_LIBNEUROSIM

# add user 'nest'
RUN adduser --disabled-login --gecos 'NEST' --home /home/nest nest

# Install NEST 2.12.0
RUN mkdir data
RUN chown -R nest:nest /home/nest
WORKDIR /home/nest
RUN su nest -c 'wget https://github.com/nest/nest-simulator/releases/download/v2.12.0/nest-2.12.0.tar.gz'
RUN su nest -c 'tar -xvzf nest-2.12.0.tar.gz'
RUN su nest -c 'mkdir /home/nest/nest-build'
RUN su nest -c 'mkdir /home/nest/nest-install'
WORKDIR /home/nest/nest-build
RUN su nest -c 'cmake -DCMAKE_INSTALL_PREFIX:PATH=/home/nest/nest-install \
    -Dwith-python=3 \
    -Dwith-mpi:BOOL=$WITH_MPI \
    -Dwith-gsl:BOOL=$WITH_GSL /usr/local/lib \
    -Dwith-libneurosim:BOOL=$WITH_LIBNEUROSIM /usr/local/lib \
    -Dwith-music=$WITH_MUSIC /usr/local/lib ../nest-2.12.0'
RUN su nest -c 'make'
RUN su nest -c 'make install'
# RUN su nest -c 'make installcheck'

WORKDIR /home/nest/

# Load NEST with every start.
RUN su nest -c "echo '. /home/nest/nest-install/bin/nest_vars.sh' >> /home/nest/.bashrc"

RUN rm -rf /home/nest/nest-build
RUN rm /home/nest/nest-2.12.0.tar.gz

WORKDIR /home/nest/data
EXPOSE 8080
COPY ./entrypoint.sh /home/nest/
RUN chown nest:nest /home/nest/entrypoint.sh
RUN chmod +x /home/nest/entrypoint.sh
ENTRYPOINT ["/home/nest/entrypoint.sh"]
