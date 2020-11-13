Install NEST
============

Standard installation
---------------------

These installation instructions should work for most users who do
not need custom configurations for their systems. If you want to
compile NEST from source, check the :ref:`advanced_install` section
and :doc:`compilation_options`.

.. tabs::


   .. tab:: Ubuntu

       Ubuntu users can install NEST via the PPA repository.

       1. Add the PPA repository for NEST and update apt:

       .. code-block:: bash

           sudo add-apt-repository ppa:nest-simulator/nest
           sudo apt-get update

       2. Install NEST:

       .. code-block:: bash

           sudo apt-get install nest


   .. tab:: Debian

       Debian users can install NEST via the Ubuntu PPA repository.

       1. Create a new ``apt`` repository entry in ``/etc/apt/sources.list.d/nest-simulator-ubuntu-nest-XXX.list`` by:

          .. code-block:: bash

             sudo apt install devscripts build-essential software-properties-common dpkg-dev
             sudo add-apt-repository --enable-source ppa:nest-simulator/nest

       2. Disable the binary package in the repository file created under ``/etc/apt/sources.list.d/`` by commenting
          out the ``deb`` line, while keeping the ``deb-src`` line. It should look similar to this:

          .. code-block:: bash

              #deb http://ppa.launchpad.net/nest-simulator/nest/ubuntu focal main
              deb-src http://ppa.launchpad.net/nest-simulator/nest/ubuntu focal main


       3. Import the PPA GPC key and rebuild the package:

          .. code-block:: bash

             sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 \
                              --recv-keys 0CF7539642ABD23CBCA8D487F0B8B6C5EC02D7DD
             sudo apt update
             sudo apt source --build nest

       4. Install any missing dependencies, if ``apt`` tells you so.
          In addition, install:

          ..  code-block:: bash

              sudo apt install python3-all dh-python

       5. After installing the dependencies, enter ``sudo apt source --build nest`` again.
          When the build finished, look for lines like

          ..  code-block:: bash

              dpkg-deb: building package 'nest-dbgsym' in '../nest-dbgsym_2.20.0-0~202001311135~ubuntu20.04.1_amd64.deb'.
              dpkg-deb: building package 'nest' in '../nest_2.20.0-0~202001311135~ubuntu20.04.1_amd64.deb'.
              #dh binary
              dpkg-genbuildinfo --build=binary
              dpkg-genchanges --build=binary >../nest_2.20.0-0~202001311135~ubuntu20.04.1_amd64.changes

          and note down the full package name. In the above example this would be
          `nest_2.20.0-0~202001311135~ubuntu20.04.1_amd64.deb`, where the number `202001311135` and potentially the
          Ubuntu version number may be different.

       6. Install the ready Debian package after the rebuild:

          ..  code-block:: bash

              sudo dpkg --install nest-simulator-x.y.z~NUMBER~ubuntu20.04.1_amd64.deb

          The package name is taken from the result of the previous step. `NUMBER` and potentially the Ubuntu
          version might differ.

       7. Test the package:

          .. code-block:: bash

             python3
             import nest


   .. tab:: NeuroFedora

       The NeuroFedora team has generously provided the latest
       versions of NEST on their platform. As that is available in the
       standard Fedora platform repositories, it can simply be
       installed using ``dnf``:

       .. code-block:: bash

           sudo dnf install python3-nest

       Find out more on the NeuroFedora site: https://docs.fedoraproject.org/en-US/neurofedora/nest/.

   .. tab:: Homebrew (macOS)

       1. `Install Homebrew <https://brew.sh/>`_ on your Mac.

       2. Install NEST via:

       .. code-block:: bash

           brew install nest

   .. tab:: Conda (Linux/macOS)

       1. Create your conda environment and install NEST. Please refer to
          our :doc:`conda_tips`.

          Without OpenMPI:

          .. code-block:: sh

              conda create --name ENVNAME -c conda-forge nest-simulator

          With OpenMPI:

          .. code-block:: sh

              conda create --name ENVNAME -c conda-forge nest-simulator=*=mpi_openmpi*

          The syntax for this install follows the pattern:
          ``nest-simulator=<version>=<build_string>``. Build strings can be
          found by listing the available versions with

          .. code-block:: sh

              conda search -c conda-forge nest-simulator

          or by browsing the `conda forge file list
          <https://anaconda.org/conda-forge/nest-simulator/files>`_ (note
          there are multiple pages). For example, to install one of the
          2.20.x versions with MPI support by OpenMPI, you would use the
          version specifier ``nest-simulator=2.20.*=*openmpi*``. The Python
          dependency is automatically resolved if you add to the above
          command a version specifier for Python, for example ``python=3`` or
          ``python=3.8``. If the Python version and build identifier are left
          unspecified, ``conda`` will install the latest version compatible
          with all requested packages.

       2. Activate your environment:

          .. code-block:: sh

             conda activate ENVNAME


