.. _tutorials_guides:

Tutorials and Guides
====================


|start| First steps: Learn how to use NEST
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


.. grid:: 1 1 3 3
    :gutter: 1

    .. grid-item-card::
        :class-header: sd-text-secondary

        PyNEST Tutorial
        ^^^

        A :ref:`step-by-step tutorial <pynest_tutorial>` that introduces NEST concepts
        and allows you to develop your first script.

    .. grid-item-card::
        :class-header: sd-text-secondary

        A basic one neuron example
        ^^^

        :doc:`One neuron example </auto_examples/one_neuron>`: A breakdown of a basic NEST script using one neuron,
        to showcase the basic structure of a simulation.

    .. grid-item-card::
        :class-header: sd-text-secondary

        Interactive graphical interface
        ^^^

        :doc:`Interactive GUI <desktop:index>`: If you prefer a graphical approach, NEST Desktop offers an interactive graphical
        format for creating neural networks.

|write| Next steps: Start developing your script
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. grid:: 1 1 3 3
    :gutter: 1

    .. grid-item-card::
         :class-header: sd-text-secondary
         :columns: 4

         Create models
         ^^^

         * :ref:`modelsmain`: Discover the available models in NEST
           or create and customize models with :doc:`NESTML <nestml:index>`


    .. grid-item-card::
         :class-header: sd-text-secondary
         :columns: 5

         Connect neurons, synapses, and devices
         ^^^

         * :ref:`connection_management`: A guide to building connections in NEST

    .. grid-item-card::
         :class-header: sd-text-secondary
         :columns: 3

         Simulate the network
         ^^^

         * :ref:`run_simulations`: A guide describing various factors in running simulations


    .. grid-item-card::
         :class-header: sd-text-secondary

         Collect data
         ^^^

         * :ref:`record_simulations`: How to collect data from neurons
           and synapses.

.. seealso::


   Check out our :ref:`numerous example PyNEST scripts <pynest_examples>`


|script| Pointers for writing your PyNEST script
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. grid:: 1 1 3 3
    :gutter: 1

    .. grid-item-card::
       :class-header: sd-text-secondary

       Handling nodes
       ^^^

       * :ref:`Manipulating nodes (neurons and devices) <node_handles>`: understand basic functionality of nodes

    .. grid-item-card::
       :class-header: sd-text-secondary

       Handling connections
       ^^^

       * :ref:`Manipulating connections (synapses) <handling_connections>`: understand basic fucntionality of connections

    .. grid-item-card::
       :class-header: sd-text-secondary

       Parametrization
       ^^^

       * :ref:`param_ex`: explore how to use parameter objects in NEST



----

Additional topics
~~~~~~~~~~~~~~~~~


.. grid:: 1 1 2 3
   :gutter: 1

   .. grid-item-card::
       :class-header: sd-d-flex-row sd-align-minor-center sd-bg-info sd-text-white

       |neuron| More about neurons
       ^^^

       * :ref:`sim_precise_spike_times`
       * :ref:`exact_integration`

   .. grid-item-card::
       :class-header: sd-d-flex-row sd-align-minor-center sd-bg-info sd-text-white

       |synapse|  More about synapses
       ^^^


       * :ref:`sim_gap_junctions`
       * :ref:`weight_normalization`


   .. grid-item-card::
       :class-header: sd-d-flex-row sd-align-minor-center sd-bg-info sd-text-white

       |device| More about devices
       ^^^

       * :ref:`stimulate_network`: An overview of various stimulation devices.

