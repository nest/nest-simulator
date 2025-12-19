.. _venv:

Install from source in a ``venv`` environment
=============================================

* Create and activate a ``venv`` environment:

.. code-block:: bash

    python -m venv nest_venv
    source nest_venv/bin/activate


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

* Install Python packages (see `requirements.txt <https://github.com/nest/nest-simulator/blob/master/requirements.txt>`_ for details):

.. code-block:: bash

     pip install -r requirements.txt # installs all dependencies for PyNEST, testing, docs, and nest-server


* Create a build directory:

.. code-block:: bash

    mkdir build_dir # Build directory should be placed outside of source repository

* Change to the build directory:

.. code-block:: bash

    cd build_dir


* Build NEST:

  You may need additional ``cmake`` options (see :ref:`cmake_options`).

.. code-block:: bash

   cmake </path/to/NEST/src>

* Compile and install NEST:

.. code-block:: bash

    make
    make install
    make installcheck

For your convenience, a shell script setting all required environment variables is provided in
``<nest_install_dir>/bin/nest_vars.sh``. Setting the environment variables in your active shell session requires
sourcing the script:

.. code-block:: bash

   source <nest_install_dir>/bin/nest_vars.sh



.. note::

   To build the developer or user documentation see :ref:`doc_workflow`
