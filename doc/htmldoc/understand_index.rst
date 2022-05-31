Understand how NEST works
=========================

Here you can dive into various topics about NEST.

.. grid:: 1 2 3 4
   :gutter: 1

   .. grid-item-card:: |neuronicon| All about neurons
       :columns: 4

       * :ref:`node_handles`

       * :ref:`param_ex`

       * :ref:`sim_precise_spike_times`

       * :ref:`exact_integration`

   .. grid-item-card:: |synapseicon| All about synapses and connections
       :columns: 5

       * :ref:`connection_management`

       * :ref:`handling_connections`

       * :ref:`sim_gap_junctions`

       * :ref:`weight_normalization`

   .. grid-item-card:: |deviceicon| All about devices
       :columns: 3

       * :ref:`record_simulations`

       * :ref:`stimulate_network`

.. grid:: 2

   .. grid-item-card:: |networkicon| Spatially-structured networks
       :columns: 5
       :link: spatial_networks 
       :link-type: ref


   .. grid-item-card:: |mathicon| Models in NEST
       :columns: 4
       :link:  models_contents
       :link-type: ref

.. grid::  3
   :gutter: 1

   .. grid-item-card:: |randomicon| NEST behavior
       :columns: 3

       * :ref:`built_in_timers`

       * :ref:`random_numbers`

       * :ref:`run_simulations`

   .. grid-item-card:: |connecticon| Connect to other tools
       :columns: 4

       * :ref:`nest_server`

       * :ref:`nest_music`


   .. grid-item-card:: |hpcicon| High performance computers (HPC) 
       :columns: 5
       :text-align: center
       :link: parallel_computing
       :link-type: ref



.. grid::  3
   :gutter: 1

   .. grid-item-card:: |converticon| Convert NEST 2.X scripts to 3.X
       :columns: 5
       :link: refguide_2_3
       :link-type: ref

   .. grid-item-card:: |releaseicon| Release notes
       :columns: 4
       :link: release_notes
       :link-type: ref

   .. grid-item-card:: |glossaryicon| Glossary 
       :columns: 3
       :link: glossary
       :link-type: ref


.. toctree::
   :maxdepth: 1
   :hidden:


   Neurons <neurons/index>
   Synapses and connections <synapses/index>
   Devices  <devices/index>
   Spatially-structured networks <networks/spatially_structured_networks>
   NEST models <models/index>
   Simulation behavior <nest_behavior/running_simulations>
   Randomness in NEST <nest_behavior/random_numbers>
   Built-in timers <nest_behavior/built-in_timers>
   Connect NEST with other tools <connect_nest/index>
   Parallel computing <hpc/parallel_computing>
   Benchmarking <hpc/benchmarking>
   From NEST 2.x to 3.x <release_notes/v3.0/refguide_nest2_nest3>
   Release notes <release_notes/index>
   Glossary <ref_material/glossary>


.. |synapseicon| image:: static/img/synapse_orange64.png
.. |networkicon| image:: static/img/brainnetwork_orange64.png
.. |modelicon| image:: static/img/math_orange64.png
.. |neuronicon| image:: static/img/neuron_orange_64.png
.. |deviceicon| image:: static/img/device_orange64.png
.. |randomicon| image:: static/img/random_orange64.png
.. |connecticon| image:: static/img/connect_orange64.png
.. |releaseicon| image:: static/img/version-control_orange64.png
.. |converticon| image:: static/img/refresh_orange64.png
.. |glossaryicon| image:: static/img/glossary_orange64.png
.. |hpcicon| image:: static/img/hpc_orange64.png
.. |mathicon| image:: static/img/math_orange64.png
