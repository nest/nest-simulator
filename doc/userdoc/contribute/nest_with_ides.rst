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
