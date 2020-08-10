Developing NEST with Eclipse
============================

These instructions are based on Eclipse Mars (4.5).

.. contents:: On this page, you'll find
   :local:
   :depth: 3

Installing Eclipse
__________________

Prerequisites
~~~~~~~~~~~~~

You have to have a Java Development Kit (JDK) v. 1.7.0 (Java 7) or later
installed. Just a Java Runtime Environment (JRE) is not enough.

You can download the JDK from
`Oracle's JDK8 Download Page <http://www.oracle.com/technetwork/java/javase/downloads/jdk8-downloads-2133151.html>`_.

Debugger under OSX
~~~~~~~~~~~~~~~~~~

Under OSX, you need to install ``gdb`` to be able to debug NEST with
Eclipse, since Eclipse does not support ``lldb`` (yet). For installation
instructions, check out the instructions for `OS X Mavericks
<http://ntraft.com/installing-gdb-on-os-x-mavericks>`_ and
`Darwin <https://sourceware.org/gdb/wiki/BuildingOnDarwin>`_.

Eclipse
~~~~~~~

From `the Eclipse downloads page <http://www.eclipse.org/downloads/>`_,
download either the *Eclipse IDE for C/C++ Developers* or the *Eclipse
Installer* and use it to install the C/C++ IDE.

Under OSX, always download the 64-bit version. Under Windows or Linux,
download the version that fits your operating system.

Workspace
~~~~~~~~~

In this document, I assume that you collect all your Eclipse projects
in a single workspace, located at ``$HOME/eclipse/workspace``. This will
keep most Eclipse-generated files out of your code directories,
reducing clutter there. But you can place your workspace
anywhere.

PyDev
~~~~~

Once you have installed and started Eclipse, go to "Help" → "Eclipse
Marketplace" and install the ``PyDev`` extension, then restart Eclipse.

Once PyDev is installed, open the Eclipse preferences, go to
"PyDev" → "Interpreters" → "Python Interpreter" and configure your
interpreter. You can try "Advance Auto-Config", but this will often
not detect the correct Python interpreter. In that case,
choose "New ..." and browse to the executable of your Python interpreter, e.g.,
``$HOME/anaconda/bin/python2.7``.

You need to repeat this step for each new workspace you enter.

CppStyle
~~~~~~~~

*CppStyle* is a source-code formatter based on ``clang-format``. You can install it from the
Eclipse Marketplace in the same way as *PyDev*; remember to restart Eclipse.

Then, open Eclipse preferences, go to ``CppStyle`` and enter the path to
your ``clang-format`` executable; remember that NEST code formatting
uses ``clang-format`` version 3.6, so you should link to an executable
for that version.

You also need to create a symbolic link from your Eclipse workspace
directory to ``$NEST_ROOT/src/.clang-format`` for ``clang-format`` to find
the file with the code formatting rules.


General settings in Eclipse
~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Open Eclipse preferences and go to "General" → "Editors" → "Text Editors"
2. Set the following:

   * ``Displayed tab width`` to 2
   * ``Insert spaces for tabs`` checked
   * ``Show print margin`` checked and colum set to 79

Directory Structure for NEST
____________________________

We discuss here some ideas on how to organize NEST source code, build
and install directories. This applies even if you do not work with
Eclipse.

We assume that all directories discussed here live in directory ``$NEST_ROOT``.

``$NEST_ROOT`` should have a single directory for source code,
``$NEST_ROOT/src``.  When working with different branches, you switch
branches inside the ``src`` directory.

For each NEST build configuration, you then need a separate build
directory. A build configuration here means any particular combination
of a source code branch and configure options, e.g., compilation with
or without MPI or with or without debugging support. Placing the
install directory for a given build configuration inside the build
directory reduces clutter.

A typical set of build directories could then look like this

====================   ========================================================
Directory              Purpose
--------------------   --------------------------------------------------------
``bld_master_nompi``   production version for laptop, only compiled from master
``bld_fixes_nompi``    testing code in branches for small fixes
``bld_fixes_mpi``      testing code in branches for small fixes with MPI
``bld_debug_nompi``    for debugging
====================   ========================================================

If you have longer-running branches for major changes, you may want to create one or more ``bld_`` directories for
this branch in addition, so that you can always "hop into" work on that branch without having to recompile much code.


Setting up NEST with Eclipse
____________________________

Preparations
~~~~~~~~~~~~

You should configure NEST as usual. In this document,
we will first set up the NEST production build
``bld_master_nompi``. Handling further configurations will be described
in a later section.

