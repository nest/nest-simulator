.. _check_code:

Check your code
===============

Below, we provide tools and scripts that you can use to check the formatting of your code.
Before you get started, please take a look at our :ref:`detailed guidelines for C++ coding in NEST <code_style_cpp>`

Development environment
-----------------------

We have provided an `environment.yml <https://github.com/nest/nest-simulator/blob/master/environment.yml>`_ file that contains all packages to do development
in NEST, including the tools to check coding style.

See our :ref:`instructions on installing NEST from source <dev_install>`.


Tooling
-------

The `clang-format <http://clang.llvm.org/docs/ClangFormat.html>`_ tool is built
on the clang compiler frontend. It prettyprints input files in a
configurable manner, and also has Vim and Emacs integration. We supply a
clang-format-file (``<build_support/format_all_c_c++_files.sh>`` to enforce some parts of the coding style. During
the code review process we check that there is no difference between the committed
files and the formatted version of the committed files.


Developers can benefit from the tool by formatting their changes
before issuing a pull request. For fixing more files at once we
provide a script that applies the formatting.

From the source directory call:

.. code::

   ./build_support/format_all_c_c++_files.sh [start folder, defaults to '$PWD']


The code has to compile without warnings (in the default settings of the build
infrastructure). We restrict ourselves to the C++11 standard for a larger support of
compilers on various cluster systems and supercomputers.

We use clang-format version 13 in our CI. If your `clang-format` executable is not version 13, you need to specify an executable with version 13 explicitly with the `--clang-format` option to ensure consistency with the NEST CI.


In addition, we let `cppcheck <http://cppcheck.sourceforge.net/>`_ statically analyse
the committed files and check for severe errors. We require cppcheck version
1.69 or later.

.. code:: sh

   cppcheck --enable=all <committed file>


Python
------

We enforce `PEP8 <https://www.python.org/dev/peps/pep-0008/>`_ formatting, using `Black
<https://github.com/psf/black>`_. You can automatically have your code reformatted before
you commit using pre-commit hooks:

.. code-block:: bash

   pip install pre-commit
   pre-commit install

Now, whenever you commit, Black will check your code. If something was reformatted it
will show up in your unstaged changes. Stage them and recommit to succesfully commit
your code. Alternatively, you can run black manually:

.. code-block:: bash

   pip install black
   black .


Local static analysis
---------------------

To run local static code checks, please refer to the "run" lines in the GitHub Actions CI definition at https://github.com/nest/nest-simulator/blob/master/.github/workflows/nestbuildmatrix.yml.
