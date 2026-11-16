.. _required_dev_tools:

Required development tools
==========================

Here, we list required tools for NEST development and explain their usage. The
tools are mostly for formatting your code. Before you get started, please take
a look at our `detailed guidelines for C++ coding in NEST <https://nest.github.io/nest-simulator/devdoc_coding_conventions.html>`_.

Development environment
-----------------------

We have provided an `environment.yml <https://github.com/nest/nest-simulator/blob/main/environment.yml>`_
file that contains all packages to do development in NEST, including the tools listed below.

See our :ref:`instructions on installing NEST from source <dev_install>`.

You can also install testing tools only by installing only test-time dependencies.

.. code-block:: bash

   pip install -r requirements_testing.txt


Tooling
-------

pre-commit
~~~~~~~~~~

We use `pre-commit <https://pre-commit.com/>`_ to run Git hooks on every commit to identify simple issues such as
trailing whitespace or not complying with the required formatting. Our ``pre-commit`` configuration is
specified in the `.pre-commit-config.yaml <https://github.com/nest/nest-simulator/blob/main/.pre-commit-config.yaml>`_
file.

To set up the Git hook scripts specified in ``.pre-commit-config.yaml``, run

.. code-block:: bash

   pre-commit install


.. note::

   If ``pre-commit`` identifies formatting issues in the committed code, the ``pre-commit`` Git hooks will reformat
   the code. If code is reformatted, it will show up in your unstaged changes. Stage them and recommit to
   successfully commit your code.

The usual workflow would be to loop ``git add -u; git hook run pre-commit`` until you don't see changes anymore. Then review your finally to-be-commited changes with ``git diff --cached`` and when satisfied, do the commit ``git commit``.

Besides the automatic git commit hook (which checks only the staged files), files can be checked and formatted manually using `pre-commit` directly:

* ``pre-commit run --all-files`` formats all files and
* ``pre-commit run --files file1.cpp file2.cpp`` formats only the specified files.

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

We use `clang-format <http://clang.llvm.org/docs/ClangFormat.html>`_ to format all C/C++ code. The configuration is defined in the `.clang-format <https://github.com/nest/nest-simulator/blob/main/.clang-format>`_ file corresponding to the version specified by ``CLANG_REQUIRE_VERSION`` in `nestbuildmatrix.yml <https://github.com/nest/nest-simulator/blob/main/.github/workflows/nestbuildmatrix.yml>`_.

``clang-format`` is run automatically via ``pre-commit``.

Local static analysis
---------------------

We have several static code analyzers in the GitHub Actions CI. To run static code checks locally,
please refer to the "run" lines in the GitHub Actions CI definition at
https://github.com/nest/nest-simulator/blob/main/.github/workflows/nestbuildmatrix.yml.

include-what-you-use
~~~~~~~~~~~~~~~~~~~~

The GitHub Actions CI runs `include-what-you-use <https://include-what-you-use.org/>`_ (IWYU) on the files in
``nestkernel/``, ``models/`` and ``libnestutil/`` that a pull request changes. The check is informational
for now and does not block merging. The job summary lists the number of findings per file, the full output
is attached to the workflow run as the ``iwyu-log`` artifact, and findings in changed files are shown as
annotations.

To run the same check locally, install IWYU (e.g. ``sudo apt install iwyu`` on Debian/Ubuntu),
configure NEST with ``clang++`` and ``-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`` (no build is needed),
and run ``build_support/check_iwyu.sh`` on the build directory:

.. code-block:: bash

   cmake -S <nest_source_dir> -B <build_dir> -DCMAKE_CXX_COMPILER=clang++          -DCMAKE_INSTALL_PREFIX=<install_dir> -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
   <nest_source_dir>/build_support/check_iwyu.sh --changed main <build_dir>

``--changed main`` checks only the files that differ from ``main``, including uncommitted changes; a
changed header is checked through its source file. Without ``--changed``, all files are checked, which
takes a few minutes. To check specific files or directories, pass them after the build directory, e.g.
``build_support/check_iwyu.sh <build_dir> nestkernel/node_manager.cpp``. IWYU analyses a header
together with its source file, so pass ``node_manager.cpp`` to check ``node_manager.h``. The output is
written to ``<build_dir>/iwyu.log``.

If IWYU reports ``'omp.h' file not found``, either configure with ``-Dwith-openmp=OFF`` or add the
directory containing your compiler's ``omp.h`` with ``-DCMAKE_CXX_FLAGS="-isystem <dir>"``. Code inside
``#ifdef`` blocks for disabled features is not analysed, so includes needed only there may be reported
as superfluous.

IWYU's suggestions are not always right. Treat them as hints, not as rules:

* Suggestions for one file belong together: an include reported as superfluous may only be
  dispensable once the suggested additions are made.
* IWYU misjudges the ``kernel::manager< ... >`` accessors and macros such as ``LOG`` defined in
  header-only files, and sometimes asks a file to include headers needed only inside other
  headers' templates.
* ``config.h`` is always kept, because IWYU does not count ``#ifdef HAVE_...`` as a use.
* IWYU's advice to include ``<stddef.h>`` for ``size_t`` is reported as ``<cstddef>``, which NEST uses.
* NEST-specific mappings, e.g. that Boost sorting is used through ``sort.h``, are defined in
  ``build_support/iwyu/nest.imp``. Extend this file if IWYU repeatedly suggests an unsuitable header.
