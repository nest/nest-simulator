.. _dev_install:

Install NEST from source
=========================


.. note::

    Please see our :ref:`development workflows and guidelines <developer_space>`, if you need
    a refresher in git or need to review the coding or documentation guidelines.



* Clone nest-simulator from Github `<https://github.com/nest/nest-simulator>`_:

.. code-block:: sh

   git clone git@github.com:<your-username>/nest-simulator.git


* or download the tarball `here <https://github.com/nest/nest-simulator/releases>`_ and unpack it:

.. code-block:: sh

    tar -xzvf nest-simulator-x.y.tar.gz



We have provided an `environment.yml <https://github.com/nest/nest-simulator/blob/master/environment.yml>`_
file that contains all possible packages needed for NEST development.

.. grid:: 2

   .. grid-item-card:: Install NEST with conda

         See our instructions for installing NEST from source in a :ref:`conda environment <condaenv>`

   .. grid-item-card:: Install NEST without environment

         If you want to install NEST without any environment, see the :ref:`instructions here <noenv>`.

What gets installed where
-------------------------

By default, everything will be installed to the subdirectories ``<nest_install_dir>/{bin,lib,share}``, where
``/install/path`` is the install path given to ``cmake``:

- Executables ``<nest_install_dir>/bin``
- Dynamic libraries ``<nest_install_dir>/lib/``
- SLI libraries ``<nest_install_dir>/share/nest/sli``
- SLI documentation ``<nest_install_dir>/share/doc/nest``
- Examples ``<nest_install_dir>/share/doc/nest/examples``
- PyNEST ``<nest_install_dir>/lib/pythonX.Y/site-packages/nest``
- PyNEST examples ``<nest_install_dir>/share/doc/nest/examples/pynest``

If you want to run the ``nest`` executable or use the ``nest`` Python module without providing explicit paths, you
have to add the installation directory to your search paths.


