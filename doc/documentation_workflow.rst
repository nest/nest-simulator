Documentation workflow
######################

What you need to know
+++++++++++++++++++++

We use `Sphinx <https://www.sphinx-doc.org/en/master/>`_ to generate documentation and `Read the Docs <https://readthedocs.org/>`_ to publish it.

The documentation for NEST is contained in the ``docs`` directory within the `NEST source code repository <https://github.com/nest/nest-simulator>`_ on GitHub.

Using `GitHub <https://github.com/>`_ allows the NEST simulator documentation to live alongside its code. Our developers benefit from smooth version control, with each release of NEST having its own documentation.

.. image:: _static/img/documentation_workflow.png
  :width: 300
  :alt: Alternative text

Changing the documentation
++++++++++++++++++++++++++

If you notice any errors or weaknesses in the documentation, please submit an `issue <https://github.com/nest/nest-simulator/issues>`_ in our GitHub repository.

You can also make changes directly to your forked copy of the `NEST source code repository <https://github.com/nest/nest-simulator>`_ and create a `pull request <https://github.com/nest/nest-simulator/pulls>`_. Just follow the workflow below!

Setting up your environment
+++++++++++++++++++++++++++

To keep things simple, we have created a conda environment for you. Installing it will enable you to smoothly generate documentation for NEST.

If you are using Linux and want to install a full development environment:

1. Install conda (we recommend `miniconda <https://docs.conda.io/en/latest/miniconda.html#>`_).

2. Switch to the ``doc`` folder in the source directory:

.. code-block:: bash

    cd </path/to/nest_source>/doc

3. Create and activate the environment:

.. code-block:: bash

   conda env create -f nest_doc_conda_env.yml
   conda update -n base -c defaults conda
   conda activate nest-doc

4. If you want to deactivate or delete the build environment:

.. code-block:: bash

   conda deactivate
   conda remove --name nest-doc --all

Generating documentation with Sphinx
++++++++++++++++++++++++++++++++++++

Now that you activated your environment, you can generate HTML files using Sphinx.

Sphinx uses reStructuredText. To learn more on how to use it, check out this `quick reference <https://docutils.sourceforge.io/docs/user/rst/quickref.html>`_.

Editing and creating pages
~~~~~~~~~~~~~~~~~~~~~~~~~~

1. You can edit and/or add ``.rst`` files in the ``doc`` directory using your editor of choice.

2. If you create a new page, open ``index.rst`` in the ``doc`` directory and add the file name under ``.. toctree::``. This will ensure it appears on the NEST simulator documentation's table of contents.

3. If you rename or move a file, please make sure you update all the corresponding cross-references.

4. Save your changes.

Rendering HTML
~~~~~~~~~~~~~~

Using Sphinx, you can build documentation locally and preview it offline:

1. Go to the ``doc`` folder in the source directory:

.. code-block:: bash

    cd </path/to/nest_source>/doc

2. Generate HTML files:

.. code-block:: bash

   make html

3. Preview files. They are then located in ``./docs/_build/html``:

.. code-block:: bash

   cd </path/to/nest_source>/doc/_build/html
   browser filename.html

4. If you add further changes to your files, repeat steps 1-3.

Previewing on Read the Docs (optional)
++++++++++++++++++++++++++++++++++++++

Proceed as follows to preview your version of the documentation on Read the Docs.

1. Check that unwanted directories are listed in ``.gitignore``:

.. code-block:: bash

   _build
   _static
   _templates

2. Add, commit and push your changes to GitHub.

3. Go to `Read the Docs <https://readthedocs.org/>`_. Sign up for an account if you don't have one.

4. `Import <https://readthedocs.org/dashboard/import/>`_ the project.

5. Enter the details of your project in the ``repo`` field and hit ``Create``.

6. `Build your documentation <https://docs.readthedocs.io/en/stable/intro/import-guide.html#building-your-documentation>`_.

This allows you to preview your work on your Read the Docs account. In order to see the changes on the official NEST simulator documentation, please submit a pull request (see below).

Creating pull request
+++++++++++++++++++++

When you feel your documentation work is finished, you can create a pull request to the ``master`` branch of the NEST Source Code Repository. Your pull request will be reviewed by our NEST Documentation Team.

Developer documentation
+++++++++++++++++++++++

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
 (file:///</path/to/nest_install_dir>/share/doc/nest/index.html) manually into the browser.
 Please replace ``/install/path`` with the path under which NEST is installed.