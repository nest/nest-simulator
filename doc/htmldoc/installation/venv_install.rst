.. _venv:

Install from source in a ``venv`` environment
=============================================

* Install base packages on your system:

.. code-block:: bash

     sudo apt install \
     cmake \
     libltdl-dev \
     libboost-filesystem-dev \
     libboost-regex-dev \
     libboost-wave-dev \
     libboost-python-dev \
     libboost-program-options-dev \
     libboost-test-dev \
     openmpi-bin \
     libopenmpi-dev \
     libgsl0-dev \
     pandoc

* Optional packages

.. code-block:: bash

     # for SONATA compatiblity
     libhdf5-dev
     # for MUSIC compatibility
     libmusic-dev \
     music-bin

* Create and activate a ``venv`` environment:

.. code-block:: bash

    python -m venv venv
    source venv/bin/activate

* Install Python packages (see `requirements.txt <https://github.com/nest/nest-simulator/blob/main/requirements.txt>`_ for details):

.. code-block:: bash

     pip install -r requirements.txt # installs all dependencies for PyNEST, testing, docs, and nest-server


* Create a build directory:

.. code-block:: bash

    mkdir build_dir # Build directory should be placed outside of source repository

* Change to the build directory:

.. code-block:: bash

    cd build_dir


* Build NEST:

  You may need additional ``cmake`` options (see :ref:`cmake_options`). Note that you should *not* give `CMAKE_INSTALL_PREFIX` to have NEST installed into the venv correctly.

.. code-block:: bash

   cmake </path/to/NEST/src>

* Compile and install NEST:

.. code-block:: bash

    make
    make install
    make installcheck

.. note::

   To build the developer or user documentation see :ref:`doc_workflow`
