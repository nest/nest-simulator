.. _userdoc_workflow:

User-level documentation workflow
=================================

Overview of workflow
--------------------

We use `Sphinx <https://www.sphinx-doc.org/en/master/>`_ to generate
documentation and `Read the Docs <https://readthedocs.org/>`_ to
publish it. Sphinx uses reStructuredText as the base format for the
documentation. To learn more about the syntax, check out this `quick
reference
<https://www.sphinx-doc.org/en/master/usage/restructuredtext/basics.html>`_.

The NEST Simulator documentation lives alongside its code. It is
contained in the ``doc/htmldoc`` directory within the `NEST source
code repository <https://github.com/nest/nest-simulator>`_ on GitHub.

We work with `GitHub <https://www.github.com>`_ as a web-based hosting
service for Git. Git allows us to keep our versions under control,
with each release of NEST having its own documentation.

.. mermaid::
   :zoom:
   :caption: Overview of documentation build. Drag and zoom to explore.

   flowchart TB

      sphinx:::TextPosition

      classDef TextPosition padding-right:25em;
      classDef orangeFill color:#fff, stroke:#f63, stroke-width:2px, fill:#f63;
      classDef blueFill color:#fff, stroke:#072f42, stroke-width:2px, fill:#072f42;
      classDef brownFill color:#fff, stroke:#652200, stroke-width:2px, fill:#652200;

      subgraph sphinx[SPHINX]
        read(Read source files):::blueFill-->ext
        read-->custom
       subgraph Parse_rst ["Parse rst"]
          ext(sphinx_extensions):::blueFill
          custom(custom_extensions):::blueFill
        end
      Parse_rst-->build
      build(Build output formats):::blueFill
      end
    subgraph EDIT SOURCE FILES
      source(repo: nest/nest-simulator):::orangeFill-- sphinx-build-->read
     end

     subgraph OUTPUT["REVIEW OUTPUT"]
       direction TB
       build--local filesystem-->local(_build directory):::brownFill
       build--hosting platform-->rtd(Read the Docs):::brownFill
       local-->HTML(HTML):::brownFill
       rtd-->HTML
     end


Contribute to the documentation
-------------------------------

You can make changes directly to your forked copy of the `NEST source
code repository <https://github.com/nest/nest-simulator>`_ and create a `pull
request <https://github.com/nest/nest-simulator/pulls>`_. Just follow the
workflow below!

If you have not done so alrealdy first

* Fork the nest-simulator repository (see :ref:`here <fork>` for details on first time setup)

* Clone the nest-simulator:

.. code-block:: bash

   git clone git@github.com:<my-username>/nest-simulator

* Create a branch to a make your changes

.. code-block:: bash

   git checkout -b <my-new-feature>

Set up your environment
~~~~~~~~~~~~~~~~~~~~~~~

Using the Conda package (includes everything to build NEST, including documentation)
````````````````````````````````````````````````````````````````````````````````````

For details on Conda, see :ref:`conda_tips`

.. code-block:: bash

    cd <nest_source_dir>/
    conda env create -p conda/
    conda activate conda/

If you later on want to deactivate or delete the build environment:

.. code-block:: bash

   conda deactivate
   rm -rf conda/

Using pip (includes packages for documentation only)
````````````````````````````````````````````````````

If you want to install only a minimal set of packages for building the
documentation and avoid using Conda, you can use pip:

.. code-block:: bash

    pip3 install -r <nest_source_dir>/doc/requirements.txt

If you use pip, install ``pandoc`` from your platform's package manager (e.g. apt):

.. code-block:: bash

    sudo apt-get install pandoc

.. admonition::  Building plantuml diagrams locally

   The plantuml diagrams require additional requirements
   to make them work in a local build.

   You will need to

   - check if you have Java installed. The minimum version needed is Java 8.
     (e.g., to install the latest available version on Ubuntu: ``apt install jre-default``)
   - Download the `plantuml jar file <https://plantuml.com/download>`_ (Minimum version is 1-2023-10)
   - Move the jar file to ``/tmp/plantuml.jar``

   - To see if plantuml diagrams render correctly after building the documentation you can take a look
     at our :ref:`test_uml`.


Edit and create pages
~~~~~~~~~~~~~~~~~~~~~~

You can now edit or add new files with your editor of choice. Most documentation files are
written in reStructuredText and are found in the ``doc/htmldoc`` directory. There are some exceptions, detailed below.
If you're unfamiliar with reStructuredText, you can find some
`helpful hints here <https://www.sphinx-doc.org/en/master/usage/restructuredtext/basics.html>`_.

Please see our :ref:`documentation style guide <doc_styleguide>` for information on how to write good documentation in NEST.


Where to find documentation in the repository
`````````````````````````````````````````````

Most documentation is located in ``doc/htmldoc`` with some exceptions.

If you want to edit Model docs, PyNEST API files, or PyNEST examples, you will need to edit the source files:

.. list-table::
   :header-rows: 1

   * - Type of documentation
     - Source location
   * - Model docs
     - ``nest-simulator/models/*.h`` in the section `BeginUserDocs`
   * - PyNEST API
     - ``nest-simulator/pynest/nest/**/*.py``
   * - PyNEST examples
     - ``nest-simulator/pynest/examples/**/*.py``


.. note::


  Also consider that any new pages you create need to be referenced in the relevant
  table of contents.



Review changes you made
~~~~~~~~~~~~~~~~~~~~~~~

To check that the changes you made are correct in the HTML output,
you will need to build the documentation locally with Sphinx.

#. Navigate to the ``doc/htmldoc`` folder:

.. code-block:: bash

   cd nest-simulator/doc/htmldoc

#. Build the docs:

.. code-block:: bash

   sphinx-build . ../_build/html -b html


#. Preview files. They are located in ``doc/_build/html``

.. code-block:: bash

   <browser> ../_build/html/index.html

.. tip::

   You can also build the user documentation in the build directory with CMake:

   .. code-block:: bash

       cmake -Dwith-userdoc=ON </path/to/NEST/src>
       make docs



Create a pull request
~~~~~~~~~~~~~~~~~~~~~

Once you're happy with the changes, you can submit a pull request on Github from your fork.
Github has a nice help page that outlines the process for
`submitting pull requests <https://help.github.com/articles/using-pull-requests/#initiating-the-pull-request>`_.

Reviewers will be assigned and go through your changes.

If you are a first time contributor, we ask that you fill out the
:download:`NEST Contributor Agreement <https://nest-simulator.readthedocs.io/en/latest/_downloads/9b65adbdacba6bfed66e68c62af4e308/NEST_Contributor_Agreement.pdf>`
form to transfer your copyright to the NEST initiative and send it to *info [at] nest-initiative.org*.

.. tip::

   If you notice any errors or weaknesses in the documentation, you can
   also submit an `Issue <https://github.com/nest/nest-simulator/issues>`_ on
   GitHub.


.. seealso::

   This workflow shows you how to create **user-level documentation**
   for NEST. For the **developer documentation**, please refer to our
   :ref:`Developer documentation workflow
   <devdoc_workflow>`.


Read the Docs
``````````````

NEST documentation is hosted on Read the Docs. If you would like to view the documentation
on Read the Docs, you can set up your own account and link it with your Github account.

See `this guide <https://docs.readthedocs.io/en/stable/intro/import-guide.htmli>`_
for more information.

.. toctree::
   :hidden:

   /testuml
