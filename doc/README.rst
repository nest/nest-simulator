The doc folder
===============

The documentation for NEST is contained in this folder. We are currently working
on improving the documentation and making updates. However, if you notice
something out of place, you can `submit an issue <https://nest.github.io/nest-simulator/development_workflow#reporting-bugs-and-issues>`_
in our GitHub repository.

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

To access user documentation, you can check out the website or generate
the html files by following the steps below.

Requirements
~~~~~~~~~~~~

To keep everything simple and easy to maintain, the installation with
conda is described here. So all you need is conda. If you don’t have it
yet, `miniconda <https://conda.io/miniconda.html>`__ is recommended.

Installation
~~~~~~~~~~~~

With the following steps a full development environment is installed -
even NEST 2.14.0.

Change to the doc/ folder (if you’re not already there).

::

   cd ./doc

Create and activate the environment.

::

   conda env create -f environment.yml
   conda update -n base conda
   source activate doc

Now generate the html files. They are then located in ./docs/build/html.

::

   make html

.. note::

   ``make pdf``, ``make latex`` and other options are possible, too.
   For more information type ``make help``.

If you want to deactivate and/or delete the build environment:

::

   source deactivate
   conda remove --name documentation --all
