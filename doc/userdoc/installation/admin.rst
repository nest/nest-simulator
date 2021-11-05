.. _admin_install:

Administrator installation instructions
---------------------------------------

If you need to deploy NEST on a machine

- For small clusters (what is small?) see docker installation / spack

Docker container
~~~~~~~~~~~~~~~~

With open-mpi + jupyter notebook
Without open-mpi + jupyter notebook ...

install docker-compose (docker desktop for windows, mac?)

 .. code-block:: bash

   docker-compose up

You can check for updates to the Docker build by typing:

 .. code-block:: bash

    docker pull nestsim/nest:<version>

See the `README <https://github.com/nest/nest-docker>`_ to find out more, but note some functionality, such as DISPLAY, will not be available.


- For supercomputers and other HPC sytstems, please contact us (see also some notes on set up from older versions of HPC systems)
:doc:`Instructions for high performance computers <hpc_install>` provides some instructions for certain machines.


EBRAINS set up?