We thus assume the following directory layout:

.. code::

   $NEST_ROOT/src                          # source code
   $NEST_ROOT/bld_master_nompi             # build directory
   $NEST_ROOT/bld_master_nompi/install     # install directory

You should configure, build and install NEST manually once (note that
I want to build NEST with gcc 6.x from Homebrew, therefore the
``-DCMAKE_C_COMPILER=gcc-6 -DCMAKE_CXX_COMPILER=g++-6`` arguments to ``cmake``;
NB: Make sure that you have checked out the master branch in the ``src`` directory):

.. code::

   cd $NEST_ROOT/bld_master_nompi
   cmake -DCMAKE_INSTALL_PREFIX=$NEST_ROOT/bld_master_nompi/install -DCMAKE_C_COMPILER=gcc-5 -DCMAKE_CXX_COMPILER=g++-5 -Dwith-debug=ON ../src
   make -j4
   make install
   make installcheck

.. note::

   With ``cmake`` you can also generate the Eclipse project files yourself by adding the option
   ``-G "Eclipse CDT4 - Unix Makefiles"``. The following section assumes, that you do not use this option.

Project setup
~~~~~~~~~~~~~

1. "File" → "New" → "Makefile project with existing code"
2. Choose an arbitrary project name
3. Browse to the ``$NEST_ROOT/src`` directory
4. Keep `C` and `C++` checked
5. Choose ``GNU Autotools Toolchain`` for indexer settings.
6. Click ``Finish``

The indexer will scan the code, this may take a while.

To make Eclipse aware of configuration-dependent settings, especially
include guards such as ``HAVE_GSL``, we need to add header files from
the build directory. To this end, select the project in the project
browser and choose ``Properties`` from the context menu. Then

1. go to ``C/C++ General > Paths and Symbols``
2. choose ``Includes`` tab and there ``GNU C``
3. click ``Add``
4. check off for ``Add to all languages``
5. click ``File system ...`` and select the
   ``$NEST_ROOT/bld_master_nompi/libnestutil`` directory
6. add the ``$NEST_ROOT/bld_master_nompi/nest`` directory in the same way
7. rebuild the index when Eclipse suggest it or by choosing "Index" →
   "Rebuild" from the context menu on the project.

To enable code formatting with ``clang-format`` via ``CppStyle``, open the Properties window
for the project and go to ``C/C++ General > Formatter``, enable project specific settings, choose ``CppStyle`` as
Code Formatter. ``Source > Format`` will now format source code according to the ``.clang-format`` file shipped with NEST.

Finally, we need to tell Eclipse about the build path.

1. From the project context menu, choose "Build configurations" → "Manage" ... .
   Rename the ``Build GNU`` build configuration to according to the build directory (helps
   keeping an overview later), in our case ``bld_master_nompi``.
2. Choose the project in the project browser, then ``Properties`` from the context
   menu.
3. Go to ``C/C++ Build``
4. It should show the``bld_master_nompi`` (or whatever name you chose)  as active
   configuration.
5. Then, in the ``Build location`` section of the ``C/C++ Build`` window, click
   ``File system ...``, then choose ``$NEST_ROOT/bld_master_nompi``.
6. If you want to build in parallel, remove the check for
   ``Use default build command`` and enter ``make -j4`` as build command
   (replace 4 with a suitable number for your computer).

Finally, we need to amend the search path for tools Eclipse uses. In the
project properties browser,

1. click ``Select ...`` and choose ``PATH``
2. select ``PATH`` in the variables list and click ``Edit ...``
3. prepend to the path
   a. ``/usr/local/bin:`` if you use Homebrew
   b. ``/opt/local/bin:`` if you use MacPorts

You can now build the project by choosing ``Build project`` from the
context menu.

To install or run the testsuite, you should add additional make
targets:

1. Go to the Context Menu of the project
2. Choose ``Make Targets > Create ...`` and add a target,
   e.g. ``install`` by entering this as the target name.
3. Remove the check for ``Run all project builders``.
4. You should create targets
   * ``all`` (builds nest)
   * ``install`` (installs nest, including tests and help)
   * ``install-exec`` (installs compiled code and Python, but not SLI
	 code, tests, or help; faster if you only changed C++ or Python files)
   * ``installcheck`` (runs the testsuite)
5. You can run the targets by choosing ``Make Targets > Build ...`` from
   the Context Menu.

