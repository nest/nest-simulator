NEST Simulator documentaiton
============================


Welcome
-------


.. grid::

  .. grid-item::

    NEST is used in computational neuroscience to model and study behavior of large networks of neurons.

    The models describe single :ref:`neuron` and :ref:`synapse` behavior and their connections.
    Different mechanisms of plasticity can be used to investigate artificial learning
    and help to shed light on the fundamental principles of how the brain works.

    NEST offers convenient and efficient commands to define and connect large networks,
    ranging from algorithmically determined connections to data-driven connectivity.
    Create connections between neurons using numerous synapse models from STDP to gap junctions.


  .. grid-item-card::

      .. carousel::
          :show_controls:
          :data-bs-ride: carousel

            .. figure:: static/img/pong_sim.gif

              PLAY PONG with NEST


            .. figure:: static/img/sudoku_solution.gif

              OR SUDOKU

            .. figure:: static/img/pynest/spatial_test3d.png
              :target: architecture.html

              Create 3D spatially structured networks


            .. figure:: static/img/pynest/structuralplasticity.png

              Showcase cool examples

              Provide users with a glimpse of what nest can do


.. grid:: 1 2 3 3

   .. grid-item-card:: Install NEST
     :class-item: sd-text-center sd-text-white sd-bg-primary


     .. code-block:: python

         pip install nest-simulator

     See more installation options here.

   .. grid-item-card:: Learn NEST
     :class-item: sd-text-center sd-text-white sd-bg-success


     Our PyNEST tutorial will show you how to create your
     first script with NEST simulator. :ref:`tutorial-link <tutorial>`

     Learn how to use  neurons, synapses and devices

   .. grid-item-card:: Explore our models
     :class-item: sd-text-center sd-text-white sd-bg-info

     NEST has extensive model catalog from . . .
     :ref:`Check out our model catalog <modelsmain>`

.. grid:: 1 2 3 3

   .. grid-item-card:: PyNEST API
     :class-item: sd-text-center sd-text-white sd-bg-dark

     Find a function

   .. grid-item-card:: Network models
     :class-item: sd-text-left sd-text-white sd-bg-primary

     * Spatially structured networks ?
     * Microcircuit
     * Mulit area model

   .. grid-item-card::  HPC
     :class-item: sd-text-left sd-text-white sd-bg-success

     * Run NEST on clusters and supercomputers


Example script
--------------

Here is an example of how a script is constructed . . .

this works - TODO add text into image


.. seealso::

   :doc:`tutorials` for  other tests of example script



.. grid:: 1 2 2 2
      :gutter: 1

      .. grid-item::
            :columns: 8

            .. code-block:: python

                import nest

                neurons = nest.Create("iaf_psc_alpha", 10000, {
                    "V_m": nest.random.normal(-5.0),
                    "I_e": 1000.0
                })

      .. grid-item::
            :columns: auto
            :class: sd-d-flex-row sd-align-minor-center

            * :py:func:`.Create`
            * :ref:`link_to_neurondocs`


.. grid:: 1 2 2 2
      :gutter: 1

      .. grid-item::
            :columns: 8

            .. code-block:: python

                 input = nest.Create("noise_generator", params={
                    "amplitude": 500.0
                 })

      .. grid-item::
            :columns: auto

            * :ref:`link_to_stimdevices`


.. grid:: 1 2 2 2

      .. grid-item::
            :columns: 8
            :class: sd-text-wrap

            .. code-block:: python

                nest.Connect(input, neurons, syn_spec={'synapse_model': 'stdp_synapse'})
                spikes = nest.Create("spike_recorder", params={
                    'record_to': 'ascii',
                    'label': 'excitatory_spikes'
                })
                nest.Connect(neurons, spikes)

      .. grid-item::
            :columns: auto

            * :py:func:`.Connect`
            * :ref:`link_to_connectiondocs`
            * :ref:`link_to_recorddevices`

.. grid:: 1 2 2 2

      .. grid-item::
            :columns: 8

            .. code-block:: python

                nest.Simulate(100.0)
                nest.raster_plot.from_device(spikes, hist=True)
                plt.show()

      .. grid-item::
            :columns: auto

            * :py:func:`.Simulate`
            * See all PyNEST functions
Install NEST
------------


.. grid:: 1 1 2 2

   .. grid-item-card::  |user| Install pre-built NEST package
       :class-title: sd-d-flex-row sd-align-minor-center

       I'm a user who wants to :ref:`install NEST on my computer <user_install>`


   .. grid-item-card:: |teacher| Install NEST for a class or workshop
       :class-title: sd-d-flex-row sd-align-minor-center

       I'm a lecturer who wants to :ref:`use NEST to teach <lecturer>`


.. grid:: 1 1 2 2

    .. grid-item-card:: |admin| Install NEST for supercomputers and clusters
       :class-title: sd-d-flex-row sd-align-minor-center

       I'm an admin or user who wants to :ref:`run NEST on HPC <admin_install>`

    .. grid-item-card:: |dev| Install NEST from source
       :class-title: sd-d-flex-row sd-align-minor-center

       I'm a developer who wants to :ref:`do development in NEST <dev_install>`

.. grid:: 1 1 2 2

    .. grid-item-card:: |nestml| Install NEST with NESTML
       :class-title: sd-d-flex-row sd-align-minor-center

       I'm a user who wants to :doc:`create or customize models <nestml:installation>`.


.. toctree::
   :caption: USAGE
   :hidden:
   :glob:

   install <installation/index>
   Tutorials <get-started_index>
   PyNEST API <ref_material/pynest_api/index>
   Examples <examples/index>
   Available models <models/index>
   Neurons in NEST <neurons/index>
   Synapses in NEST <synapses/index>
   Devices in NEST <devices/index>
   Spatially structured networks <networks/spatially_structured_networks> 
   Cite NEST <citing-nest>
   ref_material/glossary
   Community <community>

.. toctree::
   :caption: TECHNICAL DETAILS
   :hidden:

   NEST on HPC <hpc/optimizing_nest>
   nest_sonata/nest_sonata_guide
   connect_nest/nest_server
   model implementation <model_details>
   nest_behavior <nest_behavior/index>
   release_notes <whats_new/index>
   maintenance <developer_space/index>

.. toctree::
   :caption: RELATED PROJECTS
   :hidden:

   nest-desktop <https://nest-desktop.readthedocs.io/en/latest/>
   nestml <https://nestml.readthedocs.io/en/latest/>
   nestgpu <https://nestgpu.readthedocs.io/en/latest/>
   pynn <https://google.com>
   elephant <https://google.com>
   cosim <https://google.com>
   tvb <https://google.com>
   arbor <https://google.com>

.. toctree::
   :maxdepth: 1
   :hidden:

   license

.. |user| image:: static/img/020-user.svg
.. |teacher| image:: static/img/014-teacher.svg
.. |admin| image:: static/img/001-shuttle.svg
.. |dev| image:: static/img/dev_orange.svg
.. |nestml| image:: static/img/nestml-logo.png
      :scale: 15%
