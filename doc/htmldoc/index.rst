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

.. grid::

   .. grid-item-card::
      :columns: 8

      .. raw:: html
         :file: static/img/network-brain_1.1comp.svg

   .. grid-item::
      :columns: 4
      :child-align: center

      .. raw:: html

         <div class="popuptext" id="neuron">
         <a href="neurons/index.html"> <img src="_static/img/neuron_text.svg" alt="Neuron graphic">
         </a>
         </div>
         <div class="popuptext" id="synapse">
         <a href="synapses/index.html"> <img src="_static/img/synapse_text.svg" alt="Synapse graphic">
         </a>
         </div>
         <div class="popuptext" id="device">
         <a href="devices/index.html"> <img src="_static/img/device_text.svg" alt="Device graphic">
         </a>
         </div>


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