You can also check out the `CDT/Autotools/User Guide <https://wiki.eclipse.org/CDT/Autotools/User_Guide>`_.

Running NEST from Eclipse
~~~~~~~~~~~~~~~~~~~~~~~~~

To run NEST within Eclipse,

1. go to the project properties browser
2. select ``Run/Debug Settings``
3. select ``NEST Build (GNU)`` and click ``Edit ...``
4. rename to ``run_master_nompi``
5. under ``C/C++ Application`` click ``Browse ...`` and select
   ``$NEST_ROOT/bld_master_nompi/ins/bin/nest``
6. select ``Disable auto build`` (because that only builds, but does not install)

You can now run NEST by clicking the "Play" button. Input is echoed in
a slightly funny way in the build-in console, but NEST works fine. You
need to quit NEST with the ``quit`` command, ``Ctrl-D`` does not seem to
work (made my machine hang totally on one occasion).

Multiple build directories and configurations
_____________________________________________

We have little experience with multiple build directories yet, so take
this with a pinch of salt and let us know about your experiences! See
above for a general suggestion on how to organize build directories.

For the example here, we set up a ``bld_fixes_mpi`` build directory and
then add the corresponding build and run configuration in Eclipse. In
general, you need to set up one build and one run configuration for
each build directory you create.

Configuring and additional build directory
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create and configure the build directory as usual and build and
install NEST once (do not use the MPI compiler wrappers for ``cmake``, as
it will figure out the correct options itself).

.. code::

   cd $NEST_ROOT
   mkdir bld_fixes_mpi
   cd bld_fixes_mpi
   cmake -DCMAKE_INSTALL_PREFIX=$NEST_ROOT/bld_fixes_mpi/install -DCMAKE_C_COMPILER=gcc-5 -DCMAKE_CXX_COMPILER=g++-5 -Dwith-debug=ON -Dwith-mpi=ON ../src
   make -j4
   make install
   make installcheck

Then, in Eclipse

1. In the project context menu, choose
   ``Build configurations > Manage ...`` and then ``New ...``
2. Choose a name, preferably the same as the build directory, here ``bld_fixes_mpi`` and choose to
   copy settings from an existing configuration.
3. In the context menu, choose ``Build configurations > Set Active`` and
   select you new configuration.
4. Choose ``Properties`` from the context menu and go to ``C/C++ General > Path and Symbols``. Delete
   the include directories listed (for C and C++) and add the ``libnestutil`` and ``nest``
   directories from the build directory, rebuild the index when Eclipse suggest it (deleting and
   adding paths is easier than editing them, because with
   the ``Add to all languages`` option you only need to add each path once).
5. In the ``Properties`` window go to ``C/C++ Build``,
   choose the ``Builder Settings`` tab and then under "Build location"
   click ``File system ...`` and select the build directory for this
   configuration, e.g., ``$NEST_ROOT/bld_fixes_mpi``.
6. In the ``Properties`` window, go to ``Run/Debug Settings``, select an
   existing configuraton and click ``Duplicate``, then select the new
   configuration and choose ``Edit``.
7. Edit the name of the configuration, e.g. to ``run_fixes_mpi`` and the  path to the C/C++
   Application. If you have not built this configuration yet, you will get a warning; ignore it.

Building and running with multiple configurations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* You select the active configuration from the project context menu via ``Build Configurations > Set
  Active``.
* To build a different configuration directly, you can also click on the little triangle next to the
  hammer icon and select the configuration you want to build.

A build just runs make. If you want to do more (install, run the tests), you need to select one of the
make targets from the context menu via "Make Targets" → "Build" ...; in this case, you will always
run the active build configuration.

When running a new configuration for the first time,

* either click on the triangle next to the "play" button, choose "Run configurations", select the
  configuration you want to run and click "Run"
* or go to the same menu via the context menu "Run as" ... → "Run configurations" ...

Afterwards, you can select the run configuration by clicking on the little triangle next to the play button.

Debugging in Eclipse
--------------------

This section is very preliminary.

1. Create a build directory and configure NEST with the ``--with-debug`` switch, then add a
   corresponding configuration in Eclipse as described above.
2. Remember to also create a run configuration. Then, click the triangle next to the Bug to start
   debugging, choosing your debug run configuration.
3. Eclipse stops the debugger on entry to main, you probably want to click Resume here.

.. note::

   At present, we are not able to get any variable values out in gdb. This seems to be a gdb
   problem. We also have this problem with gdb on the command line. So on the Mac we may have
   to wait until Eclipse support lldb.
