# The NEST 2.16.0 Dockerfile

The 'entrypoint.sh' enables the use of different mode. 

    docker build \
            --build-arg WITH_MPI=On \
            --build-arg WITH_GSL=On \
            --build-arg WITH_MUSIC=On \
            --build-arg WITH_LIBNEUROSIM=On \
            -t nest/docker-nest-2.16.0 .          
 