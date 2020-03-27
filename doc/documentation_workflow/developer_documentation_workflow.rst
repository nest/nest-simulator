Developer documentation workflow
################################

What you need to know
+++++++++++++++++++++

For **developer documentation**, we use `doxygen <http://doxygen.org/>`__
comments extensively throughout NEST.

Instructions
++++++++++++

After installing NEST, you can extract comments from the source code with
``make doc`` and a doxygen folder with html files will be generated in the doc
folder in your source directory.

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
    (``file:///</path/to/nest_install_dir>/share/doc/nest/index.html``) manually into the browser.
    Replace ``/install/path`` with the path under which NEST is installed.