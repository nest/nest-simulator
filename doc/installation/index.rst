NEST Installation Instructions
================================

Standard Installation Instructions
------------------------------------

**These installation instructions should work for most users, who do
not need custom configurations for their systems. If you want to
compile NEST from source, see section** :ref:`advanced_install`.

.. tabs::


   .. tab:: Debian/Ubuntu PPA

       Install NEST via the PPA repository.

       1. Add the PPA repository for NEST and update apt:

       .. code-block:: bash

           sudo add-apt-repository ppa:nest-simulator/nest
           sudo apt-get update

       2. Install NEST:

       .. code-block:: bash

           sudo apt-get install nest

   .. tab:: NeuroFedora

       The NeuroFedora team has generously provided the latest
       versions of NEST on their platform. As that is available in the
       standard Fedora platform repositories, it can simply be
       installed using ``dnf``:

       .. code-block:: bash

           sudo dnf install python3-nest

       Find out more on the NeuroFedora site: https://docs.fedoraproject.org/en-US/neurofedora/nest/.       


   .. tab:: Conda (Linux/macOS)

       1. Create your conda environment and install NEST. We recommend
          that you **create a dedicated environment for NEST**, which
          should ensure there are no conflicts with previously
          installed packages.

          .. pull-quote::
        
	     We strongly recommend that you **install all programs**
             you'll need, (such as ``ipython`` or ``jupyter-lab``) in
             the environment (ENVNAME) **at the same time**, by
             **appending them to the command below**.

             Installing packages later may override previously
             installed dependencies and potentially break packages!
             See `managing environments in the Conda documentation
             <https://docs.conda.io/projects/conda/en/latest/user-guide/tasks/manage-environments.html#creating-an-environment-with-commands>`_
             for more information.

          Without OpenMPI:

          .. code-block:: sh

             conda create --name ENVNAME -c conda-forge nest-simulator

          With OpenMPI:

          .. code-block:: sh

             conda create --name ENVNAME -c conda-forge nest-simulator=*=mpi_openmpi*

          The syntax for this install follows the pattern: ``nest-simulator=<version>=<build_string>``

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
          of the latest NEST versions (e.g., ``2.18.0``) or use
          ``latest`` for the most recent build from the source code.

       .. code-block:: bash

           docker run --rm -e LOCAL_USER_ID=`id -u $USER` -v $(pwd):/opt/data -p 8080:8080 nestsim/nest:<version> notebook


       4. Once completed, a link to a Jupyter Notebook will be
          generated, as shown below. You can then copy and paste the
          link into your browser.

           .. image:: ../../_static/img/docker_link.png
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

         python

Once in Python you can type:

    .. code-block:: python

        import nest

.. note::

    If you get ImportError: No module named nest after running ``python``.  Try to run ``python3`` instead.

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

Advanced Installation Instructions
-----------------------------------

**If you need special configuration options or want to compile NEST yourself, follow
these instructions.**


.. tabs::

   .. tab::  Ubuntu/Debian

       Download the source code for the  `current release <https://github.com/nest/nest-simulator/archive/v2.18.0.tar.gz>`_.

       Follow instructions for :doc:`linux_install` and take a look at our :doc:`install_options`.


   .. tab:: GitHub

       Get the latest developer version on `GitHub <https://github.com/nest/nest-simulator>`_. Fork NEST into your GitHub repository (see details on `GitHub workflows here <https://nest.github.io/nest-simulator/>`_).


   .. tab:: macOS

       For further options on installing NEST on macOS, see :ref:`mac_manual` for Macs.


   .. tab:: HPC systems

       :doc:`Instructions for high performance computers <hpc_install>` provides some instructions for certain machines.
       Please :doc:`contact us <../community>` if you need help with your system.


.. toctree::
   :hidden:

   linux_install
   mac_install
   conda_install
   hpc_install
   livemedia
   install_options


.. note::

    Installation instructions for NEST 2.10 and earlier are provided :doc:`here <oldvers_install>`, but  we strongly encourage all our users to stay
    up-to-date with most recent version of NEST. We cannot support out-dated versions.


