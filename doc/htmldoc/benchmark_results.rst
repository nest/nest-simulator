.. _nest_benchmark_results:

NEST performance benchmarks
===========================


NEST performance is continuously monitored and improved across various network sizes.
Here we show benchmarking results for NEST version 3.8 on Jureca-DC [1]_.
The benchmarking framework and the structure of the graphs is described in [2]_.

Strong scaling experiment of the Microcircuit model [3]_
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


       * The model has ~80 000 neurons and ~300 million synapses, minimal delay 0.1 ms
       * 2 MPI processes per node, 64 threads per MPI process
       * Increasing number of computing resources decrease simulation time
       * Data averaged over 3 runs with different seeds, error bars indicate standard deviation
       * The model runs faster than real time [4]_




Strong scaling experiment of the Multi-area-model [5]_
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

       * The model has ~4.1 million neurons and ~24 billion synapses, minimal delay 0.1 ms
       * 2 MPI processes per node, 64 threads per MPI process
       * Steady decrease of run time with additional compute resources
       * Data averaged over 3 runs with different seeds, error bars indicate standard deviation




Weak scaling experiment of the HPC benchmark model [6]_
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
       * Largest network size in this diagram: ~5.8 million neurons and ~65 billion synapses, minimal delay 1.5 ms
       * 2 MPI processes per node, 64 threads per MPI process
       * The figure shows that NEST can handle massive networks and simulate them efficiently
       * Data averaged over 3 runs with different seeds, error bars indicate standard deviation


.. seealso::

   Example networks:

   * :doc:`/auto_examples/Potjans_2014/index`
   * `Multi-area model <https://inm-6.github.io/multi-area-model/>`_
   * :doc:`/auto_examples/hpc_benchmark`

References
----------

.. [1]  Juelich Supercomputing Centre. 2021.  JURECA: Data Centric and Booster Modules implementing the Modular
        Supercomputing Architecture at Jülich Supercomputing Centre. Journal of large-scale research facilities,
        7, A182. DOI: http://dx.doi.org/10.17815/jlsrf-7-182


.. [2]  Albers J, Pronold J, Kurth AC, Vennemo SB, Haghighi Mood K, Patronis A, Terhorst D, Jordan J, Kunkel S,
        Tetzlaff T, Diesmann M and Senk J (2022). A Modular Workflow for Performance Benchmarking of Neuronal Network Simulations.
        Frontiers in Neuroinformatics(16):837549. https://doi.org/10.3389/fninf.2022.837549

.. [3]  Potjans TC. and Diesmann M. 2014. The cell-type specific cortical
        microcircuit: relating structure and activity in a full-scale spiking
        network model. Cerebral Cortex. 24(3):785–806. DOI: `10.1093/cercor/bhs358 <https://doi.org/10.1093/cercor/bhs358>`__.

.. [4]  Kurth AC, Senk J, Terhorst D, Finnerty J, Diesmann M. 2022. Sub-realtime simulation of a neuronal network of natural density.
        Neuromorphic computing and engineering 2(2), 021001
        https://iopscience.iop.org/article/10.1088/2634-4386/ac55fc/meta

.. [5]  Schmidt M, Bakker R, Hilgetag CC, Diesmann M and van Albada SJ. 2018. Multi-scale
        account of the network structure of macaque visual cortex. Brain Structure
        and Function. 223: 1409 https://doi.org/10.1007/s00429-017-1554-4

.. [6]  Jordan J, Ippen T, Helias M, Kitayama I, Sato M, Igarashi J, Diesmann M, Kunkel S. 2018.
        Extremely scalable spiking neuronal network simulation code: From laptops to exacale computers.
        Frontiers in Neuroinformatics. 12. https://www.frontiersin.org/journals/neuroinformatics/articles/10.3389/fninf.2018.00002
