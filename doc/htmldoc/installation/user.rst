.. _user_install:

User install instructions
=========================

Cross-platform options
-----------------------


Docker |linux| |macos| |windows|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:ref:`See our docker installation instructions <docker>`.


Conda install |linux| |macos|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can install NEST with the :ref:`Conda forge package <conda_forge_install>`.


Live media |linux| |macos| |windows|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We have live media (.ova) if you want to run NEST in a virtual machine.

:ref:`Download the live media here <download_livemedia>`, and follow the :doc:`instructions to set up the virtual machine <livemedia>` .


-------------

Linux |linux|
-------------

Ubuntu
~~~~~~

Ubuntu users can install NEST via the PPA repository.

1. Add the PPA repository for NEST and update apt:

.. code-block:: bash

     sudo add-apt-repository ppa:nest-simulator/nest
     sudo apt-get update

2. Install NEST:

.. code-block:: bash

     sudo apt-get install nest

Debian
~~~~~~

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

.. code-block:: bash

    sudo apt install python3-all dh-python

5. After installing the dependencies, enter ``sudo apt source --build nest`` again.
   When the build finished, look for lines like:

.. code-block:: bash

    dpkg-deb: building package 'nest-dbgsym' in '../nest-dbgsym_2.20.0-0~202001311135~ubuntu20.04.1_amd64.deb'.
    dpkg-deb: building package 'nest' in '../nest_2.20.0-0~202001311135~ubuntu20.04.1_amd64.deb'.
    #dh binary
    dpkg-genbuildinfo --build=binary
    dpkg-genchanges --build=binary >../nest_2.20.0-0~202001311135~ubuntu20.04.1_amd64.changes

and note down the full package name. In the above example this would be
``nest_2.20.0-0~202001311135~ubuntu20.04.1_amd64.deb``, where the number ``202001311135`` and potentially the
Ubuntu version number may be different.

6. Install the ready Debian package after the rebuild:

.. code-block:: bash

    sudo dpkg --install nest-simulator-x.y.z~NUMBER~ubuntu20.04.1_amd64.deb

    The package name is taken from the result of the previous step. `NUMBER` and potentially the Ubuntu
    version might differ.

7. Test the package:

.. code-block:: bash

   python3
   import nest

-------------

macOS |macos|
-------------

1. `Install Homebrew <https://brew.sh/>`_.

2. Install NEST via:

.. code-block:: bash

    brew install nest

--------


Options for Windows users |windows|
------------------------------------

Please note that NEST does not officially support Windows. Members of our community have had success
using NEST on Windows with the `Windows Subsystem for Linux <https://ubuntu.com/tutorials/install-ubuntu-on-wsl2-on-windows-11-with-gui-support#1-overview>`_.
You can also try our :ref:`docker container <docker_win>`.

.. |linux| image:: ../static/img/linux.png
   :class: no-scaled-link
   :scale: 7%

.. |macos| image:: ../static/img/macos.png
   :class: no-scaled-link
   :scale: 7%

.. |windows| image:: ../static/img/windows.png
   :class: no-scaled-link
   :scale: 7%
