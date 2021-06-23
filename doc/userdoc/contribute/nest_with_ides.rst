Developing NEST with IDEs
=========================

Integrated development environments can make coding more efficient and fun.
Here are some recipes based on practical experience on how to use
IDEs in NEST development. The information is not meant to be complete,
cross-checked or up-to-date but is provided on a best-effort basis to get
you started. Kindly contribute your experiences.

Since we generally recommend out-of-source builds, only such are discussed here.

.. contents:: We currently have recipes for the following IDEs
   :local:
   :depth: 2

Eclipse
-------

This recipe describes how to set up Eclipse for editing, building and
running NEST. The description here was tested on macOS 11 and Ubuntu 16.

Requirements and limitations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* Focus on single build configuration
* Assumes all dependencies (OpenMPI, GSL, etc) installed in a Conda environment
* Does not support debugging on macOS (because Eclipse does not support lldb)
* Does not read the NEST `.clang-format` file, so code formatting may
  be incorrect
* Does not use the ``cmake4eclipse`` plug-in, because we haven't figured out
  how to use it with our complex setup. The instructions below rely on running
  ``cmake`` manually in the build directory.
* Tested with Eclipse 2020-12

Preparations
~~~~~~~~~~~~

#. Install Eclipse from `Eclipse Installer <https://www.eclipse.org/downloads/packages/installer>`_.
   Using the Installer ensures that a suitable Java is installed. The instructions
   below are based on choosing the *Eclipse IDE for Scientific Computing*.
#. From the Eclipse Marketplace, install *PyDev* and *LiClipse Text* extensions.
#. In Eclipse preferences, under ``General > Security > Secure Storage – [Tab] Advanced``,
   you should replace the default password encryption scheme to a more secure level
   than default, e.g. to ``PBE...SHA512...AES_256``.

Setting up the project
~~~~~~~~~~~~~~~~~~~~~~

#. :doc:`Clone NEST <development_workflow>` onto your computer
#. Build NEST manually

   a. Create a build directory
   #. Run ``cmake`` with suitable options
   #. Run ``make all`` and ``make install``
#. In Eclipse

   a. choose ``New > Makefile project with existing code``
   #. select the directory containing the NEST source code or enter the path
   #. choose, e.g., the ``Cross GCC Toolchain``
