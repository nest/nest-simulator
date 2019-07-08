Installation Instructions
==========================

Standard Installation Instructions
------------------------------------

**These intallation instructions should work for most of our users, who do not  need
special configurations for their systems.**


.. tabs::

   .. tab:: Linux

     .. code-block:: bash

       sudo add-apt-repository ppa:nest-simulator/nest
       sudo apt-get update
       sudo apt install nest

   .. tab:: MacOS

       .. code-block:: bash

           brew tap brewsci/science
           brew install nest

       Options have to be appended, so for example, to install NEST with PyNEST run::

           brew install nest --with-python


       *  To install homebrew, follow the instructions at `brew.sh <http://brew.sh/>`_

   .. tab:: Windows

       We don't support NEST natively on Windows, but you can run NEST in a virtual machine.

       :doc:`Download the live media here <livemedia>`.

   .. tab:: Conda for Linux or MacOS

       If you prefer using conda environments, we also have a :doc:`conda-forge package <conda_install>`



**Once NEST is installed you can run it in Python**::

     python
     import nest

**or as a stand alone application**::

     nest

Now let's create your first network!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

----

Advanced Installation Instructions
-----------------------------------

**If you need special configuration options or want to compile NEST yourselves, follow
these instructions.**


.. tabs::

   .. tab:: Linux

       Download source code for the  `current release NEST 2.18.0 <https://github.com/nest/nest-simulator/archive/v2.18.0.tar.gz>`_

       Follow instructions for :doc:`linux_install`

   .. tab:: GitHub

       Get the latest developer version

     .. note::

         The developer version should only be used if you really know what you're doing!

     .. code-block:: bash

         mkdir nest-master
         cd nest-master
         git clone https://nest/nest-simulator.git


   .. tab:: MacOS

       For further options on installing NEST on MacOS, see :ref:`mac_manual` for Macs.

   .. tab:: HPC systems?

       :doc:`hpc_install`

.. toctree::
   :hidden:

   linux_install
   mac_install
   conda_install
   hpc_install
   livemedia
   install_options


.. note:: These installation instructions are for **NEST 2.12 and later** as well as the most recent version obtained from `GitHub <https://github.com/nest/nest-simulator>`_.
 Installation instructions for NEST 2.10 and earlier are provided :doc:`here <oldvers_install>`, but  we strongly encourage all our users to stay
 up-to-date with most recent version of NEST. We cannot support out-dated versions.


