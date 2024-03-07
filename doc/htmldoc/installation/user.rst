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

.. _ubuntu_install:

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

Or install NEST with `NESTML <https://nestml.readthedocs.io/en/latest/index.html>`_

.. code-block:: bash

    sudo apt install nest python3-nestml
    python3 -m pip install --upgrade odetoolbox pygsl antlr4-python3-runtime==4.10

3. Set the environment

.. code-block:: bash

    source /usr/bin/nest_vars.sh

--------


.. _macos_install:

macOS |macos|
-------------

1. `Install Homebrew <https://brew.sh/>`_.

2. Install NEST via:

.. code-block:: bash

    brew install nest

--------

.. _windows_install:

Options for Windows users |windows|
------------------------------------

Please note that NEST does not officially support Windows. Members of our community have had success
using NEST on Windows with the `Windows Subsystem for Linux <https://ubuntu.com/tutorials/install-ubuntu-on-wsl2-on-windows-11-with-gui-support>`_.
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
