
.. _sphx_glr_auto_examples:

NEST Example Networks
=====================


.. toctree::
   :maxdepth: 1

   ../auto_examples/one_neuron
   ../auto_examples/one_neuron_with_noise
   ../auto_examples/twoneurons
   ../auto_examples/balancedneuron
   ../auto_examples/testiaf
   ../auto_examples/repeated_stimulation
   ../auto_examples/multimeter_file
   ../auto_examples/sensitivity_to_perturbation
   ../auto_examples/plot_weight_matrices
   ../auto_examples/if_curve
   ../auto_examples/pulsepacket
   ../auto_examples/correlospinmatrix_detector_two_neuron
   ../auto_examples/cross_check_mip_corrdet
   ../auto_examples/CampbellSiegert
   ../auto_examples/BrodyHopfield
   ../auto_examples/hh_psc_alpha
   ../auto_examples/hh_phaseplane
   ../auto_examples/structural_plasticity
   ../auto_examples/gap_junctions_two_neurons
   ../auto_examples/gap_junctions_inhibitory_network
   ../auto_examples/gif_population
   ../auto_examples/gif_pop_psc_exp
   ../auto_examples/brette_gerstner_fig_2c
   ../auto_examples/brette_gerstner_fig_3d
   ../auto_examples/mc_neuron
   ../auto_examples/tsodyks_depressing
   ../auto_examples/tsodyks_facilitating
   ../auto_examples/evaluate_tsodyks2_synapse
   ../auto_examples/evaluate_quantal_stp_synapse
   ../auto_examples/intrinsic_currents_spiking
   ../auto_examples/intrinsic_currents_subthreshold
   ../auto_examples/lin_rate_ipn_network
   ../auto_examples/rate_neuron_dm
   ../auto_examples/precise_spiking
   ../auto_examples/sinusoidal_poisson_generator
   ../auto_examples/sinusoidal_gamma_generator
   ../auto_examples/clopath_synapse_spike_pairing
   ../auto_examples/clopath_synapse_small_network
   ../auto_examples/brunel_alpha_numpy
   ../auto_examples/brunel_alpha_nest
   ../auto_examples/brunel_delta_nest
   ../auto_examples/brunel_siegert_nest
   ../auto_examples/brunel_exp_multisynapse_nest
   ../auto_examples/brunel_alpha_evolution_strategies
   ../auto_examples/csa_example
   ../auto_examples/csa_topology_example
   ../auto_examples/hpc_benchmark




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

   ../auto_examples/Potjans_2014/example
   ../auto_examples/Potjans_2014/network
   ../auto_examples/Potjans_2014/helpers
   ../auto_examples/Potjans_2014/network_params
   ../auto_examples/Potjans_2014/stimulus_params
   ../auto_examples/Potjans_2014/sim_params

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

    ../auto_examples/music_cont_out_proxy_example/nest_script
    ../auto_examples/music_cont_out_proxy_example/receiver_script


