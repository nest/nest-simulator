Examples
========

Here you'll find example networks demonstrating different implementations of
of models in NEST.

.. toctree::
   :hidden:
   :glob:

   *


Basic Networks
-------------------

These networks show basic network setups - a great place to start if new
to NEST

-  :doc:`One Neuron Example <one_neuron>`
-  :doc:`One Neuron with Noise <one_neuron_with_noise>`
-  :doc:`Two Neuron Example <twoneurons>`
-  :doc:`Recording with Multimeter and Writing Output to
   File <multimeter_file>`

Examples using Rate Models
----------------------------

-  :doc:`Network of linear rate neurons <lin_rate_ipn_network>` - simulate populations of lin_rate_ipn neurons

Networks with Multicompartment Models
----------------------------------------

-  :doc:`Multi-compartment Neuron Example <mc_neuron>` - using the
   three-compartment :doc:`iaf_cond_alpha_mc </quickref>` neuron

Topology Examples
------------------
to be added


-  :doc:`Balanced Neuron Example <balancedneuron>` - find a firing rate
   for the inhibitory population that will make the neuron fire at the
   same rate as the excitatory population
-  :doc:`Two Population Network using Structural
   Plasticity <structural_plasticity>` - a simple network of two
   populations where structural plasticity is used
-  :doc:`Binary Neuron Example using the Correlospinmatrix
   Detector <correlospinmatrix_detector_two_neuron>`
-  :doc:`Example of Neuron receiving Input from Multiple
   Receptors <intrinsic_currents_spiking>`
-  :doc:`Example of Neuron with Multiple Intrisic
   Currents <intrinsic_currents_subthreshold>`

-  :doc:`Sensitivity to Perturbation <sensitivity_to_perturbation>`
-  :doc:`Example how to Measure I-F Curve <if_curve>`
-  :doc:`Example of a Pulse Packet <pulsepacket>` - compares the
   average and individual membrane potential excursions in response to a
   single pulse packet with an analytically acquired voltage trace
-  :doc:`Extract connection strength for synapses and input into weight matrices <plot_weight_matrices>`
-  :doc:`Compare two variants of tsodyks-markram synapse model <plot_quantal_stp_synapse>`

Examples using different Generators
----------------------------------------

-  :doc:`Example using the Sinusoidal Poisson Generator <sinusoidal_poisson_generator>`
-  :doc:`Example using the Sinusoidal Gamma Generator <sinusoidal_gamma_generator>`
-  :doc:`Repeated Stimulation using Poisson Generator <repeated_stimulation>` -
   generate a spike train that is recorded directly by a spike detector

Examples using the Integrate and Fire Model
------------------------------------------------

-  :doc:`Integrate and Fire Neuron Example <testiaf>`
-  :doc:`Adapting exponential Integrate and Fire Model
   I <brette_gerstner_fig_2c>`
-  :doc:`Adapting exponential Integrate and Fire Model
   II <brette_gerstner_fig_3d>`
-  :doc:`Spike Synchronization of Integrate and Fire
   Neurons <BrodyHopfield>`
-  :doc:`Calculating Integrate and Fire Neuron with Poisson
   Generators <CampbellSiegert>`
-  :doc:`Plot Initial Membrane Voltage of Integrate and Fire
   Neuron <vinit_example>`
-  :doc:`Compare Precise and Grid-based Integrate and Fire Neuron
   Models <precise_spiking>`
-  :doc:`Population of Generalized Integrate and Fire Neurons with Oscillatory Behavior <gif_population>`

Examples of Connection Setups
------------------------------

    See our detailed guide on :doc:`Connection Management </guides/connection_management>`

-  :doc:`Using CSA for Connection Setup <csa_example>` - set up simple
   netowrk using the Connection Set Algebra
-  :doc:`Using CSA with Topology Layers <csa_topology_example>` - specify
   connections between topology layers using the Connection Set Algebra

Examples with Gap Junctions
----------------------------

   See our detailed guide on :doc:`Simulations with Gap Junctions </guides/simulations_with_gap_junctions>`

-  :doc:`Example of Inhibitory Network with Gap
   Junctions <gap_junctions_inhibitory_network>`
-  :doc:`Two Neuron Example with Gap Junctions <gap_junctions_two_neurons>`

Examples of Random Balanced Networks
-------------------------------------

-  :doc:`Random Balanced Network Example with Alpha
   Synapses <brunel_alpha_nest>`
-  :doc:`Random Balanced Network Example using NumPy <brunel_alpha_numpy>`
-  :doc:`Random Balanced Network Example with Delta
   Synapses <brunel_delta_nest>`
-  :doc:`Random Balanced Network Example with the Multisynapse Neuron
   Model <brunel_exp_multisynapse_nest>`
-  :doc:`Mean-field Theory for Random Balanced Network <brunel_siegert_nest>`

Examples  using Tsodyks Synapse Models
----------------------------------------

-  :doc:`Two Neuron Example Facilitating a Tsodyks
   Synapse <tsodyks_facilitating>`
-  :doc:`Two Neuron Example Depressing a Tsodyks
   Synapse <tsodyks_depressing>`
-  :doc:`Example using Tsodyks2 Synapse <evaluate_tsodyks2_synapse>`
-  :doc:`Compare Tsodyks-Markram Synapse Models <evaluate_quantal_stp_synapse>`
