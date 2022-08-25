:orphan:

.. _glossary:

Glossary
========

.. glossary::
 :sorted:

 iaf
   Integrate and fire.

 gif
   Generalized integrate and fire.

 cond
   Conductance-based.

 psc
   Post-synaptic current (current-based).

 hh
   Hodgkin huxley.

 rng
   Random number generator.

 wfr
   Waveform relaxation method.

 aeif
   Adaptive exponential integrate and fire.

 ht
   Hill and tononi.

 pp
   Point process.

 in
   Inhibitory.

 ex
   Excitatory.

 MAM
   Multi-area model.

 mpi
   Message passing interface.

 stdp
   Spike-timing dependent plasticity synapse.

 st
   Short term plasticity.

 vp
   Virtual process.

 time
   Milliseconds (ms).

 tau_m
   Membrane time constant in milliseconds (ms).

 t_ref
   Duration of refractory period in milliseconds (ms).

 t_spike
   Point in time of last spike in milliseconds (ms).

 capacitance
   Picofarads (pF).

 C_m
   Capacitance of the membrane in picofarads (pF).

 current
   Picoamperes (pA).

 I_e
   Constant input current in picoamperes (pA).

 conductance
   Nanosiemens (nS).

 g_L
   Leak conductance in Nanosiemens (nS).

 g_K
   Potassium peak conductance in Nanosiemens (nS).

 g_Na
   Sodium peak conductance in Nanosiemens (nS).

 spike rates
   Spikes/second.

 modulation frequencies
   Herz (Hz).

 frequency
   Frequncy in Hertz (Hz).

 voltage
   Millivolts (mV).

 V_m
   Membrane potential in Millivolts (mV).

 E_L
   Resting membrane potential in Millivolts (mV).

 V_th
   Spike threshold in Millivolts (mV).

 V_reset double
   Reset potential of the membrane in Millivolts (mV).

 V_min
   Absolute lower value for the membrane potential in Millivolts (mV).

 E_ex
   Excitatory reversal potential in Millivolts (mV).

 E_in
   Inhibitory reversal potential in Millivolts (mV).

 E_Na
   Sodium reversal potential in Millivolts (mV).

 E_K
   Potassium reversal potential in Millivolts (mV).

 subthreshold dynamics
   Non-spiking backgound activity of the synapses.

 refractory period
   A time period in which neurons cannot fire. This is due to depolarization.

 shotnoise
   Fluctuations in ion channels as a result of ionic migration through an open channel.

 autapse
   A neuron connected to itself.

 multapse
   A neuron that has (multiple) synapses with another neuron.

 spike-timing dependent plasticity
   STDP, a form of plasticity which adjusts the connection strength between neurons based on the relative timing of a neurons output and input spikes.

 spike train
   A sequence of actions potentials. Usually seen as events in integrate-and-fire models.

 depressing window
   A function that determines how synaptic modification depends on spike-timing (STDP).

 dendritic arbor
   Dendritic trees formed to create new synapses.

 axon
   The output structure of a neuron.

 Clopath
   Refering to the Clopath plasticity rule.

 plasticity
   The ability of a network to grow or reorganize.

 Hodgkin-Huxley
   A mathematical model that describes how action potentials in neurons can be generated and how they propagate.

 refractory time
   A time period in which neurons cannot fire due to depolarization.

 Point process
   A temporal point process is a mathematical model for a time series of discrete events.

 non-renewal process
   Point process with adapting threshold eta(t).

 rheobase
   The minimal current that is required to generate a spike.

 reversal potential
   The membrane potential at which a neuron causes no net current flow.

 time constant
   The time it takes for a signal to rise or decay in milliseconds (ms).

   See membrane time constant (tau_m) and synaptic time constant (tau_syn) in the model documentation.

 Gaussian white noise
   A random process with a mean of zero.

 sfa
   Spike-frequency adaptation.

 point neuron
   A simple neuron model where its soma along with the membrane potential dynamics are modeled as a resistance–capacitance circuit.

 propagator
   Matrix used in a numerically integrated dynamical system.

   See :ref:`exact integration <exact_integration>` page for further information.

 synaptic response kernel
   Shape of post-synaptic response, commonly an alpha, delta-pulse, or exponential function.

 eligibility trace
   A property of a synapse, which allows it to be modified for a period of time when some constraints are satisfied.

 alpha function
   Instance of a synaptic response.

 facilitation
   Mechanism of making a synapse stronger by increasing the weight.

   Opposite to depression.

 depression
   Mechanism of making a synapse weaker by decreasing the weight.

   Opposite to facilitation.

 stdp_synapse
   Synapse with spike-timing dependent plasticity.

 static_synapse
   Synapse with a fixed weight.

 refractoriness
   The time before a new action potential can take place.

 renewal process
   Spike-time statistical analysis.

  spike-frequency adaptation
   After stimulation, neurons show a reduction in the firing frequency of their spike response following an initial increase.

 GIF
   Generalized integrate-and-fire model.

 coefficient of variation
   Standard deviation divided by the mean.

 distal dendrite
   The part of the dentrite that is furthest away from the soma.

 proximal dendrite
   The part of the dentrite which is closest to the soma.

 soma
   Cell body of the neuron.

 psp
   Post-synaptic potential.

 PSC
   Post-synatpic current.

 absolute refractory
   An interval after a neuron fires a spike to prevent it from firing a spike again.

 indegree
   Amount of connections to post-synaptic cells.

 outdegree
   Amount of connections from pre-synaptic cells.

 synaptic efficacy
   The extent to which a pre-synaptic neuron affects a post-synaptic neuron.

 multimeter
   A device to record analog quantities (e.g., membrane voltage) of a neuron over time.

 events
   Spikes are encoded as events in NEST.
