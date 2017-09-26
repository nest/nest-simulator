# Docker image for the NEST simulator

The master dockerfile [./master/Dockerfile]() builds an image with a basic 
shell environment, Python 3 (and 2), OpenMPI, matplotlib, Scipy, MUSIC, 
libneurosim and Jupyter Notebook.

The NEST 2.12.0 dockerfile [./nest-2.12.0/Dockerfile]() use the master image 
and integrates [NEST 2.12.0](https://github.com/nest/nest-simulator). 
NEST is build with python3. If you want to change this, modify the Dockerfile.

You need a working docker environment. (https://docs.docker.com/)

## 1 - 2 - 3

Three simple steps to get started.

### 1 - Build the master image
    
    docker build -t nest/docker-master ./master

### 2 - Build the NEST image
        
    # For NEST 2.12.0
    docker build \
        --build-arg WITH_MPI=On \
        --build-arg WITH_GSL=On \
        --build-arg WITH_MUSIC=On \
        --build-arg WITH_LIBNEUROSIM=On \
        -t nest/docker-nest-2.12.0 ./nest-2.12.0

For other/more configuration options please change the 'Dockerfile'. See:
<https://github.com/nest/nest-simulator/blob/v2.12.0/README.md> 
    
### 3 - Run and use it
    
-   with Jupyter Notebook

        docker run -it --rm --user nest --name my_app \
            -v ~/YOURPYFOLDER:/home/nest/data \
            -p 8080:8080 nest/docker-nest-2.12.0 notebook
    
    Open the displayed URL in your browser and have fun with Jupyter 
    Notebook and NEST.
    
-   in interactive mode

        docker run -it --rm --user nest --name my_app \
            -v ~/YOURPYFOLDER:/home/nest/data \
            -p 8080:8080 nest/docker-nest-2.12.0 interactive

    After the prompt 'Your python script:' enter the filename of the script 
    you want to start. Only the filename without any path. Be sure to enter 
    the right 'YOURFOLDER' - the folder containing your scripts.

-   as virtual image
    
        docker run -it --rm --user nest --name my_app \
            -v ~/YOURPYFOLDER:/home/nest/data \
            -p 8080:8080 nest/docker-nest-2.12.0 /bin/bash
    
    You are logged in as user 'nest'. Enter 'python3' and in the 
    python-shell 'import nest'. A 'nest.help()' should display the main 
    help page.

## Docker 

-   Delete ALL images (use with caution)

        docker rmi $(docker images -a -q)

-   Export a docker image

        docker save nest/docker-nest-2.12.0 | gzip -c > nest-docker.tar.gz

-   Import a docker image

        gunzip -c nest-docker.tar.gz | docker load