Frequently Asked Questions
==============================



1. CMAKE error says a <package> was not found or <package> is too old
------------------------------------------------------------------------


*Please make sure you have followed the installation instructions* :doc:`found here <installation/index>` *and have installed the
required dependencies.*


1. Install the missing package or update the package to a more recent version.

2. Remove the contents of the build directory

    .. code-block:: bash

        rm -r /path/to/nest-simulator-x.y.z-build/*

3. Compile NEST again

    .. code-block:: bash

        cmake -DCMAKE_INSTALL_PREFIX:PATH=</install/path> </path/to/NEST/src>


**If the error still persists**, you may have more than one installation of the <package>.  A conflict may occur between different package binaries:
Your system may prefentially choose a system-wide installation of a package (e.g., /usr/bin/<package>), rather than a
local environment installation (e.g., /home/user/ENV/bin).


1. Determine the path and version of the <package>:


``which <package>``
    searches the path of executables set in the $PATH environment variable. It will tell you the path to the <package> binary that is being used in the current environment.

``echo $PATH``
    shows the complete list of directories that your system looks in to run executables (binary files).
    The first result should be the location to your active environment.

``<package> --version``
    will tell you the version of the <package> binary.

Here is an example,

.. code-block:: bash

    which python

    /usr/local/bin/python

.. code-block:: bash

    python --version

    Python 2.7.15+



2. If it looks like you have an older version on your system:

       * Remove or update old versions of <package> (You may need to uninstall and reinstall the package)

   If you do not have the <package> in your local environment:

       * Install the <package> while in your active environment.

3. Remove the contents of the build directory

   .. code-block:: bash

       rm -r /path/to/nest-simulator-x.y.z-build/*

4. Compile NEST again

  .. code-block:: bash

      cmake -DCMAKE_INSTALL_PREFIX:PATH=</install/path> </path/to/NEST/src>


2. When I try to import nest, I get an error in Python that says 'No Module named NEST' or 'Module not found'
--------------------------------------------------------------------------------------------------------------

This error message means something in your environment is not set correctly, depending on how you installed NEST.

If you compiled NEST from source
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    * Your path variables may not be set correctly, in that case run:

          .. code-block:: bash

              source </path/to/nest_install_dir>/bin/nest_vars.sh


    * You need to ensure you are using the correct Python version. (Did you compile NEST with python 3 or python 2 bindings?)
      You can check which Python version you are using, by running:

          .. code-block:: bash

              python --version

      For example, if the command above gives ``Python version 2.7``, but you compiled NEST with Python 3, then you need to run NEST in ``python3``.

If you installed NEST via the PPA
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    You may need to run ``python3`` rather than just ``python`` since the PPA is built with Python 3 bindings.


If you installed NEST via the conda-forge package
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    * Make sure you have activated the correct environment

    To get a list of all your environments, run:

        .. code-block:: bash

            conda info -e

    An asterisk (\*) indicates the active environment.

    Activate the correct environment if it's not already:

        .. code-block:: bash

            conda activate ENVNAME

    Try to ``import nest`` in Python.


    * Check that the correct package binary is used for NEST and Python: for example, in a terminal type:

         .. code-block:: bash

             which python

     This will show you the path to the Python binary that your environment is using. You may have more than one Python installation on your system. The path to Python should be within your active environment:

         .. code-block:: bash

             /path/to/conda/envs/ENVNAME/bin/python

    You can also view the list of packages in the active environment, by running:

        .. code-block:: bash

            conda list

    If the package is not in your environment, then it needs to be installed.

    If something is missing, you can try to  ``conda install <package>`` BUT be aware that this **may break pre-installed packages**!

    You may be better off creating a new Conda environment and install NEST with all needed packages at one time!
    See the section on :doc:`installation for Conda <installation/index>`.




3. Docker crashes! Message from NotebookApp: "Running as root is not recommended. Use --allow-root to bypass."
-------------------------------------------------------------------------------------------------------------------------------------------

    **We strongly recommend that you do not run Docker as root!**

    * If this happens, try to update the docker build. In the terminal type:

    .. code-block:: bash

        docker pull nestsim/nest:<version>

    replacing ``<version>`` with the actual version you want to use.

    * Then try the ``docker run`` command again.

    .. code-block:: bash

       docker run --rm -e LOCAL_USER_ID=`id -u $USER` -v $(pwd):/opt/data -p 8080:8080 nestsim/nest:<version> notebook

Can't find an answer to your question?
----------------------------------------------

We may have answered your question on GitHub or in our Mailing List!

Please check out our `GitHub issues page <https://github.com/nest/nest-simulator/issues?utf8=%E2%9C%93&q=is%3Aissue+?>`_ or search the
`mailing list <https://www.nest-simulator.org/mailinglist/>`_ for your question.
