.. _tutorials_guides:

Tutorials and Guides
====================


First steps: Learn how to use NEST
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


.. grid:: 1 1 3 3
    :gutter: 1

    .. grid-item-card::
        :class-header: sd-d-flex-row sd-align-minor-center sd-bg-primary sd-text-white

        |start| PyNEST Tutorial
        ^^^

        A :ref:`step-by-step tutorial <pynest_tutorial>` that introduces NEST concepts
        and allows you to develop your first script.

    .. grid-item-card::
        :class-header: sd-d-flex-row sd-align-minor-center sd-bg-primary sd-text-white

        |write| A basic one neuron example
        ^^^

        :doc:`One neuron example </auto_examples/one_neuron>`: A breakdown of a basic NEST script using one neuron,
        to showcase the basic structure of a simulation.

    .. grid-item-card::
        :class-header: sd-d-flex-row sd-align-minor-center sd-bg-primary sd-text-white

        |interactive| Interactive graphical interface
        ^^^

        :doc:`NEST Desktop <desktop:index>`: If you prefer a graphical approach, NEST Desktop offers an interactive graphical
        format for creating neural networks.

Next steps: Create your own network model
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. grid:: 1 1 3 3
    :gutter: 1

    .. grid-item-card::
         :class-header: sd-d-flex-row sd-align-minor-center sd-bg-info sd-text-white
         :columns: 4

         |math| Create neurons, synapses, and devices
         ^^^

         * :ref:`modelsmain`: Discover the available models in NEST
           or create and customize models with :doc:`NESTML <nestml:index>`


    .. grid-item-card::
         :class-header: sd-d-flex-row sd-align-minor-center sd-bg-info sd-text-white
         :columns: 4

         |random| Connect neurons, synapses, and devices
         ^^^
         * :ref:`connectivity_concepts`: A guide to define network connectivity in NEST
         * :ref:`spatial_networks`: A deep dive into building 2D and 3D networks
         * :ref:`connection_generator`: Using an external library for generating connections
         * :ref:`synapse_spec`: Details on parameterizing synapses

    .. grid-item-card::
         :class-header: sd-d-flex-row sd-align-minor-center sd-bg-info sd-text-white
         :columns: 4

         |device| Device management
         ^^^

         * :ref:`stimulate_network`: An overview of how to stimulate the network
         * :ref:`Get data from simulations <record_simulations>`: How to record data from neurons
           and synapses


    .. grid-item-card::
         :class-header: sd-d-flex-row sd-align-minor-center sd-bg-dark sd-text-white

         |simulate| Simulate your network
         ^^^

         * :ref:`run_simulations`: A guide describing various factors in running simulations
         * :ref:`random_numbers`: A guide to how random numbers are used in network simulations


    .. grid-item-card::
         :class-header: sd-d-flex-row sd-align-minor-center sd-bg-dark sd-text-white

         |python| Handling nodes and connections
         ^^^

         * :ref:`Manipulating nodes (neurons and devices) <node_handles>`: understand basic functionality of nodes
         * :ref:`Manipulating connections (synapses) <handling_connections>`: understand basic functionality of connections
         * :ref:`param_ex`: explore how to use parameter objects in NEST



    .. grid-item-card::
         :class-header: sd-d-flex-row sd-align-minor-center sd-bg-dark sd-text-white

         |gallery| Example gallery
         ^^^

         * Explore our :ref:`PyNEST example networks <pynest_examples>` that showcase the numerous features and models
           in NEST

    .. grid-item-card::
         :class-header: sd-d-flex-row sd-align-minor-center sd-bg-success sd-text-white

         |network| Large network models
         ^^^

         Discover how to build large and more complex network models with these examples:

         * :doc:`Brunel network </auto_examples/brunel_alpha_nest>`: A random balanced network
         * :doc:`The microcircuit  model <../auto_examples/Potjans_2014/index>`: A network model example based on Potjans and Diesman 2014
         * `The mesocircuit model <https://mesocircuit-model.readthedocs.io/en/latest/>`_: A layered cortical network with distance-dependent connectivity
         * The `multi-area model <https://inm-6.github.io/multi-area-model/>`_: A network model of the visual cortex of the macaque monkey



    .. grid-item-card::
         :class-header: sd-d-flex-row sd-align-minor-center sd-bg-success sd-text-white

         |parallel| Parallel computing
         ^^^

         * :ref:`parallel_computing`: How NEST handles thread parallel and distributed computing

----

More topics
~~~~~~~~~~~


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

       |hpc| High performance computers (HPC)
       ^^^

       * :ref:`optimize_performance`: Guides to optimize NEST performance in large scale simulations
       * :ref:`built_in_timers`: A guide to the various timers available in NEST
       * :ref:`benchmark`: How to use the beNNch framework





.. grid:: 1 1 2 3
   :gutter: 1



   .. grid-item-card::
       :class-header: sd-d-flex-row sd-align-minor-center sd-bg-dark sd-text-white

       |connect| Connect to other tools
       ^^^

       * :ref:`nest_server`: A guide showcasing how to interact with NEST via a RESTful API
       * :ref:`SONATA with NEST <nest_sonata>`: Guide to using the SONATA format
       * :ref:`MUSIC Tutorial <music_tutorial_1>`: how to transmit data between applications
         with the MUSIC interface, step-by-step
       * :ref:`nest_music`: Additional guide for building scripts with MUSIC


   .. grid-item-card::
       :class-header: sd-d-flex-row sd-align-minor-center sd-bg-dark sd-text-white


       |refresh| Get the latest changes
       ^^^

       * :ref:`Update 2.X scripts to 3.X <refguide_2_3>`
       * :ref:`See the latest changes <whats_new>`


.. toctree::
   :maxdepth: 1
   :hidden:

   tutorials/index
   tutorials/pynest_tutorial/index

.. toctree::
   :maxdepth: 1
   :hidden:

   Neurons <neurons/index>
   Synapses and connections <synapses/index>
   Devices  <devices/index>
   Spatially-structured networks <networks/spatially_structured_networks>
   High performance computing <hpc/index>
   NEST models <models/index>
   NEST and SONATA <nest_sonata/nest_sonata_guide>
   Simulation behavior <nest_behavior/running_simulations>
   Randomness in NEST <nest_behavior/random_numbers>
   Built-in timers <nest_behavior/built-in_timers>
   Connect NEST with other tools <connect_nest/index>
   From NEST 2.x to 3.x <whats_new/v3.0/refguide_nest2_nest3>

.. toctree::
   :hidden:

.. |nav| image:: static/img/GPS-Settings-256_nest.svg
.. |script| image:: static/img/script_white.svg
      :scale: 20%
.. |start| image:: static/img/start_white.svg
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
.. |write|  image:: static/img/write_nest_white.svg
      :scale: 60%
.. |parallel| image:: static/img/parallel_white.svg
.. |simulate| image:: static/img/simulate_white.svg
.. |interactive| image:: static/img/interactive_white.svg
.. |python| image:: static/img/python_white.svg
.. |gallery| image:: static/img/gallery_white.svg
