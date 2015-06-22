# Developing NEST with Eclipse

These instructions are based on Eclipse *Mars* (4.5). They are based
on earlier instructions by Thomas Heiberg.

## Installing Eclipse

### Prerequisites

You have to have a Java Development Kit (JDK) v. 1.7.0 (Java 7) or later
installed. Just a Java Runtime Environment (JRE) is not enough.

You can download the JDK from
[Oracle's JDK8 Download Page](http://www.oracle.com/technetwork/java/javase/downloads/jdk8-downloads-2133151.html).

### Debugger under OSX

Under OSX, you need to install `gdb` to be able to debug NEST with
Eclipse, since Eclipse does not support `lldb` (yet). For installation
instructions, see

 - http://ntraft.com/installing-gdb-on-os-x-mavericks
 - https://sourceware.org/gdb/wiki/BuildingOnDarwin

### Eclipse

From http://www.eclipse.org/downloads/, download either the "Eclipse
IDE for C/C++ Developers" or the "Eclipse Installer" and use it to
install the C/C++ IDE (until the official release of Eclipse Mars on
24 June 2015, this option is available under "Developer Builds").

Under OSX, always download the 64-bit version. Under Windows or Linux,
download the version that fits your operating system.

### Workspace

In this document, I assume that you collect all your Eclipse projects
in a single workspace, located at `$HOME/eclipse/workspace`. This will
keep most Eclipse-generated files out of your code directories,
reducing clutter there. But you can place your workspace
anywhere.

### PyDev

Once you have installed and started Eclipse, go to `Help > Eclipse
Marketplace` and install the `PyDev` extension.

Once `PyDev` is installed, open the Eclipse preferences, go to `PyDev
> Interpreters > Python Interpreter` and configure your
interpreter. You can try `Advance Auto-Config`, but this will often
not detect the correct Python interpreter. In that case, choose `New
...` and browse to the executable of your Python interpreter, e.g.,
`$HOME/anaconda/bin/python2.7`.

You need to repeat this step for each new workspace you enter.

## Setting up NEST with Eclipse

### Preparations

You should bootstrap and configure NEST as usual. This document
assumes the following directory layout:

    $NEST_ROOT/src          # source code
    $NEST_ROOT/bld          # build directory
    $NEST_ROOT/bld/ins   # install directory

You should build and install NEST manually once:

    cd $NEST_ROOT/bld
    $NEST_ROOT/src/configure --prefix=$NEST_ROOT/bld/ins
	make -j4
	make install
	make installcheck

### Project setup

1. `File > New > Makefile project with existing code`
1. Choose an arbitrary project name
1. Browse to the `$NEST_ROOT/src` directory
1. Keep `C` and `C++` checked
1. Choose `GNU Autotools Toolchain` for indexer settings.
1. Click `Finish`

The indexer will scan the code, this may take a while.

To make Eclipse aware of configuration-dependent settings, especially
include guards such as `HAVE_GSL`, we need to add header files from
the build directory. To this end, select the project in the project
browser and choose`Properties` from the context menu. Then

1. go to `C/C++ General > Paths and Symbols`
1. choose "Includes" tab and there `GNU C`
1. click 'Add'
1. check off for "Add to all configurations" and "Add to all
languages"
1. click "File system ..." and select the `$NEST_ROOT/bld/libnestutil`
directory
1. add the `$NEST_ROOT/bld/nest` directory in the same way
1. rebuild the index when Eclipse suggest it or by choosing `Index >
   Rebuild` from the context menu on the project.

Finally, we need to tell Eclipse about the build path.

1. Choose the project in the project browser, then `Properties` from the context
menu.
1. Go to `C/C++ Build` and in the "Build location" section click
`File system ...`, then choose `$NEST_ROOT/bld`.
1. If you want to build in parallel, remove the check for "Use default
   build command` and enter `make -j4` as build command (replace 4
   with a suitable number for your computer).

And we need to amend the search path for tools Eclipse uses. In the
project properties browser,

1. go to `C/C++ Build > Enviroment`
1. click `Select ...` and choose `PATH`
1. select `PATH` in the variables list and click `Edit ...`
1. prepend to the path
    1. `/usr/local/bin:` if you use Homebrew
    1. `/opt/local/bin:` if you use MacPorts

You can now build the project by choosing `Build project` from the
context menu.

To install or run the testsuite, you should add additional make
targets:

1. Go to the Context Menu of the project
1. Choose `Make Targets > Create ...` and add a target,
e.g. `install` by entering this as the target name.
1. Remove the check for "Run all project builders".
1. You should create targets `all`, `install`, `install-exec` and
`installcheck`.
1. You can run the targets by choosing `Make Targets > Build ...`from
the Context Menu.

See also https://wiki.eclipse.org/CDT/Autotools/User_Guide .

### Running NEST

To run NEST within Eclipse,

1. go to the project properties browser
1. select `Run/Debug Settings`
1. select `NEST Build (GNU)` and click `Edit ...`
1. under `C/C++ Application` click `Browse ...` and select
`$NEST_ROOT/bld/ins/bin/nest`

You can now run NEST by clicking the "Play" button.

**DO NOT QUIT NEST WITH CTRL-D, or your computer WILL HANG HORRIBLY!**


### Multiple configurations

We have little experience with multiple branches and configurations in
Eclipse yet, so take these recommendations with a pinch of salt---and
feel free to provide improvements!

Switching the source code between Git branches is straightforward:
Choose `Team > Switch to ...`and select the branch you want.

Managing the build and install directories requires a bit more
work. We recommend that you create one build directory for each branch
and configuration (e.g. `bld_master`, `bld_master_debug`,
`bld_master_mpi`, `bld_myfix`) and place the `install` directory
inside each build directory to keep directory clutter down. For
short-lived branches with small changes, you may want to work with the
build directory for the parent branch and rather rebuild after each
branch switch.

Configure NEST in the build directory as usual.

Then, in Eclipse

1. In the project context menu, choose `Build configurations > Manage
...` and then `New ...`
1. Choose a name, e.g., `Build Debug (GNU)` and choose to copy
settings from an existing configuration.
1. In the context menu, choose `Build configurations > Set Active` and
select you new configuration.
1. Choose `Properties` from the context menu, go to `C/C++ Build`,
   choose the `Builder Settings` tab and then under "Build location"
   click `File system ...` and select the build directory for this
   configuration, e.g., `$NEST_ROOT/bld_master_debug`.
1. In the Properties menu, go to `Run/Debug Settings`, select an
existing configuraton and click `Duplicate`, then select the new
configuration and choose `Edit`. 
1. Edit the name of the configuratoin and the  path to the C/C++
   Application. If you have not built this configuration yet, you will get a warning; ignore it.
1. Still in the Edit window, select the correct Build configuration.


### Code formatting
