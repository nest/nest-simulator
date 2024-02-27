Welcome to the NEST Simulator documentation
===========================================




.. grid::

  .. grid-item::

    NEST is used in computational neuroscience to model and study behavior of large networks of neurons.

    The models describe single neuron and synapse behavior and their connections.
    Different mechanisms of plasticity can be used to investigate artificial learning
    and help to shed light on the fundamental principles of how the brain works.

    NEST offers convenient and efficient commands to define and connect large networks,
    ranging from algorithmically determined connections to data-driven connectivity.
    Create connections between neurons using numerous synapse models from STDP to gap junctions.

    .. button-ref:: tutorials_guides
       :ref-type: ref
       :shadow:
       :color: primary

       Get started with NEST

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


                  Investigate neuron and astrocyte interactions


                .. figure:: examples/hpc_benchmark_connectivity.svg


                  Test perfomance and benchmarks

                .. figure:: static/img/pong_sim.gif


                  Play PONG with NEST

                .. figure:: static/img/gapjunctions.png


                  Explore synapse types like gap junctions

       .. grid-item::

          .. button-ref:: pynest_examples
             :ref-type: ref
             :color: info
             :align: center
             :shadow:

             Discover all our examples!


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
