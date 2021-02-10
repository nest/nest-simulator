Developer documentation workflow
################################

What you need to know
+++++++++++++++++++++

For developer documentation, we use `Doxygen <http://doxygen.org/>`__
comments extensively throughout NEST.

After installing NEST, you can extract comments from the source code with
``make doc``. A ``doxygen`` folder with HTML files will be generated in the ``doc`` folder in your source directory.

.. note::
   This workflow shows you how to create **developer documentation** for NEST. For **user documentation**, please refer to our :doc:`User documentation workflow <user_documentation_workflow>`.


Instructions
++++++++++++

Make sure you have already :doc:`installed NEST <../installation/index>` and created your ``build`` and ``install`` directories. Your ``CMake`` version needs to be up-to-date.

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

Helpdesk
++++++++

For a list of commands for SLI and C++, you can access the the online command
index via the command line

::

   import nest
   nest.helpdesk()


.. note::

    The ``helpdesk()`` command needs to know which browser to launch in order to display
    the help pages. The browser is set as an option of helpdesk. Please see the file
    ``~/.nestrc`` for an example, which sets Firefox as browser.

.. note::

    The ``helpdesk()`` command does not work if you have compiled
    NEST with MPI support, but you have to enter the address of the helpdesk
    (``file://<nest_install_dir>/share/doc/nest/index.html``) manually into the browser.
    Replace ``<nest_install_dir>`` with the path under which NEST is installed.