#. Right click the project and choose ``Properties`` from the context
   menu

   a. Under ``C/C++ Build/Build Variables``, define ``BUILD_DIR`` and ``CONDA_ENV``,
      both of type ``Path``. The first should contain the full path to the build
      directory you created above, the second the full path to your conda
      environment, usually something like ``.../miniconda3/envs/nest-dev``.
   #. Under ``C/C++ Build – [Tab] Builder Settings``,

      #. uncheck ``Use default build command``
      #. set ``Build Command`` to ``make -k -j4 all install`` (adjust
	 number of processes to your situation)
      #. set ``Build Directory`` to ``${BUILD_DIR}``
   #. Under ``C/C++ Build > Environment``, prepend
      ``${CONDA_ENV}/bin`` to ``PATH``
   #. Under ``C/C++ General > Paths and Symbols – [Tab] Includes``, add the
      following two direcories

      * ``${BUILD_DIR}/libnestutil`` (contains ``config.h``)
      * ``${CONDA_ENV}/include`` (all headers from packages provided in conda environment)
   #. Under ``PyDev - Interpreter/Grammar``, choose the interpreter from
      your Conda environment (you may need to add it by following the
      ``Click here to configure an interpreter not listed`` link and
      then ``Browse for python/pypy exe`` (this temporarily takes you
      to the global Eclipse preferences in a separate window).
   #. If you do not install PyNEST into a default Python package installation location,
      then under ``PyDev - PYTHONPATH [Tab] External Libraries`` click ``Add source folder``
      and select the `lib/pythonX.Y/site-packages` directory in the NEST installation
      directory.
   #. Under ``Run/Debug Settings``, add a ``New ...`` launch
      configuration, entering in ``C/C++ Application`` the full path
      to the installed ``nest`` executable.

Usage
~~~~~

* Eclipse should now find all includes.
* It should intepret switches such as ``HAVE_GSL`` and ``HAVE_MPI``
  correctly, i.e., shade the code for the option that is not given.
  If this does not seem to work, try to rebuild the C/C++ index by
  opening C++ source file and chosing ``Project > C/C++ Index >
  Rebuild``.
* Clicking the hammer icon should compile and install NEST. Errors
  will be shown in the console and summarized in the warnings tab
  and you can jump directly to corresponding code locations.
* ``Run`` should run NEST in a console inside Eclipse. Under Linux,
  ``Debug`` should also start a debugging session. To get most out of
  debugging, run ``cmake`` in the build directory with
  ``-Dwith-debug=ON``.


Visual Studio Code
------------------

The following section will guide you through setting up Visual Studio Code (VS Code) for editing, building,
running, and debugging NEST. Tested with Ubuntu 20.04. Steps for macOS should be equivalent, but with ``⌘``
instead of ``ctrl`` in keyboard shortcuts.

Requirements and limitations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* Assumes a suitable compiler (GCC/Clang/etc.) is installed.
* Assumes CMake version 3.15 or newer is installed.
* C++ debugging assumes GDB is installed if on Linux, and Xcode and LLDB is installed if on macOS.
* Debugging C++ from VS Code is only possible with a SLI script. It is probably possible to launch
  the Python debugger, then attach a C++ debugging instance to that process, but that is left
  as an exercise for the reader.
* Tested with VS Code 1.53.2.

Preparations
~~~~~~~~~~~~

#. Install VS Code. See the
   `VS Code setup documentation <https://code.visualstudio.com/docs/setup/setup-overview>`_ for instructions.
#. In VS Code, open the extensions menu by choosing it in the sidebar, or pressing ``Ctrl+Shift+X``.
   Install the following extensions (all published by Microsoft):

   * *C/C++ Extension Pack*
   * *Python*
   * *PyLance*

Setting up the project
~~~~~~~~~~~~~~~~~~~~~~

#. Clone NEST onto your computer. It is recommended to clone it into a project directory,
   where you also can put the build and install directories. This guide will assume that
   NEST is cloned into a directory named ``source`` in a project directory.
#. In VS Code, choose ``File > Open Folder`` and open the NEST source directory you just cloned.
#. Choose ``File > Preferences > Settings``, switch from **User** to **Workspace**, and set the following:

   a. *Cmake: Build Directory* to ``${workspaceFolder}/../build``
   #. *Cmake: Install Prefix* to ``${workspaceFolder}/../build/install``
   #. *Cpp Standard* to ``c++11``

#. In the source directory, open ``.vscode/c_cpp_properties.json``, and add

   .. code-block:: JSON

      "compileCommands": "${workspaceFolder}/../build/compile_commands.json",

   to the configuration.

After running **CMake: Configure** in the next section, which generates the compile commands, VS Code should find
all includes and know about included classes, functions, and variables. Additionally,
it should know about switches such as ``HAVE_GSL`` and ``HAVE_MPI``, and mark relevant inactive regions.
When compiling, NEST will be compiled into a build directory ``build`` next to the cloned source directory, and installed
in a subdirectory ``install`` of the build directory.

Building NEST
~~~~~~~~~~~~~

#. Open the Command Palette (``Ctrl+Shift+P``) and run **CMake: Select a Kit**. Select the compiler you want to use.
#. Open the Command Palette and run **CMake: Select variant**. Select the variant you want to use (for example,
   select **Debug** for quick compilation and debug information).
#. Open the Command Palette and run **CMake: Configure**. This will configure the project in the build directory.
   You only need to do this before the first time you build. A panel should open and show the output from the CMake command.
   Verify in the configuration summary that NEST has found the right libraries, and the right Python installation.
#. Open the Command Palette and run **CMake: Build** or select **Build** from the Status bar. A panel will now show
   the build progress. You can click on the lock symbol in the top right corner of the panel to toggle autoscrolling.
#. Once the build is finished, open the Command Palette and run **CMake: Install** to install NEST into the
   *Install Prefix* directory specified in the previous section.

Running and debugging
~~~~~~~~~~~~~~~~~~~~~

Running a NEST Python script
############################

The steps below give a rough guide to how you can run a NEST Python script. For more detailed
documentation on working with Python in VS Code, see the
`VS Code Python tutorial <https://code.visualstudio.com/docs/python/python-tutorial>`_.

#. Select a Python interpreter by opening the Command Palette (``Ctrl+Shift+P``) and running
   **Python: Select Interpreter**. Select the Python installation found by NEST in the configuration step.
#. Open ``File > Preferences > Settings``, go to **Terminal>integrated>Env:<your OS>**, and click on
   **Edit in settings.json**. VS Code will open ``settings.json`` and create a JSON object (JSON objects are
   similar to dictionaries in Python). In that object, add

   .. code-block:: JSON

      "PYTHONPATH": "${workspaceFolder}/../build/install/lib/<YOUR PYTHON VERSION>/site-packages:${env:PYTHONPATH}"

   replacing ``<YOUR PYTHON VERSION>`` with your Python version, e.g. ``python3.8``. You can check the
   ``build/install/lib/`` directory to get the correct name.
#. Open or create a Python script.
#. When you open a Python file for the first time, VS Code will ask if you want to install a linter. It is
   recommended to install a linter, for example *Flake8*, to keep the code clean and readable.
#. Run the script by clicking the triangle (▷) at the top right corner, or right-clicking in the editor and choosing
   **Run Python file in Terminal**. A panel should open with a terminal showing the output.

