# Docker image for the NEST simulator

## Usage

    nest_docker.sh [--help] <command> [<args>] [<version>]
    
    --help      print this usage information.
    <command>   can be either 'provision', 'run' or 'clean'.
    [<args>]    can be either 'notebook', 'interactice' or 'virtual'.
    [<version>] version number of NEST (e.g. 2.12.0 or 2.14.0).
    
    Example:    sh nest-docker.sh provision 2.14.0
                sh nest-docker.sh run notebook 2.14.0
                

## In short

The master dockerfile [./master/Dockerfile]() builds an image with a basic 
shell environment, Python 3, OpenMPI, matplotlib, Scipy, MUSIC, 
libneurosim and Jupyter Notebook.

The NEST dockerfiles use the master image and integrates [NEST](https://github.com/nest/nest-simulator). 

You need a working docker environment. (https://docs.docker.com/)

## Getting the repository

    git clone https://github.com/nest/nest-simulator.git
    cd nest-simulator/extras/docker
    

## 1 - 2 (- 3)

In the following, VESRION is the version number of NEST you want to use 
(right now 2.12.0 or 2.14.0).

Two little steps to get started

### 1 - Provisioning
    
    sh nest-docker.sh provision VERSION

(You can adapt some configuration options in nest-docker.sh. For other/more 
configuration options please change the 'Dockerfile'. See:
<https://github.com/nest/nest-simulator/blob/v2.12.0/README.md>) 
    
### 2 - Run
 
-   with Jupyter Notebook

        sh nest-docker.sh run notebook VERSION  
                    
    Open the displayed URL in your browser and have fun with Jupyter 
    Notebook and NEST.
    
-   in interactive mode

        sh nest-docker.sh run interactive VERSION

    After the prompt 'Your python script:' enter the filename of the script 
    you want to start. Only the filename without any path. The file has to 
    be in the path where you start the script. 

-   as virtual image
    
        sh nest-docker.sh run virtual VERSION
        
    You are logged in as user 'nest'. Enter 'python' and in the 
    python-shell 'import nest'. A 'nest.help()' should display the main 
    help page.

### (3) - Delete the NEST Images

    sh nest-docker.sh clean

Be careful. This stops EVERY container and delete the NEST Images.

## Useful Docker commands 

-   Delete ALL images (USE WITH CAUTION!)

        docker system prune -fa --volumes

-   Export a docker image

        docker save nest/docker-nest-2.12.0 | gzip -c > nest-docker.tar.gz

-   Import a docker image

        gunzip -c nest-docker.tar.gz | docker load