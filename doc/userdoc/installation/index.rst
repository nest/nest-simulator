Install NEST
============

Who are you?

I'm a Linux or mac user who wants to run simulation scripts for NEST on my computer.

::

    pip3 install nest

See sample code

- Jochen - sample with links

Add models

- NESTML

Extend NEST

- NEST extension module

Perform analyses

- Elephant

Run benchmarks

- benchmark docs

.. note::

   For Windows see neurofedora live media | future WSL

.. note::

   Exceptional cases that need other steps?


----

I'm a lecturer who wants to set up a temporary NEST environment for a classroom or workshop to demonstrate neuroscientifc concepts

For a centralized installation workflow where NEST is made available through a port? Participants do no need to install
NEST themselves, but need access NEST Server or .... (requirement mac or Linux - does MS work?)

- Docker installation

- NEST Server?

- Check out  graphical user interface NEST-Desktop, to help visualize and illustrate complex network simulations.


For situations where particpants will need to install NEST, we recommend the compneurolab live media (all operating systems)

----

I'm an administrator of a lab or cluster that requires a semi-permanent or permanent installation of NEST for researchers and students.
I am responsible for deploying NEST on a machine

- For small clusters (what is small?) see docker installation / spack

- For supercomputers and HPC sytstems, please contact us (see also some notes on set up from older versions of HPC systems)



----

I am a developer and wants to contribute to the code or documentation of NEST

- compile from source

- developer workflows / tools

- Run benchmarks



* User installation :ref:`std_install`

* Deployer - (for classrooms) :ref:`container_install`

* Developer installation :ref:`adv_install`


.. _std_install:

Standard installation
---------------------

NEST can be installed via pip.


without openmpi:


 .. code-block:: bash

    pip3 install nest-simulator


with openmpi:


 .. code-block:: bash

    pip3 install nest-simulator

 .. note::


   macOS may need special installation instructions. see here
   for windows, there is native biuld, use docker or virtual machine

.. _container_install:

Docker and live media installation
----------------------------------

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

Live media for virtual machines
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We have live media (.ova) if you want to run NEST in a virtual machine. This option is suitable for Windows users, since we don't support
NEST natively on Windows,

:ref:`Download the live media here <download_livemedia>`, and follow the :doc:`instructions to set up the virutal machine <livemedia>` .


----

.. _adv_install:

Advanced installation
---------------------

Compile NEST from source code:

*  Download the source code for the  `current release <https://github.com/nest/nest-simulator/releases>_`

* Unpack the tarball

  .. code-block::

    tar -xzvf nest-simulator-x.y.z.tar.gz

* Or Fork NEST and clone the repository on `GitHub <https://github.com/nest/nest-simulator>`_. See details on :doc:`GitHub workflows here <../contribute/development_workflow>`).

If not already installed on your system, the following packages are recommended (see also the Dependencies section)

create a build directory

  .. code-block:: bash

     mkdir build/

change into build directory

  .. code-block:: bash

     cd build/

compile and build NEST:

  .. code-block:: bash

     cmake /source/directory/

See our :doc:`cmake_options` for different configuration options

  .. code-block:: bash

     make
     make install
     make installcheck



Installation for specific machines and clusters
-----------------------------------------------

- Ebrains

- Supercomputer

- some specific mac

:doc:`Instructions for high performance computers <hpc_install>` provides some instructions for certain machines.


Developer tools
---------------

A complete list of required packages to run tests, build documentation, nest-server, NEST
is available in a conda yaml file.

See development workflows here


----

If installation didn't work, see the :doc:`troubleshooting section <../troubleshooting>`.




.. toctree::
   :hidden:

   linux_install
   mac_install
   hpc_install
   livemedia
   cmake_options
