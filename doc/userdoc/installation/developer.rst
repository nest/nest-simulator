.. _dev_install:

Developer installation instructions
-----------------------------------


Compile NEST from source code:

*  Download the source code for the  `current release <https://github.com/nest/nest-simulator/releases>`_.

* Unpack the tarball.

.. code-block::

    tar -xzvf nest-simulator-x.y.z.tar.gz

* Or Fork NEST and clone the repository on `GitHub <https://github.com/nest/nest-simulator>`_.
  See details on :ref:`GitHub workflows here <git_workflow>`.

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




Developer tools
~~~~~~~~~~~~~~~

A complete list of required packages to install NEST, run tests, and  build documentation, is available in the
``environment.yml`` file  on the `NEST simulator GitHub page <https://github.com/nest/nest-simulator>`_.

Check out all the :ref:`development workflows here <developer_space>`.


