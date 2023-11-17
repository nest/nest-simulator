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


                .. figure:: static/img/astrocyte_tripartite.png

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

Description of this diagram. Click the image below to discover more!

.. mermaid::

   flowchart TB

     classDef nodeStyle color:#fff, stroke:#fff0, fill:#015491;
     classDef nodeStyle2 color:#fff, stroke:#fff0, fill:#072f42;
     classDef nodeStyle3 color:#222, stroke:#fff0, fill:#22222233;

     subgraph models [Built-in or user provided models]
        direction RL
        neuron_models:::nodeStyle --> network:::nodeStyle
        synapse_models:::nodeStyle --> network

     end
     subgraph nest-simulator
       direction TB
       stimulating_devices:::nodeStyle2 --> simulate
       simulate:::nodeStyle2 --> recording_devices:::nodeStyle2
     end
     models -->nest-simulator

     exp(Experimental protocols)
     act
    recording_devices --> act(Activity data):::nodeStyle3
    exp:::nodeStyle3 --> stimulating_devices

   simulate[<img src='_static/img/nest_logo.png' /> Simulation \n ______________________]
   neuron_models[<img src='_static/img/neuron.svg' /> \n Neuron Models]
   synapse_models[<img src='_static/img/synapse.svg' /> \n Synapse Models]
   stimulating_devices[<img src='_static/img/stimulatelight.svg' /> Stimulating Devices]
   recording_devices[<img src='_static/img/recordinglight.svg' /> Recording Devices]
   network[<img src='_static/img/networkbrainlight.svg' /> \n Network Models]
   click network href "https://nest-simulator.org"
   click neuron_models href "/models/index.html"
   click synapse_models href "file:///home/mitchell/Work/build-repo/doc/_build/html/understand_index.html"
   click stimulating_devices href "https://nest-simulator.org"
   click recording_devices href "https://nest-simulator.org"
   click stimulate href "https://nest-simulator.org"



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
