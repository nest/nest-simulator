:orphan:

The doc folder
==============

The documentation for NEST is contained in this folder. We are currently working
on improving the documentation and making updates. However, if you notice
something out of place, you can `submit an issue <https://nest.github.io/nest-simulator/development_workflow#reporting-bugs-and-issues>`_
in our GitHub repository.

Build the docs locally
-----------------------

You will need the NEST source code in your project::


    cd nest-simulator/doc/

Requirements for building the docs include::

 python3
 setuptools
 pandoc
 sphinx
 recommonmark
 sphinx-rtd-theme
 nbsphinx
 breathe
 sphinx-gallery
 ipython
 mock


You can now build the docs using this command (make sure you are in the doc folder)::

    make html


Once this complete, you can open `_build/html/index.html` to view the docs!

::

    xdg-open _build/html/index.html

For **developer documentation**, we use `doxygen <http://doxygen.org/>`__
comments extensively throughout NEST.
After installing NEST, you can extract comments from the source code with
``make doc`` and a doxygen folder with html files will be generated in the doc
folder in your source directory.

For a list of commands for SLI and C++, you can access the the online command
index via the command line

::

   import nest
   nest.helpdesk()


.. note::

 The command ``helpdesk()`` needs to know which browser to launch in order to display
 the help pages. The browser is set as an option of helpdesk. Please see the file
 ``~/.nestrc`` for an example setting firefox as browser.
 Please note that the command helpdesk does not work if you have compiled
 NEST with MPI support, but you have to enter the address of the helpdesk
 (file:///install/path/share/doc/nest/index.html) manually into the browser.
 Please replace ``/install/path`` with the path under which NEST is installed.

Local install of user documentation
--------------------------------------

If you are using Linux or macOS and want to locally install the docs
and generate the html files, follow the steps below.

Requirements
~~~~~~~~~~~~

To keep everything simple and easy to maintain, the installation with
conda is described here. So all you need is conda. If you don’t have it
yet, `miniconda <https://conda.io/miniconda.html>`__ is recommended.

Installation
~~~~~~~~~~~~

With the following steps a full development environment is installed -
even the newest NEST release.

Change to the doc/ folder in the source directory (if you’re not already there).

::

   cd ./doc

Create and activate the environment.

::

   conda env create -f nest_doc_conda_env.yml
   conda update -n base conda
   conda activate nest_doc

Now generate the html files. They are then located in ./docs/_build/html.

::

   make html

If you want to deactivate and/or delete the build environment:

::

   conda deactivate
   conda remove --name nest_doc --all
