---
layout: index
---

[« Back to the index](index)

<hr>

# Writing an extension module

NEST has a modular architecture which allows you to add your own
neuron and synapse models to the NEST simulator without any need to
modify the NEST software itself, but by just adding a new module. You
can then either load this module dynamically at runtime (preferred) or
you can link NEST against your module.

By writing a new module, you can add

* your own neuron models
* your own synapse types
* your own connection (or other) functions

to NEST. For the benefit of the NEST Community at large, we would
encourage you to share your modules with other NEST users. Please see
the [workflow manual](development_workflow) to find out how to
initiate the inclusion by issuing a pull request.

On this page, you will find a (brief) overview over how to create your
own module, based on the example `MyModule`, which you find as part of
the NEST distribution. For information about how to write new neuron
or synapse models or functions as part of your module, please see the
corresponding documentation linked on the [documentation
index](index).

If you have questions, problems, or feedback about your experience
with external modules, please join the [mailing
list](http://www.nest-initiative.org/community) to share it with us
and other users.

## Prerquisites

1. Download, build and install NEST. NEST should be built outside the source code directory.
2. GNU Autotools. We have tested these instructions with the following versions:
        m4-1.4.11
        libtool-2.4.2
        autoconf-2.69
        automake-1.14.1 
3. You can use the install_autotools.sh script in the extras directory to install recent versions of the Autotools in your home directory.
4. The NEST source code and installation directory must be accessible for building modules.
5. Define the environment variable NEST_INSTALL_DIR to contain the path to which you have installed NEST, e.g. using bash,

        export NEST_INSTALL_DIR=/Users/plesser/NEST/ins

    This environment variable is not strictly necessary, but saves you typing later. 

## Building MyModule

As a starting point, try to build MyModule as follows:

1. From the NEST source directory, copy directory examples/MyModule to somewhere outside the NEST source, build or install directories.
2. Change to the new location of MyModule and prepare by issuing

        imt-b23010408:MyModule plesser$ ./bootstrap.sh
        Bootstrapping MyModule
          -> Removing old automake cache ...
          -> Running aclocal ...
          -> Installing libtool components ...
          -> Running autoheader ...
          -> Running automake ...
          -> Running autoconf ...
        Done.

    You may see some more output.
3. Leave MyModule and create a build directory for it, e.g., mmb next to it

        cd ..
        mkdir mmb
        cd mmb

4. Configure. The configure process uses the script nest-config.sh to find out where NEST is installed, where the source code resides, and which compiler options were used for compiling NEST. If nest-config.sh is not in your path, you need to provided it explicitly like this

        ../MyModule/configure --with-nest=${NEST_INSTALL_DIR}/bin/nest-config

   MyModule will then be installed to ${NEST_INSTALL_DIR}. This ensures that NEST will be able to find initializing SLI files for the module.
   You should not use the --prefix to select a different installation destination. If you do, you must make sure to use addpath in SLI before loading the module to ensure that NEST will find the SLI initialization file for your module.

5. Compile.

       make
       make install
       
6. The previous command installed MyModule to the NEST installation directory, including help files generated from the source code. 

## Using MyModule

1. Set your LD_LIBRARY_PATH to include $NEST_INSTALL_DIR/lib/nest, where $NEST_INSTALL_DIR is the root of the NEST installation directory tree. On OSX, set DYLD_LIBRARY_PATH.
2. Start NEST.
3. Load the module using

       SLI ] (mymodule) Install
       Apr 30 17:06:11: *** Info: Install
       Apr 30 17:06:11: loaded module My NEST Module

4. You should now see pif_psc_alpha in the modeldict and drop_odd_spike in the synapsedict. You can learn more about these models and the additional (meaningless) connection function supplied by the model by typing

       /pif_psc_alpha help
       /drop_odd_spike help
       /StepPatternConnect help

5. In PyNest, use

       nest.Install("mymodule")

   This is available under Linux and OSX starting with NEST 1.9.8497 and later. Link the module into NEST as described below if you run into problems. 

## Creating your own module

1. Start with the code from MyModule, replace anything called "mymodule" in any form of camelcasing by the name of your module, and proceed as above.
2. When you change names of source code files or add/remove files, you need to update `Makefile.am`.
3. `make dist` will roll a tarball of your module for distribution to others.
4. `mymodule.cpp` and `sli/mymodule.sli` contain versioning information that you may want to update. It helps to keep the C++ code and SLI wrapper of your module in sync. 

## Linking MyModule into NEST

1. Build NEST and MyModule as described above.
2. Change back to the NEST build directory.
3. Reconfigure NEST informing it about your MyModule. Note that the module MUST be installed in the NEST installation directory tree!

       ../src/configure --with-modules="mymodule"

   Several modules can be given, separated by spaces.
4. Recompile and install NEST.
5. The module should now be available as soon as NEST has started up. It will also be available in PyNEST.
6. When you make any change to your module, you must first re-compile and re-install your module.
7. Then move to the NEST build directory and issue

       make -C nest clean
       make
       make install-exec

   This rebuilds only the NEST executable. 