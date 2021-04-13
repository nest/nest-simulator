:orphan:

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
   postsynaptic current (current-based)

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

   all parameters listed here are defined as `type double` in NEST

.. glossary::

 **time**
    milliseconds `ms`

 tau_m
    Membrane time constant in ms

 t_ref
    Duration of refractory period in ms

 t_spike
    point in time of last spike in

 **capacitance**
    picofarads `pF`

 C_m
    Capacitance of the membrane in pF

 **current**
    picoamperes `pA`

 I_e
    Constant input current in pA.

 **conductance**
    nanosiemens `nS`

   g_L
    Leak conductance in nS

   g_K
    Potassium peak conductance in nS.

   g_Na
    Sodium peak conductance in nS.

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
   Resting membrane potential in mV.

 V_th
   Spike threshold in mV.

 V_reset double
   Reset potential of the membrane in mV.

 V_min
   Absolute lower value for the membrane potential in mV

 E_ex
   Excitatory reversal potential in mV.

 E_in
    Inhibitory reversal potential in mV.

 E_Na
   Sodium reversal potential in mV.

 E_K
   Potassium reversal potential in mV.





 subthreshold dynamics
   Non-spiking backgound activity of the synapses.

 refractory period
   A time period in which neurons cannot fire. This is due to depolarization.

 shotnoise
   Fluctuations in ion channels as a result ionic migration through an open channel.

 autapse
   A neuron innervating itelf forming an autapse instead of a synapse.

 multapse
   A neuron having (multiple) synapses with another neuron.

 spike-timing dependent plasticity
   STDP, a form of plasticity which adjusts the connection strength between neurons based on the relative timing of a neurons output and input spikes.

 spike train
   A sequence of actions potentials. Usually seen as events in integrate-and-fire models.

 depressing window
   A function that determines how synaptic modification depends on spike-timing. (STDP)

 dendritic arbor
   Dendritic trees formed to create new synapses.

 axon
   The output structure of a neuron.

 Clopath

 plasticity
   The ability of a network to grow or reorganize.

 Hodgkin-Huxley
   A mathematical model that describes how action potentials in neurons can be generated and how they propagate.

 refractory time
   A time period in which neurons cannot fire. This is due to depolarization.

 Point process
   A configuration of points in space that usually have some distrubtion.


 non-renewal process
   Point process with adapting threshold eta(t).

 rheobase
   The minimal current that is required to generate a spike.

 reversal potential
   The membrane potential at which a neuron causes no net current flow.

 time constant

 Gaussian white noise
   A random process with zero mean.

 stc

 sfa

 point neuron
   A simple neuron model representing its soma with the membrane potential dynamics modeled as a resistance–capacitance circuit.

 propagator

 synaptic current kernel

 eligibility trace
   A property of a synapse which allows it to be modified for a period of time when some constraints are satisfied.

 reversal potential
   The membrane potential at which a neuron causes no net current flow.

 activation variable

 alpha function

 facilitation

 stdp
  Spike-timing dependent plasticity

 Two-timescale adaptive threshold
 
 dead time
 
 refractoriness
   The time before a new action potential can take place.

 renewal process
 
 spike train
   A sequence of action potentials

 threshold rate

 spike-frequency adaptation

 relative refractory mechanisms

 GIF

 coefficient of variation
   Standard deviation divided by the mean.

 mirrored perturbations

 distal (dendrite)

 proximal (dendrite)

 soma (dendrite)

 psp
   Post-synaptic potential

 PSC
   Post-synatpic current

 transient
 
 exponential link function

 feedback kernel theta

 multiplicative depression

 power-law potentiation

 nullcline

 Rp

 Vp

 convergent projection

 absolute refractory

 matching potential

 intrinsic current

 in-degree
 
 synaptic efficacy
   The extent to which a presynaptic neuron affects a postsynaptic neuron.

 multimeter
   A device that allows to record the membrane voltage of a neuron over time.

 events
   Spikes are encoded as events in nest.