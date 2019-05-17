Install NEST with conda-forge (Linux and MacOSX) - BETA -
============================================================

The nest-simulator is now available as a package on conda-forge.
If you don't have conda installed, follow the instructions on `miniconda <https://conda.io/miniconda.html>`__.

The conda-forge package contains a **basic install of NEST without** any extra configuration options
such as MPI. If you need to install NEST with these configuration options,
please see our :doc:`install guide for Linux <linux_install>` or :doc:`install guide for MacOSX <mac_install>`

.. admonition:: Important!

   The conda-forge package of nest-simulator is still being tested, let us know if you come across
   any problems by `submitting an issue on GitHub <https://github.com/nest/nest-simulator/issues>`_.


----

Linux install using conda-forge
--------------------------------

1. Create your conda environment:

.. code-block:: sh

  conda create --name ENVNAME

2. Activate your environment:

.. code-block:: sh

  conda activate ENVNAME

3. Install nest-simulator from conda-forge:

.. code-block:: sh

  conda install -c conda-forge nest-simulator


4. Once installation is complete, you can open up Python or IPython
   in the terminal and import nest:

.. code-block:: python
  :linenos:

  python
  import nest


If installation was successful, you should see the following output:

.. figure:: ../_static/img/import_nest.png
   :scale: 40%
   :alt: import nest

5. **The install is now complete!** :ref:`Start exploring NEST <after_install>`

-----

MacOSX install using conda-forge
----------------------------------

1. Create your conda environment:

.. code-block:: sh

    conda create --name ENVNAME

2. Activate your environment:

.. code-block:: sh

    conda activate ENVNAME

3. Install nest-simulator from conda-forge:

.. code-block:: sh

    conda install -c conda-forge nest-simulator


4. Once installation is complete you can now use nest via pythonw:


.. admonition:: Important!

     For NEST to run on OSX, a framework build of python is needed. This is not the
     default python available to conda, so you must use **pythonw** rather than python!

.. code-block:: python
    :linenos:

    pythonw
    import nest

.. _after_install:

Next Steps
------------

Once you have completed installation, take a look at our :doc:`PyNEST tutorials <../tutorials/index>` page
find out how to create your first simulation or checkout some of our :doc:`example networks <../examples/index>`!

----

Source Code:
https://github.com/conda-forge/nest-simulator-feedstock/

Anaconda cloud package:
https://anaconda.org/conda-forge/nest-simulator
