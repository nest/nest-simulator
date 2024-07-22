.. _nest_benchmark_results:

NEST performance benchmarks
===========================


NEST performance is continuously monitored and improved across various network sizes.
Here we show benchmarking results for NEST version 3.8 on Jureca-DC.


Strong scaling experiment of the Microcircuit model [1]_
---------------------------------------------------------

.. grid:: 1 1 1 1

   .. grid-item::
       :columns: 10
       :class: sd-align-major-center

       .. image:: /static/img/mc_benchmark.png

.. grid:: 1 1 1 1

   .. grid-item::
       :columns: 10
       :class: sd-align-minor-center


       * The model has ~80 000 neurons and ~300 million synapses
       * Increasing number of computing resources decrease simulation time
       * The model runs faster than real time




Strong scaling experiment of the Multi-area-model [2]_
-------------------------------------------------------

.. grid:: 1 1 1 1

 .. grid-item::
       :class: sd-align-major-center
       :columns: 10

       .. image:: /static/img/mam_benchmark.png


.. grid:: 1 1 1 1

 .. grid-item::
       :columns: 10
       :class: sd-align-minor-center

       * The model has ~3.2 million neurons and ~10 billion synapses
       * Steady decrease of run time with additional compute resources




Weak scaling experiment of the HPC benchmark model [3]_
--------------------------------------------------------

.. grid:: 1 1 1 1

   .. grid-item::
       :columns: 10
       :class: sd-align-major-center

       .. image:: /static/img/hpc_benchmark.png


.. grid:: 1 1 1 1

   .. grid-item::
       :columns: 10
       :class: sd-align-minor-center


       * The size of network scales proportionally with the computational resources used
       * Largest network size in this diagram: ~5.6 million neurons and ~61 billion synapses
       * The figure shows that NEST can handle massive networks and simulate them efficiently


.. seealso::

   Example networks:

   * :doc:`/auto_examples/Potjans_2014/index`
   * `Multi-area model <https://inm-6.github.io/multi-area-model/>`_
   * :doc:`/auto_examples/hpc_benchmark`

References
----------

.. [1]  Potjans TC. and Diesmann M. 2014. The cell-type specific cortical
        microcircuit: relating structure and activity in a full-scale spiking
        network model. Cerebral Cortex. 24(3):785–806. DOI: `10.1093/cercor/bhs358 <https://doi.org/10.1093/cercor/bhs358>`__.


.. [2] Schmidt M, Bakker R, Hilgetag CC, Diesmann M and van Albada SJ. 2018. Multi-scale
       account of the network structure of macaque visual cortex. Brain Structure
       and Function. 223: 1409 https://doi.org/10.1007/s00429-017-1554-4

.. [3] Jordan J, Ippen T, Helias M, Kitayama I, Sato M, Igarashi J, Diesmann M, Kunkel S. 2018.
       Extremely scalable spiking neuronal network simulation code: From laptops to exacale computers.
       Frontiers in Neuroinformatics. 12. https://www.frontiersin.org/journals/neuroinformatics/articles/10.3389/fninf.2018.00002
