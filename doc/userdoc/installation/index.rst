Install NEST
============

Standard installation
---------------------

NEST can be installed

Via pip:

without openmpi:


 .. code-block:: bash

    pip3 install nest-simulator


with openmpi:


 .. code-block:: bash

    pip3 install nest-simulator

.. note::


  macOS may need special installation instructions. see here
  for windows, there is native biuld, use docker or virtual machine

Install NEST using containers and virtual environments
------------------------------------------------------

virtual box
~~~~~~~~~~~

       We have live media (.ova) if you want to run NEST in a virtual machine. This option is suitable for Windows users, since we don't support
       NEST natively on Windows,

       :ref:`Download the live media here <download_livemedia>`, and follow the :doc:`instructions to set up the virutal machine <livemedia>` .


docker containers
~~~~~~~~~~~~~~~~~

With open-mpi + jupyter notebook
Without open-mpi + jupyter notebook ...

install docker-compose (docker desktop for windows, mac?)

 .. code-block:: bash

   docker-compose up

You can check for updates to the Docker build by typing:

 .. code-block:: bash

    docker pull nestsim/nest:<version>

  See the `README <https://github.com/nest/nest-docker>`_ to find out more, but note some functionality, such as DISPLAY, will not be available.

----

Advanced installation
---------------------

Compile NEST from source code:

*  Download the source code for the  `current release <https://github.com/nest/nest-simulator/releases>_`

* Unpack the tarball


* Or Fork NEST and clone the repository on `GitHub <https://github.com/nest/nest-simulator>`_. See details on :doc:`GitHub workflows here <../contribute/development_workflow>`).

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





High performance computer
-------------------------

   :doc:`Instructions for high performance computers <hpc_install>` provides some instructions for certain machines.

Developer tools
---------------

A complete list of required packages to run tests, build documentation, nest-server, NEST
is available in a conda yaml file.

See development workflows here



----

If installation didn't work, see the :doc:`troubleshooting section <../troubleshooting>`.





Alternative installation options
---------------------------------

macOS
~~~~~

* For macOS see :doc:`mac_install`


.. toctree::
   :hidden:

   linux_install
   mac_install
   hpc_install
   livemedia
   cmake_options
