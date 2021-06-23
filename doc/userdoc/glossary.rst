:orphan:

.. _sec_glossary:

Glossary
========

Common abbreviations in NEST
----------------------------
.. glossary::

 iaf
   integrate and fire

 gif
   generalized integrate and fire

 cond
   conductance-based

 psc
   post-synaptic current (current-based)

 hh
   hodgkin huxley

 rng
   random number generator

 wfr
   waveform relaxation method

 aeif
   adaptive exponential integrate and fire

 ht
   hill and tononi

 pp
   point process

 in
   inhibitory

 ex
   excitatory

 MAM
   multi-area model

 mpi
   message passing interface

 stdp
   spike-timing dependent plasticity synapse

 st
   short term plasticity

 vp
   virtual process

Physical units and variable names used for NEST parameters
----------------------------------------------------------

.. note::

   all parameters listed here are defined as `type double` in NEST.

.. glossary::

 **time**
   milliseconds `ms`

 tau_m
   membrane time constant in ms

 t_ref
   duration of refractory period in ms

 t_spike
   point in time of last spike in

 **capacitance**
   picofarads `pF`

 C_m
   Capacitance of the membrane in pF

 **current**
   picoamperes `pA`

 I_e
   constant input current in pA

 **conductance**
   nanosiemens `nS`

 g_L
   Leak conductance in nS

 g_K
   Potassium peak conductance in nS

 g_Na
   Sodium peak conductance in nS

 **spike rates**
   spikes/s

 **modulation frequencies**
   herz `Hz`

 frequency
   frequncy in Hz

 **voltage**
   millivolts `mV`

 V_m
   Membrane potential in mV

 E_L
   Resting membrane potential in mV

 V_th
   Spike threshold in mV

 V_reset double
   Reset potential of the membrane in mV

 V_min
   Absolute lower value for the membrane potential in mV

 E_ex
   Excitatory reversal potential in mV

 E_in
   Inhibitory reversal potential in mV

 E_Na
   Sodium reversal potential in mV

 E_K
   Potassium reversal potential in mV

 subthreshold dynamics
   Non-spiking backgound activity of the synapses

 refractory period
   A time period in which neurons cannot fire. This is due to depolarization.

 shotnoise
   Fluctuations in ion channels as a result ionic migration through an open channel.

 autapse
   A neuron innervating itelf forming an autapse instead of a synapse.

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
   A time period in which neurons cannot fire. This is due to depolarization.

 Point process
   A temporal point process is a mathematical model for a time series of discrete events. 

 non-renewal process
   Point process with adapting threshold eta(t).

 rheobase
   The minimal current that is required to generate a spike.

 reversal potential
   The membrane potential at which a neuron causes no net current flow.

 time constant
   The time it takes for a signal to rise or decay. (ms)

   See membrane time constant (tau_m) and synaptic time constant (tau_syn) in in model documentations.

 Gaussian white noise
   A random process with zero mean.

 sfa
   spike-frequency adaptation

 point neuron
   A simple neuron model representing its soma with the membrane potential dynamics modeled as a resistance–capacitance circuit.

 propagator
   Matrix used in numerically integrating dynamical system. See exact integration page.

 synaptic response kernel
   Shape of post-synaptic response. Commonly an alpha, delta-pulse, or exponential function.

 eligibility trace
   A property of a synapse which allows it to be modified for a period of time when some constraints are satisfied.

 reversal potential
   The membrane potential at which a neuron causes no net current flow.

 alpha function
   Instance of synaptic response.

 facilitation
   Mechanism of making a synapse stronger by increasing the weight. Opposite to depression.
 
 depression
   Mechanism of making a synapse weaker by decreasing the weight. Opposite to facilitation.

 stdp_synapse
   Synapse with spike-timing dependent plasticity.

 static_synapse
   Synapse with a fixed weight.
 
 refractoriness
   The time before a new action potential can take place.

 renewal process
   refer to spike-time statistical analysis
 
 spike train
   A sequence of action potentials

 spike-frequency adaptation
   After stimulation, neurons show a reduction in the firing frequency of their spike response following an initial increase. 

 GIF
   Generalized integrate-and-fire model

 coefficient of variation
   Standard deviation divided by the mean

 distal dendrite
   The part of the dentrite which is further away from the soma.

 proximal dendrite
   The part of the dentrite which is closer to the soma.

 soma
   Cell body of the neuron

 psp
   Post-synaptic potential

 PSC
   Post-synatpic current

 absolute refractory
   An interval after the neurons fires a spike in which it is prevent to fire a spike again.

 indegree
   Amount of connections to post-synaptic cells.

 outdegree
   Amount of connections from pre-synaptic cells.
 
 synaptic efficacy
   The extent to which a pre-synaptic neuron affects a post-synaptic neuron.

 multimeter
   A device that allows to record analog quantities (e.g. membrane voltage) of a neuron over time.

 events
   Spikes are encoded as events in nest.
