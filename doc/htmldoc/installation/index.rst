.. _install_nest:

Install NEST
============


.. grid:: 3
   :gutter: 1

   .. grid-item-card:: |user|  Install a pre-built NEST package using
      :class-title: sd-d-flex-row sd-align-minor-center

      * :ref:`Docker (cross-platform) <docker>`
        (Includes NESTML)
      * :ref:`conda-forge (Linux/macOS) <conda_forge_install>`
      * :ref:`Ubuntu PPA (Linux) <ubuntu_install>`
      * :ref:`Homebrew (macOS) <macos_install>`
      * :ref:`Options for Windows users <windows_install>`


   .. grid-item-card:: |dev| Source install
      :class-title: sd-d-flex-row sd-align-minor-center

      If you want to do development with NEST

      * :ref:`dev_install`

   .. grid-item-card:: |admin| HPC install
      :class-title: sd-d-flex-row sd-align-minor-center

      If you are working with HPC systems

      * :ref:`Install NEST for supercomputers and clusters <hpc_install>`

   .. grid-item-card:: |desktop|
      :class-title: sd-d-flex-row sd-align-minor-center

      A graphical user interface, ideal for learning and teaching concepts
      regarding neural networks in classrooms and workshops.

      :doc:`NEST Desktop documentation <desktop:index>` :octicon:`link-external`

      :ref:`Install docker container with NEST + NEST Desktop <docker_compose>`

   .. grid-item-card:: |nestml|
      :class-title: sd-d-flex-row sd-align-minor-center

      Install the NEST modeling language to create and customize models


      :doc:`NESTML documentation <nestml:index>` :octicon:`link-external`

      :ref:`Install docker container for NEST + NESTML <docker>`




----


If installation didn't work, see the :ref:`troubleshooting section <troubleshooting>`.


.. toctree::
   :hidden:
   :glob:

   mac_install
   hpc_install
   livemedia
   cmake_options
   *

.. |user| image:: ../static/img/020-user.svg
.. |teacher| image:: ../static/img/014-teacher.svg
.. |admin| image:: ../static/img/001-shuttle.svg
.. |dev| image:: ../static/img/dev_orange.svg
.. |desktop| image:: ../static/img/nestdesktop022023.svg
.. |nestml| image:: ../static/img/nestml022023.svg
