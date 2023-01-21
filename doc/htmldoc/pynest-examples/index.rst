.. _pynest_examples:

PyNEST examples
===============

Explore all examples NEST has to offer. You can run every example as a Jupyter notebook. Click on the `Try it on EBRAINS` button to launch
Jupyter lab on EBRAINS. For more information see :ref:`our guide on running notebooks <run_jupyter>`.

.. grid:: 1 1 2 3

    .. grid-item-card:: Simple networks
           :img-top: ../static/img/pynest/mc_neuron.png

           * :doc:`one_neuron`
           * :doc:`one_neuron_with_noise`
           * :doc:`twoneurons`
           * :doc:`balancedneuron`
           * :doc:`aeif_cond_beta_multisynapse`
           * :doc:`testiaf`
           * :doc:`vinit_example`
           * :doc:`mc_neuron`

    .. grid-item-card:: 2D Spatially-structured networks
           :img-top: ../static/img/pynest/layer4.png

           * :doc:`spatial/conncomp`
           * :doc:`spatial/conncon_sources`
           * :doc:`spatial/conncon_targets`
           * :doc:`spatial/connex`
           * :doc:`spatial/connex_ew`
           * :doc:`spatial/ctx_2n`
           * :doc:`spatial/gaussex`
           * :doc:`spatial/grid_iaf`
           * :doc:`spatial/grid_iaf_irr`
           * :doc:`spatial/grid_iaf_oc`


    .. grid-item-card:: 3D Spatially-structured networks
           :img-top: ../static/img/pynest/spatial_test3d.png

           * :doc:`spatial/test_3d`
           * :doc:`spatial/test_3d_exp`
           * :doc:`spatial/test_3d_gauss`


.. grid:: 1 1 2 3

    .. grid-item-card:: NEST Sudoku solver
           :img-top: ../static/img/sudoku_solution.gif

           * :doc:`sudoku/sudoku_solver`
           * :doc:`sudoku/plot_progress`

    .. grid-item-card:: NEST Pong game
           :img-top: ../static/img/pong_sim.gif

           * :doc:`pong/run_simulations`
           * :doc:`pong/generate_gif`

.. grid:: 1 1 2 3

    .. grid-item-card:: Random balanced networks (Brunel)
           :img-top: ../static/img/pynest/brunel_alpha.png

           * :doc:`brunel_alpha_nest`
           * :doc:`brunel_delta_nest`
           * :doc:`brunel_siegert_nest`
           * :doc:`brunel_exp_multisynapse_nest`
           * :doc:`brunel_alpha_evolution_strategies`


    .. grid-item-card:: Cortical microcircuit (Potjans)
           :img-top: ../static/img/pynest/raster_plot.png

           * :doc:`cortical_microcircuit_index`

    .. grid-item-card:: GLIF (from Allen institute)
           :img-top: ../static/img/pynest/glif_cond.png

           * :doc:`glif_cond_neuron`
           * :doc:`glif_psc_neuron`


.. grid:: 1 1 2 3

    .. grid-item-card:: Compartmental neurons
           :img-top: ../static/img/pynest/dendritic_synapse_conductances.png

           * :doc:`compartmental_model/receptors_and_current`
           * :doc:`compartmental_model/two_comps`

    .. grid-item-card:: Rate neurons
           :img-top: ../static/img/pynest/rate_neuron.png

           * :doc:`lin_rate_ipn_network`
           * :doc:`rate_neuron_dm`


    .. grid-item-card:: GIF (from Gerstner lab)
           :img-top: ../static/img/pynest/gif_pop.png

           * :doc:`gif_cond_exp_multisynapse`
           * :doc:`gif_population`
           * :doc:`gif_pop_psc_exp`


.. grid:: 1 1 2 3

    .. grid-item-card:: Hodgkin-Huxley
           :img-top: ../static/img/pynest/hh_phase.png

           * :doc:`hh_psc_alpha`
           * :doc:`hh_phaseplane`

    .. grid-item-card:: Brody Hopfield
           :img-top: ../static/img/nest_logo-faded.png

           * :doc:`BrodyHopfield`

    .. grid-item-card:: Brette and Gerstner
           :img-top: ../static/img/pynest/brette_gerstner2c.png


           * :doc:`brette_gerstner_fig_2c`
           * :doc:`brette_gerstner_fig_3d`

.. grid:: 1 1 2 3

    .. grid-item-card:: Precise spiking
           :img-top: ../static/img/pynest/precisespiking.png

           * :doc:`precise_spiking`

    .. grid-item-card:: Campbell Siegert
           :img-top: ../static/img/nest_logo-faded.png

           * :doc:`CampbellSiegert`



.. grid:: 1 1 2 3

    .. grid-item-card:: Gap junctions
           :img-top: ../static/img/pynest/gap_junctioninhib.png

           * :doc:`gap_junctions_two_neurons`
           * :doc:`gap_junctions_inhibitory_network`

    .. grid-item-card:: Structural plasticity
           :img-top: ../static/img/pynest/structuralplasticity.png

           * :doc:`structural_plasticity`

    .. grid-item-card:: Synapse collection
           :img-top: ../static/img/pynest/synapsecollection.png

           * :doc:`synapsecollection`

.. grid:: 1 1 2 3

    .. grid-item-card:: Urbanczik
           :img-top: ../static/img/pynest/urbanczik_syn.png

           * :doc:`urbanczik_synapse_example`

    .. grid-item-card:: Clopath
           :img-top: ../static/img/pynest/clopath.png

           * :doc:`clopath_synapse_spike_pairing`
           * :doc:`clopath_synapse_small_network`


    .. grid-item-card:: Tsodyks
           :img-top: ../static/img/pynest/tsodyks_dep.png

           * :doc:`tsodyks_depressing`
           * :doc:`tsodyks_facilitating`
           * :doc:`evaluate_tsodyks2_synapse`


.. grid:: 1 1 2 3

    .. grid-item-card:: Recording and analyzing networks
           :img-top: ../static/img/pynest/correlospinmatrix.png

           * :doc:`multimeter_file`
           * :doc:`plot_weight_matrices`
           * :doc:`if_curve`
           * :doc:`pulsepacket`
           * :doc:`correlospinmatrix_detector_two_neuron`
           * :doc:`cross_check_mip_corrdet`

    .. grid-item-card:: Stimulating networks
           :img-top: ../static/img/pynest/sensitivitypertubation.png

           * :doc:`sinusoidal_poisson_generator`
           * :doc:`sinusoidal_gamma_generator`
           * :doc:`repeated_stimulation`
           * :doc:`sensitivity_to_perturbation`
           * :doc:`intrinsic_currents_spiking`
           * :doc:`intrinsic_currents_subthreshold`

    .. grid-item-card:: Handling output
           :img-top: ../static/img/pynest/store_restore.png

           * :doc:`recording_demo`
           * :doc:`store_restore_network`
           * :doc:`music_cont_out_proxy_example/nest_script`


.. grid:: 1 1 2 3

    .. grid-item-card:: HPC benchmark
           :img-top: ../static/img/nest_logo-faded.png

           * :doc:`hpc_benchmark`

    .. grid-item-card:: Connection set algebra
           :img-top: ../static/img/nest_logo-faded.png

           * :doc:`csa_example`
           * :doc:`csa_spatial_example`

.. toctree::
   :maxdepth: 1
   :hidden:

   Cortical microcircuit model (based on Potjans and Diesmann, 2014) <cortical_microcircuit_index>

.. toctree::
   :hidden:
   :glob:

   running_notebooks
   *
   spatial/*
   sudoku/*
   pong/*
   Potjans_2014/*
   compartmental_model/*