Running a NEST Python script with a Python debugger
###################################################

The steps below give a rough guide to how you can run a NEST Python script with the built-in debugger. For more detailed
documentation on Python debugging in VS Code, see the
`VS Code Python debugging documentation <https://code.visualstudio.com/docs/python/debugging>`_.

#. Set up the interpreter and script as described above.
#. In the Side Bar, open the **Run** pane, or press ``Ctrl+Shift+D``.
#. Create a Python debug config by either

   * selecting **Add configuration...** from the dropdown menu, or
   * clicking the ``Create a launch.json file`` link, if the ``launch.json`` doesn't exist
#. The debug configuration defaults to the current open Python file. Go back to the Python script and start the
   debugger by selecting the debug configuration from the dropdown in the Run pane (you can also use
   the hotkey ``F5``).
#. A panel with output will open, and the program will run until it finishes, or encounters an error or a breakpoint.

Running a SLI script with a debugger
####################################

The steps below give a rough guide to how you can run NEST with GDB in VS Code. For more detailed
documentation on C++ debugging in VS Code, see the
`VS Code C++ debugging documentation <https://code.visualstudio.com/docs/cpp/cpp-debug>`_.

#. In the Side Bar, open the **Run** pane, or press ``Ctrl+Shift+D``.
#. Add a debug config by either

   * selecting **Add configuration...** from the dropdown menu, or
   * clicking the ``Create a launch.json file`` link, if the ``launch.json`` doesn't exist
#. Choose the template for ``C/C++ (gdb) launch`` (or ``C/C++ (lldb) launch`` if on macOS) and

   * change the entry for ``program`` to ``"${workspaceFolder}/../build/install/bin/nest"``
   * add ``"${file}"`` to the ``args`` list
#. Open your SLI script and start debugging by selecting the debug configuration from the dropdown in the Run pane.
#. A panel with output will open, and the program will run until it finishes, or encounters an error or a breakpoint.

Xcode Workflow
--------------

This section contains instructions on how to develop NEST on a Mac (OSX 10.10.3 as of this writing) using Xcode (Version 6.3.2). As the shipped gcc, aka clang (based on LLVM 3.6.0svn), does not support OpenMP and there is no MPI shipped by default, this also explains how to get a proper gcc (with OpenMP and MPI enabled) installed on Mac.

Setup Infrastructure
~~~~~~~~~~~~~~~~~~~~

We need several packages installed, before we can become productive with NEST:

* gcc
* openmpi 1.6 (or later)
* gsl
* cmake
* libtool
* ipython, python, cython, ... The best way to install all the python requirements is to use `Anaconda <https://store.continuum.io/cshop/anaconda/>`_.

We present two ways to install the rest: MacPorts and Homebrew. For both versions you need to have Xcode and Xcode command line tools installed:

1. Install Xcode from the AppStore.
1. Install the Xcode command line tools by executing the following line in the Terminal and following the instructions in the windows that will pop up

   .. code-block:: sh

      xcode-select --install


Homebrew
^^^^^^^^

1. Follow the install instructions for Homebrew (`short <http://brew.sh/>`_) or `long <https://github.com/Homebrew/homebrew/blob/master/share/doc/homebrew/Installation.md#installation>`_)
1. Open up the Terminal and execute the following lines:

   .. code-block:: sh

      brew install gcc gsl cmake open-mpi libtool

MacPorts
^^^^^^^^

(We recommend using the Homebrew workflow, since there you can use a more current OpenMPI version for NEST, but we leave the MacPorts instructions for legacy purposes.)

1. Follow the install instructions for `MacPorts <https://www.macports.org/install.php>`_.
1. Open up the Terminal and execute the following lines:

   .. code-block:: sh

      sudo port install gcc48
      sudo port select gcc mp-gcc48 # make gcc-48 the default compiler
      sudo port install gsl +gcc48
      sudo port install cmake       # build tools

1. NEST on Mac requires OpenMPI 1.6 from MacPorts to work properly, so we have to get this older version for MacPort. Download the portsfile `Portfile-openmpi-1.6.4.txt <http://www.nest-simulator.org/wp-content/uploads/2014/12/Portfile-openmpi-1.6.4.txt>`_ and save it under the name ``Portfile`` in an arbitraty directory.
1. In Terminal, move to the directory containing Portfile and run

   .. code-block:: sh

      sudo port install +gcc48 +threads configure.compiler=macports-gcc-4.8

Install NEST
~~~~~~~~~~~~

1. Get NEST from Github. You should follow the ``Fork`` / ``Pull Request`` process and clone from your fork:

   .. code-block:: sh

      cd <somebase>
      mkdir NEST
      cd NEST
      mkdir src build install
      git clone https://github.com/nest/nest-simulator.git src

   Afterwards you should have a directory structure like:

   .. code-block::

      <somebase>/NEST/
      - src/
      - build/
      - install/

