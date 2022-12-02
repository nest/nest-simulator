.. _devdoc_workflow:

Developer documentation workflow
################################

What you need to know
+++++++++++++++++++++

For developer documentation, we use `Doxygen <http://doxygen.org/>`__
comments extensively throughout NEST.

After installing NEST, you can extract comments from the source code
with ``make docs``. A ``doxygen`` folder with HTML files will be
generated in the ``doc`` folder in your source directory.

.. note::

   This workflow shows you how to create **developer documentation**
   for NEST. For the **user documentation**, please refer to our
   :ref:`User documentation workflow <userdoc_workflow>`.

Instructions
++++++++++++

1. Install Doxygen and graphviz.

If you are a Linux user, type:

.. code-block::
   :name: Linux

   sudo apt install doxygen graphviz

For macOS, please use `Homebrew <https://brew.sh/>`_:

.. code-block::

   brew install doxygen graphviz

2. Navigate to, or create a ``build`` directory. See :ref:`install_nest`.

3. Add the ``-Dwith-devdoc=ON`` flag to your regular CMake command:

.. code-block::

   cmake -Dwith-devdoc=ON

3. Generate HTML:

.. code-block::

   make docs

4. Preview documentation:

.. code-block::

   cd doc/doxygen/html
   browser index.html
