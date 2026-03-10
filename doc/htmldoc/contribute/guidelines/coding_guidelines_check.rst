.. _required_dev_tools:

Required development tools
==========================

Here, we list required tools for NEST development and explain their usage. The
tools are mostly for formatting your code. Before you get started, please take
a look at our :ref:`detailed guidelines for C++ coding in NEST <code_style_cpp>`.

Development environment
-----------------------

We have provided an `environment.yml <https://github.com/nest/nest-simulator/blob/master/environment.yml>`_
file that contains all packages to do development in NEST, including the tools listed below.

See our :ref:`instructions on installing NEST from source <dev_install>`.

Tooling
-------

pre-commit
~~~~~~~~~~

We use `pre-commit <https://pre-commit.com/>`_ to run Git hooks on every commit to identify simple issues such as
trailing whitespace or not complying with the required formatting. Our ``pre-commit`` configuration is
specified in the `.pre-commit-config.yaml <https://github.com/nest/nest-simulator/blob/master/.pre-commit-config.yaml>`_
file.

To set up the Git hook scripts specified in ``.pre-commit-config.yaml``, run

.. code-block:: bash

   pre-commit install


.. note::

   If ``pre-commit`` identifies formatting issues in the commited code, the ``pre-commit`` Git hooks will reformat
   the code. If code is reformatted, it will show up in your unstaged changes. Stage them and recommit to
   successfully commit your code.

Black
~~~~~

We enforce `PEP8 <https://www.python.org/dev/peps/pep-0008/>`_ formatting of Python code by using the uncompromising
`Black <https://github.com/psf/black>`_ formatter.

``Black`` is run automatically with ``pre-commit``.

Run ``Black`` manually with

.. code-block:: bash

   black .

isort
~~~~~

We use `isort <https://github.com/PyCQA/isort>`_ to sort imports in Python code.

``isort`` is run automatically with ``pre-commit``.

Run ``isort`` manually with

.. code-block:: bash

   isort --profile=black --thirdparty="nest" .

clang-format
~~~~~~~~~~~~

We use `clang-format <http://clang.llvm.org/docs/ClangFormat.html>`_ to format C/C++ code.
Our ``clang-format`` configuration is specified in the
`.clang-format <https://github.com/nest/nest-simulator/blob/master/.clang-format>`_ file.

``clang-format`` is run automatically with ``pre-commit``.

We supply the
`build_support/format_all_c_c++_files.sh <https://github.com/nest/nest-simulator/blob/master/build_support/format_all_c_c++_files.sh>`_
shell script to run ``clang-format`` manually:

.. code-block:: bash

   ./build_support/format_all_c_c++_files.sh [start folder, defaults to '$PWD']

.. note::

   We use ``clang-format`` version 17.0.4 in our CI. If your ``clang-format`` executable is
   not version 17, you need to specify an executable with version 17.0.4 explicitly with
   the `--clang-format` option to ensure consistency with the NEST CI.

Local static analysis
---------------------

We have several static code analyzers in the GitHub Actions CI. To run static code checks locally,
please refer to the "run" lines in the GitHub Actions CI definition at
https://github.com/nest/nest-simulator/blob/master/.github/workflows/nestbuildmatrix.yml.
