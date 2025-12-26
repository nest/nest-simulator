.. _install_nest:

Install and run NEST
====================

.. grid::
   :gutter: 1


   .. grid-item-card:: |user| I want to run NEST on my laptop
       :class-title: sd-d-flex-row sd-align-minor-center

       You can ``pip install nest-simulator``

       See our :ref`detailed instructions` including installing NEST with
       |nestml| and |desktop|.

       .. dropdown:: Other options for pre-packaged NEST
          :class-title: sd-font-weight-light sd-fs-7 sd-text-secondary
          :class-container: sd-align-major-center
          :color: light

          * :ref:`Docker (cross-platform) <docker>`
            (Includes NESTML)
          * :ref:`conda-forge (Linux/macOS) <conda_forge_install>`
          * :ref:`Ubuntu PPA (Linux) <ubuntu_install>`
          * :ref:`Homebrew (macOS) <macos_install>`
          * :ref:`Options for Windows users <windows_install>`

   .. grid-item-card:: |dev| I want to do development with NEST
      :class-title: sd-d-flex-row sd-align-minor-center

      For customizing the build (e.g., adding MPI support), you can

      * :ref:`dev_install`


.. grid::
   :gutter: 1


   .. grid-item-card:: |launch| I want to try out NEST without installing anything
      :class-title: sd-d-flex-row sd-align-minor-center

      With the JupyterHub service from EBRAINS, you can
      try out NEST (and many other tools) directly in your browser.

      .. rst-class:: imgbutton

         .. image:: https://nest-simulator.org/TryItOnEBRAINS.png
            :target: https://lab.ebrains.eu/hub/user-redirect/git-pull?repo=https%3A%2F%2Fgithub.com%2Fnest%2Fnest-simulator-examples&urlpath=lab%2Ftree%2Fnest-simulator-examples%2Fnotebooks%2Fnotebooks%2Fone_neuron.ipynb&branch=main


      * For more info see :ref:`our guide to running Jupyter notebooks on EBRAINS <run_jupyter>`


   .. grid-item-card:: |teach| I want to use NEST in the classroom/for a workshop
      :class-title: sd-d-flex-row sd-align-minor-center


      Use |desktop| -
      a graphical user interface, ideal for learning and teaching concepts
      regarding neural networks in classrooms and workshops.

      * :doc:`Lecturer guide <desktop:lecturer/index>`
      * :ref:`Introduction to PyNEST tutorial <pynest_tutorial>`
      * :doc:`NESTML tutorials <nestml:tutorials/index>`



.. grid::
   :gutter: 1

   .. grid-item-card:: |hpc| I want to run NEST on an HPC system
     :class-title: sd-d-flex-row sd-align-minor-center
     :columns: 6

      We have provided some helpful guides to understanding and optimizing NEST
      in HPC environments:

      * :ref:`optimize_performance`
      * :ref:`parallel_computing`


What's next?
-------------


.. grid::
   :gutter: 1

   .. grid-item-card::
     :columns: 6

     See our :ref:`tutorials and guides <tutorials_guides>` to get started with NEST
     or
     explore are vast catalog of :ref:`examples <pynest_examples>`.

----

If installation didn't work, see the :ref:`troubleshooting section <troubleshooting>`.


.. toctree::
   :hidden:
   :glob:

   mac_install
   livemedia
   cmake_options
   *

.. |user| image:: ../static/img/020-user.svg
.. |teacher| image:: ../static/img/014-teacher.svg
.. |launch| image:: ../static/img/001-shuttle.svg
.. |dev| image:: ../static/img/dev_orange.svg
.. |desktop| image:: ../static/img/nestdesktop022023.svg
   :width: 120px
   :target: https://nest-desktop.readthedocs.io/en/latest/
.. |nestml| image:: ../static/img/nestml022023.svg
   :width: 80px
   :target: https://nestml.readthedocs.io/en/latest/
.. |hpc| image:: ../static/img/hpc_orange128.svg
.. |teach| image:: ../static/img/014-teacher.svg
