Welcome to the NEST Simulator documentation
===========================================




.. grid::
  :gutter: 2

  .. grid-item::

     .. grid:: 1 1 1 1
       :gutter: 2

       .. grid-item::

          NEST is used in computational neuroscience to model and study behavior of large networks of neurons.

          The models describe single neuron and synapse behavior and their connections.
          Different mechanisms of plasticity can be used to investigate artificial learning
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


                  Create spatially structured networks


                .. figure:: static/img/astrocyte_interaction.png


                  Inspect neuron and astrocyte interactions


                .. figure:: static/img/hpc_benchmark_connectivity.svg


                  Test perfomance and benchmarks

                .. figure:: static/img/pong_sim.gif


                  Simulate a game of PONG with NEST

                .. figure:: static/img/gapjunctions.png


                  Explore synapse types like gap junctions

       .. grid-item::

          .. button-ref:: pynest_examples
             :ref-type: ref
             :color: info
             :align: center
             :shadow:

             Discover all our examples!

.. grid::

    .. grid-item::
        :columns: 6
        :child-direction: row
        :child-align: end

        .. raw:: html

           <object width="89%" height="89%" data="_static/img/excalidraw_nestconcept.svg" type="image/svg+xml"></object>


    .. grid-item::
        :columns: 5

        **How NEST works - Conceptual approach**

        This diagram provides an overview of the components essential for building and simulating a network model with NEST:
        The input from stimulation devices, neuron  and synapse models,
        and connectivity rules.
        You can choose what data to record with the recording device. After simulation, the
        output is ready for analysis with external tools.

        NEST offers a comprehensive set of predefined components, including over a 100 :doc:`neuron models </models/index_neuron>` and :doc:`synapse models </models/index_synapse>`,
        a variety of :ref:`connection rules <connectivity_concepts>`, :doc:`stimulation devices </models/index_generator>`, and :doc:`recording devices </models/index_recorder>`. Plus,
        NEST is flexible, allowing you to implement your own custom
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