In addition to native installations from ready-made packages, we
provide containerized versions of NEST in several formats:

.. tabs::

   .. tab:: Docker (Linux/macOS)

       Docker provides an isolated container to run applications. The
       NEST Docker container includes a complete install of NEST and
       is set up so you can create, modify, and run Juptyer Notebooks
       and save them on your host machine.  (See the Note below for
       alternative ways to use the Docker container.)

       1. If you do not have Docker installed, follow the Docker
          installation instructions for your system here:
          https://docs.docker.com/install/.

          If you are using **Linux**, we **strongly recommend** you
          also create a Docker group to manage Docker as a non-root
          user. See instructions on the Docker website:
          https://docs.docker.com/install/linux/linux-postinstall/


       2. Create a directory or change into a directory that you want
          to use for your Jupyter Notebooks.

       .. code-block:: bash

           mkdir my_nest_scripts
           cd my_nest_scripts

       3. Run the Docker container. Replace the ``<version>`` with one
          of the latest NEST versions (e.g., ``2.20.0``) or use
          ``latest`` for the most recent build from the source code.

       .. code-block:: bash

           docker run --rm -e LOCAL_USER_ID=`id -u $USER` -v $(pwd):/opt/data -p 8080:8080 nestsim/nest:<version> notebook


       4. Once completed, a link to a Jupyter Notebook will be
          generated, as shown below. You can then copy and paste the
          link into your browser.

           .. image:: ../_static/img/docker_link.png
              :align: center
              :width: 1000px


       5. You can now use the Jupyter Notebook as you normally
          would. Anything saved in the Notebook will be placed in the
          directory you started the Notebook from.

       6. You can shutdown the Notebook in the terminal by typing
          :kbd:`Ctrl-c` twice.  Once the Notebook is shutdown the
          container running NEST is removed.


       .. note::

           You can check for updates to the Docker build by typing:

           .. code-block:: bash

                docker pull nestsim/nest:<version>

       .. note::

          You can also create an instance of a terminal within the container itself and, for example, run Python scripts.

          .. code-block::

              docker run --rm -it -e LOCAL_USER_ID=`id -u $USER` -v $(pwd):/opt/data -p 8080:8080 nestsim/nest:<version> /bin/bash

          See the `README <https://github.com/nest/nest-docker>`_ to find out more, but note some functionality, such as DISPLAY, will not be available.

   .. tab:: Live Media (cross-platform)

       We have live media (.ova) if you want to run NEST in a virtual machine. This option is suitable for Windows users, since we don't support
       NEST natively on Windows,

       :ref:`Download the live media here <download_livemedia>`, and follow the :doc:`instructions to set up the virutal machine <livemedia>` .




**Once NEST is installed, you can run it in Python, IPython, or Jupyter Notebook**

For example, in the terminal type:

    .. code-block:: bash

         python3

Once in Python you can type:

    .. code-block:: python

        import nest

**or as a stand alone application**::

     nest


If installation was successful, you should see the NEST splash screen in the terminal:

.. figure:: ../_static/img/import_nest.png
   :scale: 50%
   :alt: import nest


**Installation is now complete!**


:doc:`Now we can start creating simulations! <../getting_started>`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If installation didn't work, see the :doc:`troubleshooting section <../troubleshooting>`.

.. seealso::

    * :doc:`PyNEST tutorials <../tutorials/index>`

    * :doc:`Example networks <../examples/index>`


----

.. _advanced_install:

Advanced installation
---------------------

**If you need special configuration options or want to compile NEST yourself, follow
these instructions.**


.. tabs::

   .. tab::  Ubuntu/Debian

       Download the source code for the  `current release <https://github.com/nest/nest-simulator/archive/v2.20.0.tar.gz>`_.

       Follow instructions for :doc:`linux_install` and take a look at our :doc:`compilation_options`.


   .. tab:: GitHub

       Get the latest developer version on `GitHub <https://github.com/nest/nest-simulator>`_. Fork NEST into your GitHub repository (see details on `GitHub workflows here <https://nest.github.io/nest-simulator/>`_).


   .. tab:: macOS

       For further options on installing NEST on macOS, see :doc:`mac_install` for Macs.


   .. tab:: HPC systems

       :doc:`Instructions for high performance computers <hpc_install>` provides some instructions for certain machines.
       Please :doc:`contact us <../community>` if you need help with your system.


.. toctree::
   :hidden:

   linux_install
   mac_install
   hpc_install
   livemedia
   install_options