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

.. _iwyu_check:

Cleaning up includes with include-what-you-use
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

`include-what-you-use <https://include-what-you-use.org/>`_ (IWYU) reports missing and superfluous includes
and can apply its advice. ``build_support/check_iwyu.sh`` runs it on ``nestkernel/``, ``models/`` and
``libnestutil/``. It is a tool for cleaning up includes on demand, not a required check: IWYU's advice
depends on the IWYU version and on the standard library, so two machines can disagree.

To get the same results as everyone else, run it in the pinned container (IWYU 0.23 with GCC 15's
standard library on Ubuntu 26.04). This needs Docker or Podman; the first run downloads the image, and
every run installs the required packages, which takes a few minutes:

.. code-block:: bash

   build_support/iwyu/run_in_container.sh --changed main        # report findings
   build_support/iwyu/run_in_container.sh --changed main --fix  # apply them

The same container runs in CI on demand: add the label ``iwyu`` to a pull request, and the
``include-what-you-use`` workflow checks the files the pull request changes, on every push until the
label is removed. The job summary lists the number of findings per file, the full output is attached as
the ``iwyu-log`` artifact, and findings in changed files are shown as annotations.

To run IWYU without a container, install it (e.g. ``sudo apt install iwyu`` on Debian/Ubuntu), configure
NEST with ``-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`` (no build is needed) and run the script on the build
directory:

.. code-block:: bash

   cmake -S <nest_source_dir> -B <build_dir> -DCMAKE_CXX_COMPILER=clang++          -DCMAKE_INSTALL_PREFIX=<install_dir> -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
   <nest_source_dir>/build_support/check_iwyu.sh --changed main <build_dir>

``--changed main`` checks only the files that differ from ``main``, including uncommitted changes.
Without ``--changed``, all files are checked, which takes a few minutes. To check specific files or
directories, pass them after the build directory, e.g.
``build_support/check_iwyu.sh <build_dir> nestkernel/node_manager.cpp``. The output is written to
``<build_dir>/iwyu.log``.

``--fix`` applies the advice with IWYU's ``fix_includes.py`` (``fix_include`` on Debian/Ubuntu) and
formats the changed files with ``clang-format``. The advice for one file can change once other files are
fixed, so repeat until no findings are left. Then review the changes and build NEST in a minimal
configuration (e.g. ``-Dwith-mpi=OFF -Dwith-openmp=OFF -Dwith-gsl=OFF -Dwith-boost=OFF``) and with as many
features as possible: IWYU only sees the code of the features enabled when NEST was configured (the
container enables OpenMP, MPI, GSL, Boost and libltdl, but not MUSIC, SIONlib or HDF5), so includes needed
only by other features may be reported as superfluous. ``fix_includes.py`` also adds includes outside of
any ``#ifdef`` block: move includes of optional libraries (e.g. ``<mpi.h>``, ``<gsl/...>``) into the
matching ``#ifdef HAVE_...`` block, after ``config.h``.

IWYU analyses a header ``foo.h`` together with ``foo.cpp``. For every other header, i.e., header-only code
and ``*_impl.h`` files, the script adds a stub source file that only includes that header, so each header
is analysed as well and must compile on its own; compiler errors in such a stub are reported as
findings ("does not compile on its own"). If IWYU reports ``'omp.h' file not found``, either configure with
``-Dwith-openmp=OFF`` or add the directory containing your compiler's ``omp.h`` with
``-DCMAKE_CXX_FLAGS="-isystem <dir>"``.

Things to keep in mind:

* Suggestions for one file belong together: an include reported as superfluous may only be
  dispensable once the suggested additions are made.
* Uses inside a macro are charged to the header that defines the macro, so that header must include
  what the macro needs (or the macro calls a function declared there, as ``LOG`` does).
* IWYU cannot always see that a template-dependent use needs a complete type. If removing an include
  that IWYU calls superfluous breaks the build, mark it with ``// IWYU pragma: keep``. If a header's
  templates require users to have another header (e.g. ``exceptions.h`` and ``compose.hpp``), use
  ``// IWYU pragma: export`` in that header.
* ``config.h`` and ``*_impl.h`` includes are always kept: IWYU does not count ``#ifdef HAVE_...``
  as a use and cannot tell where the templates in ``*_impl.h`` are instantiated.
* With GCC 15's standard library, IWYU 0.23 alternates between suggesting ``<vector>`` and
  ``<format>`` for ``std::vector``, and ``<utility>`` and ``<iterator>`` for ``std::pair``.
  ``nest.imp`` maps both symbols to the header that defines them.
* IWYU's advice to include ``<stddef.h>`` for ``size_t`` is reported as ``<cstddef>``, which NEST uses.
* NEST-specific mappings, e.g. that Boost sorting is used through ``sort.h``, are defined in
  ``build_support/iwyu/nest.imp``. Extend this file if IWYU repeatedly suggests an unsuitable header.
