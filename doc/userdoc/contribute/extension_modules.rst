Writing an extension module
===========================

.. note::

   The example extension module is hosted on its own repository, at:
   https://github.com/nest/nest-extension-module/

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
the :doc:`workflow manual <development_workflow>` to find out how to
initiate the inclusion by issuing a pull request.

On this page, you will find a (brief) overview over how to create your
own module, based on the example ``MyModule``, which you find at
https://github.com/nest/nest-extension-module/. For information about
how to write new neuron or synapse models or functions as part of your
module, please see the corresponding documentation linked on the
:doc:`developer documentation index <index>`.

If you have questions, problems, or feedback about your experience
with external modules, please join the `mailing
list <http://www.nest-initiative.org/community>`_ to share it with us
and other users.

Prerequisites
-------------

1. Download, build, and install NEST. NEST should be built outside the source code directory.
1. The NEST source code and installation directory must be accessible for building modules.
1. Define the environment variable ``NEST_INSTALL_DIR`` to contain the path to which you have installed NEST, e.g., using bash,

   .. code-block:: sh

      export NEST_INSTALL_DIR=/Users/plesser/NEST/install

   This environment variable is not strictly necessary, but saves you typing later.


Building MyModule
-----------------

As a starting point, try to build MyModule as follows:

1. From the NEST source directory, copy directory ``MyModule`` to somewhere outside the NEST source, build or install directories.
1. Create a build directory for it on the same level as MyModule (e.g. mmb)

   .. code-block:: sh

      cd /path/to/MyModule
      cd ..
      mkdir mmb
      cd mmb

1. Configure. The configure process uses the script ``nest-config`` to find out where NEST is installed, where the source code resides, and which compiler options were used for compiling NEST. If ``nest-config`` is not in your path, you need to provided it explicitly like this

   .. code-block:: sh

      cmake -Dwith-nest=${NEST_INSTALL_DIR}/bin/nest-config ../MyModule

   MyModule will then be installed to ``${NEST_INSTALL_DIR}``. This ensures that NEST will be able to find initializing SLI files for the module.

   You should not use the `-DCMAKE_INSTALL_PREFIX` to select a different installation destination. If you do, you must make sure to use ``addpath`` in SLI before loading the module to ensure that NEST will find the SLI initialization file for your module.

1. Compile.

   .. code-block:: sh

      make
      make install

1. The previous command installed MyModule to the NEST installation directory, including help files generated from the source code.


Using MyModule
--------------

1. Start NEST.
1. Load the module using

   .. code-block::

      SLI ] (mymodule) Install
      Apr 30 17:06:11: *** Info: Install
      Apr 30 17:06:11: loaded module My NEST Module

1. You should now see ``pif_psc_alpha`` in the ``modeldict`` and ``drop_odd_spike`` in the ``synapsedict``. You can learn more about these models and the additional (meaningless) connection function supplied by the model by typing

   ```
   /pif_psc_alpha help
   /drop_odd_spike help
   /StepPatternConnect help
   ```

1. In PyNEST, use

   .. code-block:: Python

      nest.Install("mymodule")

   This is available under Linux and MacOS. Link the module into NEST as described below if you run into problems.


Creating your own module
------------------------

1. Start with the code from MyModule.
1. Follow the instructions (1. - 4.) at the top of the ``CMakeLists.txt`` file in the MyModule directory.
1. Replace anything called "mymodule" in any form of camelcasing by the name of your module, and proceed as above.
1. When you change names of source code files or add/remove files, you need to update the variable `MODULE_SOURCES` in `CMakeLists.txt` .
1. ``make dist`` will roll a tarball of your module for distribution to others.
1. ``mymodule.cpp`` and ``sli/mymodule.sli`` contain versioning information that you may want to update. It helps to keep the C++ code and SLI wrapper of your module in sync.


Linking MyModule into NEST
--------------------------

1. Build NEST and MyModule as described above.
1. Change back to the NEST build directory.
1. Reconfigure NEST informing it about your MyModule. Note that the module MUST be installed in the NEST installation directory tree!

   .. code-block:: sh

      cmake [...] -Dexternal-modules=my ../src

   Several modules can be given, separated by semicolon.

   .. note::

      Instead of giving the full module name ``mymodule``, only give the ``SHORT_NAME`` ``my`` for the option ``-Dexternal-modules=...``.

1. Recompile and install NEST.
1. The module should now be available as soon as NEST has started up. It will also be available in PyNEST.
1. When you make any change to your module, you must first re-compile and re-install your module.
1. Then move to the NEST build directory and issue

   .. code-block:: sh

      make -C nest clean
      make
      make install

   This rebuilds only the NEST executable.
