Workflow using Snakemake
========================

A workflow that will run the scaling experiments comparing ``ignore_and_fire`` with the ``iaf_psc_alpha`` is available as a Snakefile.
If you want to reproduce the results,  you can use this workflow.

1. Ensure :ref:`NEST <install_nest>` and `snakemake <https://snakemake.readthedocs.io/en/stable/>`_ are installed in your active environment

2. Clone the NEST repository if you have not already done so.

   .. code-block:: sh

     git clone git@github.com:<your-username>/nest-simulator.git

3. In the nest-simulator repository, go to ``pynest/examples/ignore_and_fire/`` directory.

   .. code-block:: sh

     cd nest-simulator/pynest/examples/ignore_and_fire

4. Run ``snakemake -cN`` where N is the number of cores you want to use.


Get the :download:`Snakefile`


.. seealso::

  * :ref:`sec_model_description`
  * :doc:`/auto_examples/ignore_and_fire/index`
