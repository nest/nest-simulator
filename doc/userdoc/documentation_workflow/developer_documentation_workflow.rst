Developer documentation workflow
################################

What you need to know
+++++++++++++++++++++

For developer documentation, we use `Doxygen <http://doxygen.org/>`__
comments extensively throughout NEST.

After installing NEST, you can extract comments from the source code
with ``make doc``. A ``doxygen`` folder with HTML files will be
generated in the ``doc`` folder in your source directory.

.. note::
   This workflow shows you how to create **developer documentation**
   for NEST. For the **user documentation**, please refer to our
   :doc:`User documentation workflow <user_documentation_workflow>`.

Instructions
++++++++++++

Make sure you have already :doc:`installed NEST
<../installation/index>` and created your ``build`` and ``install``
directories. Your ``CMake`` version needs to be up-to-date.

1. Install Doxygen.

If you are a Linux user, type:

.. code-block::
   :name: Linux

   sudo apt install doxygen

For macOS, please use `Homebrew <https://brew.sh/>`_:

.. code-block::

   brew install doxygen

2. Go to your build directory:

.. code-block::

   cd </path/to/nest-build>

3. Generate HTML:

.. code-block::

   make doc

4. Preview documentation:

.. code-block::

   cd doc/doxygen/html
   browser index.html
