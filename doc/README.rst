:orphan:

The doc folder
==============

This folder contains the static parts of the documentation for NEST,
as well as the infrastructure and tools for extracting documentation
dynamically from the source code and building a unified HTML site
using Sphinx.

For each of the releases, the fullly built documentation is available
at https://nest-simulator.readthedocs.io/

Build the user documentation locally
------------------------------------

If you intend to work on the documentation, or if you want to obtain a
local version of it for other reasons, you can build the documentation
by simply running

    make html

in the build directory of NEST (i.e. the directory where you ran
``cmake``). To install the dependencies for building the
documentation, you can either directly run

    pip install -r <nest_source_dir>/doc/requirements.txt

or alternatively install the dependencies into a conda environment by
running

    conda env create -f <nest_source_dir>/doc/environment.yml -p conda
    conda activate conda/

Once this complete, you can open `doc/html/index.html` to view the docs.

::

    xdg-open doc/html/index.html

Developer documentation
-----------------------
    
For **developer documentation**, we use `doxygen <http://doxygen.org/>`__
comments extensively throughout NEST.
After installing NEST, you can extract comments from the source code with
``make doc`` and a doxygen folder with html files will be generated in the doc
folder in your source directory.

SLI documentation
-----------------

For a list of commands for SLI, you can access the the online command
index via the command line

::

   import nest
   nest.helpdesk()


.. note::

 The command ``helpdesk()`` needs to know which browser to launch in
 order to display the help pages. The browser is set as an option of
 helpdesk. Please see the file ``~/.nestrc`` for an example setting
 firefox as browser.  Please note that the command helpdesk does not
 work if you have compiled NEST with MPI support, but you have to
 enter the address of the helpdesk
 (file://<nest_install_dir>/share/doc/nest/index.html) manually into
 the browser.
