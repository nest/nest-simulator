Developer installation instructions
-----------------------------------

- compile from source

- developer workflows / tools

- Run benchmarks

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




Developer tools
~~~~~~~~~~~~~~~

A complete list of required packages to run tests, build documentation, nest-server, NEST
is available in a conda yaml file.

See development workflows here


