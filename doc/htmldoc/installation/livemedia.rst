.. _live_media:

NEST LIVE MEDIA Installation
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




.. _download_livemedia:

Download the NEST image for VMs
-------------------------------

The VM image of NEST is available in the OVA format, and is suitable, for example, for importing into VirtualBox.
If you run **Windows**, this is the option for you OR if you just want to run NEST without installing it on your computer.

* Download the `latest live media image  <https://nest-simulator.org/downloads/gplreleases/nest-latest.ova>`_.

  `Checksum <https://nest-simulator.org/downloads/gplreleases/nest-latest.ova.sha512sum>`_

* Start Virtual Box and import the virtual machine image (**File** > **Import Appliance**)

* Once imported, you can run the NEST image

* The user password is **nest**.



Older versions of VM images
---------------------------


`NEST Live Media 3.1 <https://nest-simulator.org/downloads/gplreleases/nest-3.1.ova>`_

`Checksum 3.1 <https://nest-simulator.org/downloads/gplreleases/nest-3.1.ova.sha512sum>`_

`NEST Live Media 2.20.2 <https://nest-simulator.org/downloads/gplreleases/nest-2.20.2.ova>`_

`Checksum 2.20.2 <https://nest-simulator.org/downloads/gplreleases/nest-2.20.2.ova.sha512sum>`_


We continuously aim to improve NEST, implement features, and fix bugs with every new version;
thus, we encourage our users to use the **most recent version of NEST**.



Notes
~~~~~~~~

* For better performance you can increase the memory for the machine **Settings** > **System** > **Base Memory**

* To allow fullscreen mode of the virtual machine you also need to increase the video memory above 16MB. (**Settings** > **Display** > **Video Memory**)

* If you need to share folders between the virtual machine and your regular desktop  environment, click on **Settings**. Choose **Shared Folder** and add the folder you wish to share. Make sure to mark **automount**.

* To install Guest Additions, select **Devices** > **Insert Guest Additions CD image...**  (top left of the VirtualBox Window). Then, open a terminal (Ctrl+Alt+t), go to "/media/nest/VBOXADDITIONS.../" and run "sudo bash VboxLinuxAdditions.run".

* To set the correct language layout for your keyboard (e.g., from "US" to "DE"), open a terminal and type: "sudo dpkg-reconfigure keyboard-configuration". After setting the correct keyboard map you have to reboot the system to activate the changes.



