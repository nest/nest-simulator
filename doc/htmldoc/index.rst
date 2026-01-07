Welcome to the NEST Simulator documentation!
============================================


.. grid::
  :gutter: 2

  .. grid-item::

     .. grid:: 1 1 1 1
       :gutter: 2

       .. grid-item::

          NEST is used in computational neuroscience to model and study behavior of large networks of neurons.

          The models describe single neuron and synapse behavior and their connections.
          Different mechanisms of plasticity can be used to investigate learning
          and help to shed light on the fundamental principles of how the brain works.

          NEST offers convenient and efficient commands to define and connect large networks,
          ranging from algorithmically determined connections to data-driven connectivity.
          Create connections between neurons using numerous synapse models from STDP to gap junctions.


       .. grid-item::

          .. button-ref:: tutorials_guides
             :ref-type: ref
             :shadow:
             :color: primary

             Start exploring NEST

  .. grid-item::

     .. grid:: 1 1 1 1
       :gutter: 2

       .. grid-item-card::

          .. carousel::
              :show_indicators:
              :show_fade:
              :show_dark:
              :data-bs-ride: carousel

                .. figure:: static/img/network_model_sketch_mesocircuit.png
                  :target: networks/index.html

                  Create large network models

                .. figure:: static/img/astrocyte_interaction.png
                  :target: auto_examples/astrocytes/index.html

                  Inspect neuron and astrocyte interactions

                .. figure:: static/img/hpc_benchmark_connectivity.svg
                  :target: auto_examples/hpc_benchmark.html

                  Test perfomance and benchmarks

                .. figure:: static/img/pong_sim.gif
                  :target: auto_examples/pong/run_simulations.html

                  Simulate a game of PONG with NEST

                .. figure:: static/img/pynest/eprop_supervised_classification_infrastructure.png
                  :target: auto_examples/eprop_plasticity/index.html

                  Explore eligibility propagation plasticity

       .. grid-item::

          .. button-ref:: pynest_examples
             :ref-type: ref
             :color: info
             :align: center
             :shadow:

             Discover all our examples!


How NEST works --- The Big Picture
----------------------------------

.. grid::

    .. grid-item::

        .. raw:: html

           <object data="_static/img/excalidraw_nestconcept_horiz.svg" type="image/svg+xml"></object>


.. grid::

    .. grid-item::


        A NEST simulation is created with input from :doc:`stimulation devices </devices/index>`,
        :doc:`neuron models </neurons/index>`, and :doc:`synapse models </synapses/index>`,
        along with  :ref:`connection rules <connectivity_concepts>`.
        You can choose what data to record with :doc:`recording devices </devices/index>`.
        After simulation, the output is ready for analysis with NEST's built in :py:mod:`.raster_plot` and :py:mod:`.voltage_trace`
        modules or external tools such as :doc:`Elephant <elephant:index>`.


        You can find these components in NEST or you can implement your own custom
        models and extend NEST's functionalities using :doc:`NESTML <nestml:index>` and the :doc:`NEST extension module <extmod:index>`, respectively.
        Check out our wide-ranging list of :doc:`network model <examples/index>` examples.


.. toctree::
   :caption: USAGE
   :hidden:
   :glob:

   Install <installation/index>
   Tutorials and Guides <get-started_index>
   Examples <examples/index>
   Models <models/index>
   Python API <ref_material/pynest_api/index>
   ref_material/glossary
   NEST performance results <https://performance-benchmarks.readthedocs.io/en/latest>
   Cite NEST <citing-nest>
   License <license>


.. toctree::
   :caption: COMMUNITY
   :hidden:
   :glob:

   Contact us <community>
   Contribute <developer_space/index>
   What's new? <whats_new/index>
   NEST Homepage <https://nest-simulator.org>
   Acknowledgments <https://github.com/nest/nest-simulator/blob/master/ACKNOWLEDGMENTS.md>

.. toctree::
   :caption: RELATED PROJECTS
   :hidden:

   NEST Desktop <https://nest-desktop.readthedocs.io/en/latest/>
   NESTML <https://nestml.readthedocs.io/en/latest/>
   NESTGPU <https://nest-gpu.readthedocs.io/en/latest/>
   NEST NEAT <https://nest-neat.readthedocs.io/en/latest/>
