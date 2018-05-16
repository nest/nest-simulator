NEST LIVE MEDIA installation
=============================


Download and install a virtual machine if you do not already have one installed.

.. note:: 

   Although, the following instructions are for `Virtual Box <https://www.virtualbox.org/wiki/Downloads>`_, you can use a different virtual machine, such as `VMWare <http://www.vmware.com>`_. 

For Linux users, it is possible to install Virtual Box from the package repositories.

Debian::

    sudo apt-get install virtualbox

Fedora::

    sudo dnf install virtualbox

SuSe::

    sudo zypper install virtualbox

NEST image setup
------------------

* Download the `NEST live medium <http://www.nest-simulator.org/downloads/gplreleases/lubuntu-16.04_nest-2.12.0.ova>`_

* Start Virtual Box and import the virtual machine image "lubuntu-16.04_nest-
2.12.0.ova" (**File** > **Import Appliance**)

* Once imported, you can run the NEST image

* The user password is **nest**.

Notes
~~~~~~~~

* For better performance you can increase the memory for the machine **Settings** > **System** > **Base Memory**

* To allow fullscreen mode of the virtual machine you also need to increase the video memory above 16MB. (**Settings** > **Display** > **Video Memory**)

* If you need to share folders between the virtual machine and your regular desktop  environment, click on **Settings**. Choose **Shared Folder** and add the folder you wish to share. Make sure to mark **automount**.

* To install Guest Additions, select **Devices** > **Insert Guest Additions CD image...**  (top left of the VirtualBox Window). Then, open a terminal (Ctrl+Alt+t), go to "/media/nest/VBOXADDITIONS_.../" and run "sudo bash VboxLinuxAdditions.run".

* To set the correct language layout for your keyboard (e.g., from "US" to "DE"), you can use the program "lxkeymap", which you start by typing "lxkeymap" in the terminal.

Cyg`win
~~~~~~~

Cywin is a software layer which emulates Unix system calls. NEST should compile and install under Cygwin with the generic installation instructions, but we have not tested this for a long time and do not support NEST under Cygwin at present. Compilation under Cygwin is very slow, but the execution times are comparable to an equivalent Linux installation.


