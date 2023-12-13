NEST Simulator documentation
============================


Welcome!
--------


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
              :show_controls:
              :show_fade:
              :data-bs-ride: carousel

                .. figure:: static/img/pong_sim.gif

                  Play PONG with NEST

                .. figure:: static/img/pynest/spatial_test3d.png

                  Create 3D spatially structured networks


                .. figure:: static/img/astrocyte_interaction.png

                  Investigate neuron and astrocyte interactions


       .. grid-item::

          .. button-ref:: pynest_examples
             :ref-type: ref
             :color: info
             :align: center
             :shadow:

             Click here to discover all our examples!

----

Conceptual approach
-------------------

.. mermaid::

   flowchart LR

     classDef nodeStyle color:#fff, stroke:#fff0, fill:#015491;
     classDef nodeStyle2 color:#fff, stroke:#fff0, fill:#072f42;
     classDef nodeStyle3 color:#222, stroke:#fff0, fill:#22222233;

     exp --> nest-simulator
     models -->nest-simulator
     nest-simulator --> act

     subgraph  exp [Experimental protocols]
      ir(input rates, input currents, <br> timed sequences, etc.):::nodeStyle3
     end
     subgraph nest-simulator
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
       smp(spike membrane potential, <br> synaptic weights, etc.):::nodeStyle3
     end

     class act sg
     class exp sg
     class models main
     class nest-simulator main
     classDef sg fill:#ddd, stroke:#4441, color:#111;
     classDef main fill:#fff0, stroke:#f63, color:#111, font-weight: bold, stroke-dasharray:5 10, stroke-width:3px;

   simulate[<img src='_static/img/nest_logo.png' /> Simulation \n ______________________]
   neuron_models[<img src='_static/img/neuron.svg' /> \n Neuron Models]
   synapse_models[<img src='_static/img/synapse.svg' /> \n Synapse Models]
   stimulating_devices[<img src='_static/img/stimulatelight.svg' /> Stimulating Devices]
   recording_devices[<img src='_static/img/recordinglight.svg' /> Recording Devices]
   network[<img src='_static/img/networkbrainlight.svg' /> \n Network Models]
   click network href "./networks/spatially_structured_networks.html"
   click neuron_models href "./models/index_neuron.html"
   click synapse_models href "./models/index_synapse.html"
   click stimulating_devices href "./models/index_generator.html"
   click recording_devices href "./models/index_device.html"
   click simulate href "./nest_behavior/running_simulations.html"




----


UML DIAGRAM

.. uml::

    !include https://raw.githubusercontent.com/plantuml-stdlib/C4-PlantUML/master/C4_Context.puml
    ' syntax see https://github.com/plantuml-stdlib/C4-PlantUML/blob/master/README.md#system-context--system-landscape-diagrams

    ' !include <C4/C4_Container>
    !define TUPADR https://raw.githubusercontent.com/tupadr3/plantuml-icon-font-sprites/master/
    !define FONTAWESOME https://raw.githubusercontent.com/tupadr3/plantuml-icon-font-sprites/master/font-awesome-5
    !include TUPADR/devicons2/bash.puml
    !include FONTAWESOME/file_invoice.puml
    !include FONTAWESOME/chart_bar.puml

    LAYOUT_LEFT_RIGHT()
    'LAYOUT_AS_SKETCH()
    HIDE_STEREOTYPE()

    AddBoundaryTag("nest", $borderColor="#ff6633", $borderThickness=3, $fontColor="#ff6633")


    Boundary(models, "Builtin or user provided choice of models") {
        Person(network, "network model", $sprite="img:https://www.nest-simulator.org/images/network_brain_white.png{scale=.25}", $descr="«connectome»")
        Person(neuron, "neuron models", $sprite="img:https://www.nest-simulator.org/images/neuron_text.png{scale=.3}", $link="https://nest-simulator--2969.org.readthedocs.build/en/2969/neurons/index.html", $descr="«ODEs»")
        Person(synapse, "synapse models", $sprite="img:https://www.nest-simulator.org/images/synapse_text.png{scale=.3}", $link="https://nest-simulator--2969.org.readthedocs.build/en/2969/synapses/index.html", $descr="«ODEs»")
    }

    Boundary(nestsim, "NEST Simulator", $tags="nest") {
        System(nest, "simulation", $sprite="img:https://www.nest-simulator.org/images/nest.png{scale=.6}")
        System(stimulation, "stimulation devices", $sprite="img:https://www.nest-simulator.org/images/device_text.png{scale=.3}")
        System(recording, "recording devices", $sprite="img:https://www.nest-simulator.org/images/device_text.png{scale=.3}")
    }

    SystemDb(protocol, "experimental\nprotocol", $sprite="file_invoice")
    SystemDb(activity, "activity data", $sprite="chart_bar")

    Rel_L(stimulation, nest, "stimulate")
    Rel_L(nest, recording, "measure")
    Rel(network, nest, "")
    Rel(neuron, network, "")
    Rel(synapse, network, "")

    Rel_U(protocol, stimulation, "input rates,\ninput currents,\ntimed sequences, …")
    Rel(recording, activity, "spikes,\nmembrane potentials,\nsynaptic weights, …")

.. toctree::
   :caption: USAGE
   :hidden:
   :glob:

   Install <installation/index>
   Tutorials and Guides <get-started_index>
   Examples <examples/index>
   PyNEST API <ref_material/pynest_api/index>
   Available models <models/index>
   ref_material/glossary
   Technical docs <developer_space/index>


.. toctree::
   :caption: COMMUNITY
   :hidden:
   :glob:

   Cite NEST <citing-nest>
   Contact us <community>
   Contribute <contribute>
   What's new? <whats_new/index>
   NEST Homepage <https://nest-simulator.org>

.. toctree::
   :caption: RELATED PROJECTS
   :hidden:

   NEST Desktop <https://nest-desktop.readthedocs.io/en/latest/>
   NESTML <https://nestml.readthedocs.io/en/latest/>
   NESTGPU <https://nestgpu.readthedocs.io/en/latest/>

.. toctree::
   :maxdepth: 1
   :caption: LICENSE
   :hidden:

   license
