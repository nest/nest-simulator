.. _devdoc_workflow:

Developer documentation workflow
================================

What you need to know
---------------------

For developer documentation of the C++ code, we use `Doxygen <http://doxygen.org/>`__
comments extensively throughout NEST. If you add or modify the code, please ensure
you document your changes with the correct doxygen syntax.


Additional documentation for developers and contributors can be found on :ref:`Read the Docs <developer_space>`

.. note::

   This workflow shows you how to create **developer documentation**
   for NEST. For the **user documentation**, please refer to our
   :ref:`User documentation workflow <userdoc_workflow>`.

GitHub Pages
------------

The C++ developer documentation is deployed to GitHub pages.

* https://nest.github.io/nest-simulator/index.html

You can view the latest master version of the documentation along with the 2 most recent releases of NEST.

GitHub workflow on pull request
-------------------------------

If you create a pull request against the nest/nest-simulator repository and have modified

  * any C++ file (``*.cpp, *.h``),
  * the doxygen config file (``doc/fulldoc.conf.in``), or
  * the doxygen css file (``doc/htmldoc/static/css/doxygen-aweseme.css``),

then GitHub will build the C++ documentation and upload it as an artifact. You can
download and view the HTML pages that it generated locally on your computer.

Where to find the artifact
~~~~~~~~~~~~~~~~~~~~~~~~~~

At the bottom of the main pull request page (tab "Conversation"), you will see a list of
CI workflows. If you click on "Build and Deploy C++ Documenation", you will see the progress page of
that workflow. You will find the downloadable artifact on the bottom of that page.

Local build
------------

If you want to build the C++ doxygen on your computer at any point during your development, follow these instructions:


1. Install Doxygen and graphviz.

   If you are a Linux user, type:

   .. code-block::

      sudo apt install doxygen graphviz

   For macOS, please use `Homebrew <https://brew.sh/>`_:

   .. code-block::

      brew install doxygen graphviz

2. Navigate to or create a ``build`` directory. See :ref:`install_nest`.

3. Add the ``-Dwith-devdoc=ON`` flag to your regular CMake command:

   .. code-block::

      cmake -Dwith-devdoc=ON <path/to/source>

3. Generate HTML:

   .. code-block::

      make docs

4. Preview documentation:

   .. code-block::

      cd doc/doxygen/html
      <browser> index.html
