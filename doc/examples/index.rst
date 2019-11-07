
.. _sphx_glr_auto_examples:

NEST Example Networks
=====================


.. toctree::
   :maxdepth: 1

   one_neuron
   one_neuron_with_noise
   twoneurons
   balancedneuron
   testiaf
   repeated_stimulation
   multimeter_file
   sensitivity_to_perturbation
   plot_weight_matrices
   if_curve
   pulsepacket
   correlospinmatrix_detector_two_neuron
   cross_check_mip_corrdet
   CampbellSiegert
   BrodyHopfield
   hh_psc_alpha
   hh_phaseplane
   structural_plasticity
   gap_junctions_two_neurons
   gap_junctions_inhibitory_network
   gif_population
   gif_pop_psc_exp
   brette_gerstner_fig_2c
   brette_gerstner_fig_3d
   mc_neuron
   tsodyks_depressing
   tsodyks_facilitating
   evaluate_tsodyks2_synapse
   evaluate_quantal_stp_synapse
   intrinsic_currents_spiking
   intrinsic_currents_subthreshold
   lin_rate_ipn_network
   rate_neuron_dm
   precise_spiking
   sinusoidal_poisson_generator
   sinusoidal_gamma_generator
   clopath_synapse_spike_pairing
   clopath_synapse_small_network
   brunel_alpha_numpy
   brunel_alpha_nest
   brunel_delta_nest
   brunel_siegert_nest
   brunel_exp_multisynapse_nest
   brunel_alpha_evolution_strategies
   csa_example
   csa_topology_example
   hpc_benchmark




.. _sphx_glr_auto_examples_Potjans_2014:

Microcircuit Example
=====================


 Hendrik Rothe, Hannah Bos, Sacha van Albada

Description
-----------

 This is a PyNEST implementation of the microcircuit model by Potjans and

-  This example contains several files:

   -  ``helpers.py``
      Helper functions for the simulation and evaluation of the
      microcircuit.
   -  ``network.py``
      Gathers all parameters and connects the different nodes with each
      other.
   -  ``network_params.py``
      Contains the parameters for the network.
   -  ``sim_params.py``
      Contains the simulation parameters.
   -  ``stimulus_params.py``
      Contains the parameters for the stimuli.
   -  ``example.py``
      Use this script to try out the microcircuit.

**How to use the Microcircuit model example:**

To run the microcircuit on a local machine, we have to first check that the
variables `N_scaling` and `K_scaling` in ``network_params.py`` are set to
``0.1``. `N_scaling` adjusts the number of neurons and `K_scaling` adjusts
the number of connections to be simulated. The full network can be run by
adjusting these values to 1. If this is done, the option to print the time
progress should be set to False in the file ``sim_params.py``. For running, use
``python example.py``. The output will be saved in the directory ``data``.

The code can be parallelized using OpenMP and MPI, if NEST has been built with
these applications `(Parallel computing with NEST) <https://www.nest-simulator.org/parallel_computing/>`__.
The number of threads (per MPI process) can be chosen by adjusting
`local_num_threads` in ``sim_params.py``. The number of MPI processes can be
set by choosing a reasonable value for `num_mpi_prc` and then running the
script with the following command.

.. code-block:: python

   mpirun -n num_mpi_prc python example.py

The default version of the simulation uses Poissonian input, which is defined
in the file ``network_params.py`` to excite neuronal populations of the
microcircuit. If no Poissonian input is provided, DC input is calculated, which
should approximately compensate the Poissonian input. It is also possible to
add thalamic stimulation to the microcircuit or drive it with constant DC
input. This can be defined in the file ``stimulus_params.py``.

.. toctree::

   Potjans_2014/example
   Potjans_2014/network
   Potjans_2014/helpers
   Potjans_2014/network_params
   Potjans_2014/stimulus_params
   Potjans_2014/sim_params

.. _sphx_glr_auto_examples_music_cont_out_proxy_example:

MUSIC example
==============================

Requirements
------------

-  MUSIC 1.1.15 or higher
-  NEST 2.14.0 or higher compiled with MPI and MUSIC
-  NumPy

Instructions
------------

This example runs 2 NEST instances and one receiver instance. Neurons on
the NEST instances are observed by the music_cont_out_proxy and their
values are forwarded through MUSIC to the receiver.

.. code-block:: bash

  mpiexec -np 3 music test.music

.. toctree::

    music_cont_out_proxy_example/nest_script
    music_cont_out_proxy_example/receiver_script