.. grid:: 1 1 2 3
   :gutter: 1


   .. grid-item-card::
       :class-header: sd-d-flex-row sd-align-minor-center sd-bg-dark sd-text-white

       |network| Networks
       ^^^

       * :ref:`spatial_networks`: A deep dive into building 2D and 3D networks
       * :doc:`The microcircuit model <../auto_examples/Potjans_2014/index>`: A network model example based on Potjans and Diesman 2014.
       * The `multi-area model <https://inm-6.github.io/multi-area-model/>`_: A network model of the visual cortex of the macaque monkey.

   .. grid-item-card::
       :class-header: sd-d-flex-row sd-align-minor-center sd-bg-dark sd-text-white

       |hpc| High performance computers (HPC)
       ^^^

       * :ref:`parallel_computing`: How NEST handles thread parallel and distributed computing
       * :ref:`optimize_performance`: Guides to optimize NEST performance in large scale simulations
       * :ref:`benchmark`: How to use the beNNch framework.

   .. grid-item-card::
       :class-header: sd-d-flex-row sd-align-minor-center sd-bg-dark sd-text-white

       |random| NEST behavior
       ^^^

       * :ref:`built_in_timers`: A guide to the various timers available in NEST.
       * :ref:`random_numbers`: A guide to how random number are used in network simulations.


.. grid:: 1 1 2 3
   :gutter: 1


   .. grid-item-card::
       :class-header: sd-d-flex-row sd-align-minor-center sd-bg-success sd-text-white

       |sonata| NEST SONATA guide
       ^^^

       * :ref:`SONATA with NEST <nest_sonata>`: Guide to using the SONATA format

   .. grid-item-card::
       :class-header: sd-d-flex-row sd-align-minor-center sd-bg-success sd-text-white

       |connect| Connect to other tools
       ^^^

       * :ref:`nest_server`: A guide showcasing how to interact with NEST via a RESTful API.
       * :ref:`MUSIC Tutorial <music_tutorial_1>`: how to transmit data between applications
         with the MUSIC interface, step-by-step.
       * :ref:`nest_music`: Additional guide for building scripts with MUSIC


   .. grid-item-card::
       :class-header: sd-d-flex-row sd-align-minor-center sd-bg-success sd-text-white


       |refresh| Get the latest changes
       ^^^

       * :ref:`Update 2.X scripts to 3.X <refguide_2_3>`
       * :ref:`See the latest changes <whats_new>`



.. toctree::
   :maxdepth: 1
   :hidden:

   tutorials/index

.. toctree::
   :maxdepth: 1
   :hidden:

   Neurons <neurons/index>
   Synapses and connections <synapses/index>
   Devices  <devices/index>
   Spatially-structured networks <networks/spatially_structured_networks>
   High performance computing <hpc/index>
   NEST models <models/models-main>
   NEST and SONATA <nest_sonata/nest_sonata_guide>
   Simulation behavior <nest_behavior/running_simulations>
   Randomness in NEST <nest_behavior/random_numbers>
   Built-in timers <nest_behavior/built-in_timers>
   Connect NEST with other tools <connect_nest/index>
   From NEST 2.x to 3.x <whats_new/v3.0/refguide_nest2_nest3>




.. |nav| image:: static/img/GPS-Settings-256_nest.svg
.. |script| image:: static/img/script_orange.svg
      :scale: 20%
.. |start| image:: static/img/start_orange.svg
      :scale: 40%
.. |user| image:: static/img/020-user.svg
.. |teacher| image:: static/img/014-teacher.svg
.. |admin| image:: static/img/001-shuttle.svg
.. |dev| image:: static/img/dev_orange.svg
.. |nestml| image:: static/img/nestml-logo.png
      :scale: 15%
.. |synapse| image:: static/img/synapse_white.svg
.. |neuron|  image:: static/img/neuron_white.svg
.. |glossary|  image:: static/img/glossary_white.svg
.. |git|  image:: static/img/git_white.svg
.. |refresh|  image:: static/img/refresh_white.svg
.. |hpc|  image:: static/img/hpc_white.svg
.. |random|  image:: static/img/random_white.svg
.. |math|  image:: static/img/math_white.svg
.. |network|  image:: static/img/network_brain_white.svg
.. |device|  image:: static/img/device_white.svg
.. |connect|  image:: static/img/connect_white.svg
.. |sonata|  image:: static/img/sonata_white.svg
.. |write|  image:: static/img/write_nest_orange.svg
      :scale: 60%