1. Build NEST

   .. code-block:: sh

      cd src
      cd ../build

      # with Homebrew infrastructure run:
      cmake -DCMAKE_INSTALL_PREFIX=$PWD/../install -Dwith-debug=ON -Dwith-mpi=ON -DCMAKE_C_COMPILER=gcc-5 -DCMAKE_CXX_COMPILER=g++-5 $PWD/../src
      # with MacPorts infrastructure run:
      cmake -DCMAKE_INSTALL_PREFIX=$PWD/../install -Dwith-debug=ON -Dwith-mpi=ON -DCMAKE_C_COMPILER=gcc-mp-4.8 -DCMAKE_CXX_COMPILER=g++-mp-4.8 $PWD/../src

      make -j8    # run make with 8 processes
      make install
      make installcheck

.. note::

   It is important, that the ``cmake`` command is *not* executed with relative paths, in order for Xcode to find source files mentioned in the build logs.

.. note::

   If you want to debug your code with Xcode later, it has to be compiled with ``debug-options`` enabled.

.. note::

   Always supply a concrete ``CMAKE_C_COMPILER`` and ``CMAKE_CXX_COMPILER`` for the configure: e.g. ``-DCMAKE_C_COMPILER=gcc-5 -DCMAKE_CXX_COMPILER=g++-5`` (for Homebrew) or ``-DCMAKE_C_COMPILER=gcc-mp-4.8 -DCMAKE_CXX_COMPILER=g++-mp-4.8`` (for MacPorts). Otherwise Xcode will prefer to use the gcc/clang version.

.. note::

   Even if you want to build with MPI enabled, do not set the wrapper compilers for ``CMAKE_*_COMPILER``, as cmake will figure out the correct compiler options on its own.

.. note::

   With cmake it is also possible, to generate the XCode project files with ``-G Xcode``, but this will require you to build with ``gcc/clang``. The following instructions assume, that you do not use this option.


Get Xcode working with NEST
~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Create a new project, which we will call ``NEST-fork`` in this article. In the menu select File -> New -> Project... . Then select OS X -> Other -> External Build System (with build tool ``/usr/bin/make``)
1. Add the NEST sources to the project. There is a ``+`` in the left-bottom corner (see image). Click ``Add Files to "NEST-fork"...``. Then select the ``<somebase>/NEST/src/`` folder (do not copy items and use groups).

   .. figure:: _images/xcode_article/add_files.png
      :alt: Add Sources

   Also add the generated files:

   .. code-block::

      <somebase>/NEST/build/libnestutil/config.h
      <somebase>/NEST/build/libnestutil/sliconfig.h
      <somebase>/NEST/build/nest/static_modules.h

1. On the left panel select the newly created project ``NEST-fork``, then select the created target:

   .. figure:: _images/xcode_article/execution_dir.png
      :alt: Execution path

   Here you set set Directory to ``<somebase>/NEST/build``. This will be the directory, in which the ``make`` command is executed. Also check ``Pass build settings in environment``.

1. Next select the ``Build Settings`` panel.

   .. figure:: _images/xcode_article/add_path.png
      :alt: Add $PATH

  Here you ``Add User-Defined Setting`` and name it ``PATH``. In the ``NEST-fork`` column (the second) you copy the content of your ``PATH`` variable (do ``echo $PATH`` in the Terminal).

1. The build system (CMD+B) should work from now on.

Running NEST from Xcode
~~~~~~~~~~~~~~~~~~~~~~~

We have to edit the Targets Scheme:

1. In the menu select: Product -> Scheme -> Manage Schemes...
1. Select the ``NEST-fork`` target and hit ``Edit...``
1. Select the ``Run`` option on the left and then on the right select ``Info``.
1. As ``Executable`` select ``<somebase>/NEST/install/bin/nest``.
1. You can specify arguments in the ``Arguments`` panel.

.. note::

   The executable ``<somebase>/NEST/install/bin/nest`` will only be updated, if you execute ``make install`` in the terminal.


Code Completion in Xcode
~~~~~~~~~~~~~~~~~~~~~~~~

We have to create a new target and configure it appropriately:

1. In the menu select: File -> New -> Target....
1. Make the target a OS X -> Command Line Tool (Next), of type C++ in your project (preselected). We call it ``completion``
1. Remove all files and folders that are created with the new target.
1. In the tab "Build Phase" of the new target, under "Compile Sources" add all ``*.h``, ``*.hpp``, ``*.c``, ``*.cc``, ``*.cpp`` files from the list (you can use CMD+a).

   .. figure:: _images/xcode_article/completion.png
      :alt: Code Completion

1. Now Xcode generates its index and after that code completion should work.
