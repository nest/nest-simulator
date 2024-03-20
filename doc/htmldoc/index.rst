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


                  Investigate neuron and astrocyte interactions


                .. figure:: static/img/hpc_benchmark_connectivity.svg


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


Conceptual diagram
------------------

.. mermaid::

   %%{
     init:{
       "theme": "base",
       "themeVariables": {
          "lineColor": "#f63",
          "fontSize" : "18px"
       }
     }
    }%%
   flowchart LR

     classDef nodeStyle color:#fff, stroke:#fff0, fill:#0E6A93;
     classDef nodeStyle2 color:#000, stroke:#fff0, fill:#072f4200;
     classDef nodeStyle3 color:#222, stroke:#fff0, fill:#bbb0;

     exp --> nest-simulator
     models -->nest-simulator
     nest-simulator --> act

     subgraph  exp [Experimental protocols]
      ir:::nodeStyle3
     end
     subgraph nest-simulator [nest-simulator]
       direction TB
       stimulating_devices:::nodeStyle2 --> simulate
       simulate:::nodeStyle2 --> recording_devices:::nodeStyle2
     end

     subgraph models [Built-in or user provided models]
        direction LR
        neuron_models:::nodeStyle --> network:::nodeStyle
        synapse_models:::nodeStyle --> network

     end
     subgraph act [Activity data]
       smp:::nodeStyle3
     end

     class act sg
     class exp sg
     class models bg_models
     class nest-simulator bg_devices
     classDef sg fill:#bbb4, stroke:#4441, color:#111;
     classDef bg_models fill:#bbb7, stroke:#3330, color:#000, font-weight: bold;
     classDef bg_devices fill:#bbb7, stroke:#3330, color:#000, font-weight: bold;

   %% Note that the spaces used in the "Simulation" text are the copy pasted unicode space character.
   %% HTML spaces like %emsp; do not work
   simulate(<img src="_static/img/nest_logo.png"/>     Simulation     \n)
   neuron_models(<img src="_static/img/neuron.svg" /> \n  Neuron Models)
   synapse_models(<img src="_static/img/synapse.svg" /> \n Synapse Models )
   stimulating_devices(<img src="_static/img/stimulatelight.svg" /> Stimulating Devices)
   recording_devices(<img src="_static/img/recordinglight.svg" />  Recording Devices )
   network(<img src="_static/img/networkbrainlight.svg" /> \n  Network  Models )
   smp(<img src="_static/img/pynest/mc_neuron_sm.png" /> \n spikes, membrane potential, \n synaptic weights, etc.)
   ir(<img src="_static/img/experiment.png"/> \n Input rates, input currents, \n timed sequences, etc.)
   click network href "./networks/index.html"
   click neuron_models href "./models/index_neuron.html"
   click synapse_models href "./models/index_synapse.html"
   click stimulating_devices href "./models/index_generator.html"
   click recording_devices href "./models/index_device.html"
   click simulate href "./nest_behavior/running_simulations.html"

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
