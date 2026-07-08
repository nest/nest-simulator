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

          To get started you can:

          ``pip install nest-simulator``

          see :ref:`here <install_nest>` for all installation options!


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

          .. raw:: html

             <div id="home-carousel" class="carousel slide carousel-fade carousel-dark" data-bs-ride="carousel">
               <div class="carousel-indicators">
                 <button type="button" data-bs-target="#home-carousel" data-bs-slide-to="0" class="active" aria-current="true" aria-label="Slide 1"></button>
                 <button type="button" data-bs-target="#home-carousel" data-bs-slide-to="1" aria-label="Slide 2"></button>
                 <button type="button" data-bs-target="#home-carousel" data-bs-slide-to="2" aria-label="Slide 3"></button>
                 <button type="button" data-bs-target="#home-carousel" data-bs-slide-to="3" aria-label="Slide 4"></button>
                 <button type="button" data-bs-target="#home-carousel" data-bs-slide-to="4" aria-label="Slide 5"></button>
               </div>
               <div class="carousel-inner">
                 <div class="carousel-item active">
                   <a href="networks/index.html">
                     <img src="_static/img/network_model_sketch_mesocircuit.png" class="d-block w-100" alt="Create large network models">
                   </a>
                   <div class="carousel-caption d-none d-md-block">
                     <h5>Create large network models</h5>
                   </div>
                 </div>
                 <div class="carousel-item">
                   <a href="auto_examples/astrocytes/index.html">
                     <img src="_static/img/astrocyte_interaction.png" class="d-block w-100" alt="Inspect neuron and astrocyte interactions">
                   </a>
                   <div class="carousel-caption d-none d-md-block">
                     <h5>Inspect neuron and astrocyte interactions</h5>
                   </div>
                 </div>
                 <div class="carousel-item">
                   <a href="auto_examples/hpc_benchmark.html">
                     <img src="_static/img/hpc_benchmark_connectivity.svg" class="d-block w-100" alt="Test performance and benchmarks">
                   </a>
                   <div class="carousel-caption d-none d-md-block">
                     <h5>Test performance and benchmarks</h5>
                   </div>
                 </div>
                 <div class="carousel-item">
                   <a href="auto_examples/pong/run_simulations.html">
                     <img src="_static/img/pong_sim.gif" class="d-block w-100" alt="Simulate a game of PONG with NEST">
                   </a>
                   <div class="carousel-caption d-none d-md-block">
                     <h5>Simulate a game of PONG with NEST</h5>
                   </div>
                 </div>
                 <div class="carousel-item">
                   <a href="auto_examples/eprop_plasticity/index.html">
                     <img src="_static/img/eprop_supervised_regression_sine-waves.png" class="d-block w-100" alt="Explore eligibility propagation plasticity">
                   </a>
                   <div class="carousel-caption d-none d-md-block">
                     <h5>Explore eligibility propagation plasticity</h5>
                   </div>
                 </div>
               </div>
             </div>

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
