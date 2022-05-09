Understand how NEST works
=========================

Here you can dive into various topics about NEST.

    .. :ref:`Neurons <neurons_index>`
       :ref:`Synapses and connections <synapse_index>`
       :ref:`Devices <device_index>`
       :ref:`Spatially-structured networks <spatial_networks>`

.. grid:: 3
   :gutter: 1

   .. grid-item-card::
       :img-background: static/img/neuron.png
 
       All about neurons
       ^^^^^^^^^^^^^^^^^

       :ref:`node_handles`

       :ref:`param_ex`

       :ref:`sim_precise_spike_times`

       :ref:`exact_integration`

   .. grid-item-card:: 
       :img-background: static/img/synapse1_sm.png

       All about synapses and connections
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
       :ref:`connection_management`

       :ref:`handling_connections`

       :ref:`sim_gap_junctions`

       :ref:`weight_normalization`

   .. grid-item-card:: All about devices

       :ref:`record_simulations`

       :ref:`stimulate_network`

.. grid:: 2
   :gutter: 1

   .. grid-item-card:: Spatially-structured networks 
       :img-top: static/img/synapse1_sm.png
 
       Header
       ^^^^^^

       
       :ref:`Spatially-structured networks <spatial_networks>`

   .. grid-item-card:: Models in NEST
       :img-bottom: static/img/synapse1_sm.png

        .. button-link:: models/models-main.html
            :color: primary

            Explore NEST models

.. grid::  3
   :gutter: 1

   .. grid-item-card::  NEST behavior

       * :ref:`built_in_timers`

       * :ref:`random_numbers`

       * :ref:`run_simulations`

   .. grid-item-card:: Connect to other tools

       * :ref:`nest_server`

       * :ref:`nest_music`

   .. grid-item-card:: High performance computers (HPC) 

       * :ref:`parallel_computing`



.. grid::  3
   :gutter: 1

   .. grid-item-card:: Convert NEST 2.X scripts to 3.X

        :ref:`refguide_2_3`

   .. grid-item-card:: Release notes

       :ref:`release_3.3`

       :ref:`release_3.2`

       :ref:`release_3.1`

       :ref:`release_3.0`

   .. grid-item-card:: Glossary



     .. link-button:: glossary.html
       :text: Go to glossary
       :classes: btn-success

.. toctree::
   :maxdepth: 1
   :hidden:


   From NEST 2.x to 3.x <release_notes/v3.0/refguide_nest2_nest3>
   NEST models <models/models-main>
   Neurons <neurons/index>
   Synapses and connections <synapses/index>
   Devices  <devices/index>
   Spatially-structured networks <networks/spatially_structured_networks>
   Simulation behavior <nest_behavior/running_simulations>
   Randomness in NEST <nest_behavior/random_numbers>
   Built-in timers <nest_behavior/built-in_timers>
   Connect NEST with other tools <connect_nest/index>
   Parallel computing <hpc/parallel_computing>
   Benchmarking <hpc/benchmarking>
   Release notes <release_notes/index>
   Glossary <ref_material/glossary>
