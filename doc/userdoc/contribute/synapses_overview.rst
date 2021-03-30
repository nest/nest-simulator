Synapses in NEST
================

This document gives a brief overview over how synapses are currently
implemented in NEST. Synapses in NEST come in two parts:

1. a Connection object that manages synaptic weight and delay, and

1. code implementing the synaptic currents or conductances resulting
   from a spike arrival; the latter is always coded in the model
   neuron class.

Both parts will be discussed in turn. Some of the discussion below is
simplified, ignoring issues of parallelization; the intent is to give
you the general idea.

Storage of synapses
-------------------

All targets of a given neuron are stored in a target list containing
one ``Connection`` object per target. When a spike is sent, NEST
traverses the target list and sends information about the spike, i.e.,
weight, delay and spike time to each receiving neuron.

Some synapses implement spike-time dependent plasticity, i.e., they
modify the synaptic weight stored in the ``Connection`` object based on
the spike history of the receiving neuron. These synapses access the
spike history through special member functions of the receiving neuron
class, which must be derived from ``ArchivingNode`` (most models are).

The details of connection storage infrastructure are explained in
Kunkel et al. (2014), Spiking network simulation code for petascale
computers. Front. Neuroinform. 8:78. `doi:10.3389/fninf.2014.00078 <http://dx.doi.org/10.3389/fninf.2014.00078>`_.

Event delivery
--------------

When a spike arrives at a neuron by a call to its ``handle(SpikeEvent&)``
method, the ``SpikeEvent`` provides information about the synaptic weight,
the spike time, and the delay, so that the actual arrival time of the
spike can be calculated.

It is now entirely up to the neuron class to handle this
information. In particular, all dynamics of synaptic currents or
conductances must be implemented in the neuron model. See the
:doc:`model development guide <neuron_and_device_models>` for examples
of how this is done.

NEST currently does not have any models with voltage-dependent
synapses, such as NMDA channels. A simple way to introduce such a
voltage dependency in, e.g., the ``iaf_cond_alpha`` model would be to
multiply the synaptic conductance ``g(t)`` with a sigmoidal function
``m(V)`` in the expression computing the right-hand side of the membrane
potential equation.

It is a bit unfortunate that one needs to include the code for the
synaptic currents/conductances anew in each new neuron model, but for
many simpler models this leads to more efficient code than one would
obtain if synaptic current dynamics were represented by separate
objects. We will consider changing this in the future, though.


Using different receptor ports
------------------------------

Introducing NMDA channels obviously raises the question of how to
differentiate input to different types of channels on a neuron, e.g.,
AMPA, GABA_A, GABA_B, and NMDA channels.

Most NEST models are very simplistic in this respect so far: they
differentiate at most between an excitatory and an inhibitory synapse,
and all input with positive weight is handled by the excitatory, all
with negative weight by the inhibitory synapse.

More channels can be handled using the receptor type/receiver port
mechanism in NEST. Please see ``iaf_alpha_cond_mc`` or
``iaf_psc_alpha_multisynapse`` for examples.


Temporal aspects
----------------

Time is a very important issue when processing spikes. Especially when
the effect of a spike on the neuron depends on the history or state of
the neuron, it is crucial that one considers causality carefully.

This is the reason why history information about a neuron is available
only via the ArchivingNode interface, while state information is
currently "exported" only through the UniversalDataLogger interface
(under development, available in few neurons today). The paper
`Morrison et al. (2007) <http://dx.doi.org/10.1162/neco.2007.19.6.1437>`_
paper has more on spike history handling.


Spike handling
--------------

NEST delivers spikes in batches. Simulation proceeds for min_delay
(minimum delay) time. During this time, any generated spikes are
stored in a central spike queue. After each min_delay interval, spikes
are delivered from the central queue via the ConnectionManager and the
Connection objects to the individual neurons.

All delays are handled inside the neuron, as described above. This
means that when a spike "passes through" the Connection object, the
actual biological arrival time (time when spike occured at sender plus
delay) of the spike may be up to max_delay (maximum delay) time units
in the future. This means, in particular, that Connection objects
cannot perform any computations that depend on the state of the neuron
at the time of "biological" spike arrival; they can only use historic
information.

Another important point is that spikes do NOT pass the Connection
object in correct order of biological arrival time---they are
unordered in time.
