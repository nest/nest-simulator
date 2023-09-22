.. _hh_details:

How NEST generates spikes in Hodgkin-Huxley style models
========================================================




The way in which Hogkin-Huxley style neuron models in NEST generate outgoing spikes is not trivial. In particular when implementing new HH-style models, for example via NESTML, this can lead to confusing observations, where spikes recorded by a ``spike_recorder`` seem to disagree with membrane potential traces recorded with a ``voltmeter``. This document addresses the underlying challenges.

The original Hodgkin-Huxley model is not a model of a neuron, but a model for spike propagation down the squid axon.
In this model, there is no such thing as a singular spike in the sense we commonly use it when discussing
integrate-and-fire point neuron models.

In the HH model, under certain conditions, we have a rapidly rising sodium current depolarizing the membrane up to around +30 mV,
followed by a rapidly activating potassium current repolarizing the membrane again towards -70 mV.
The resulting membrane potential excursion looks like a spike, but the dynamics are entirely continuous and described by the
Hogkin-Huxley ordinary differential equations (ODEs).
These ODEs also incorporate the refractory mechanism.
If one creates a chain of HH-compartments, activation will thus travel one way, again as an entirely continuous process.
Only when the activation reaches an axon terminal will something new happen: exocytosis of transmitter substances into the synaptic cleft.

In contrast, typical point-neuron models have, on one hand, the sub-threshold membrane potential dynamics, described by continuous ODEs,
and some threshold condition, on the other hand. When the threshold condition is met,
we say the neuron emits a spike (in the sense of a singular event causing exocytosis at axon terminals),
and typically some reset mechanism applies, followed by some refractory mechanism.

When we now force the HH model into this point-neuron framework (subthreshold dynamics + threshold mechanism + reset + refractoriness), the modeler needs to make choices.
For the subthreshold dynamics, one will usually use the HH equations, since this is a HH model, after all.

For the threshold mechanism, various choices would be possible, but somehow one needs to detect the fast rise followed by fast fall of the membrane potential.
In the built-in ``hh_psc_alpha model``, this is done by

.. code-block:: cpp

    else if ( S_.y_[ State_::V_M ] >= 0 and U_old > S_.y_[ State_::V_M ] ) // ( threshold and maximum )

The first criterion makes sure that we are well into the rising flank of the Na-driven depolarization; the second criterion ensures that
we have just passed the maximum of that excursion.
For the reset, ``hh_psc_alpha`` does nothing, which makes sense, since the subthreshold HH dynamics have the reset built in—the `K` current.
For refractoriness, ``hh_psc_alpha`` has a fixed refractory time (2 ms by default). In this model, the only effect of refractoriness is that it prohibits spiking.
The subthreshold dynamics evolve freely during refractoriness. This again makes sense, since the HH dynamics have refractoriness built in.
This is different in iaf-models, where the membrane potential is typically clamped to a reset potential during refractoriness.

Now the latter point raises the question of why ``hh_psc_alpha`` has an explicit refractory mechanism even though the HH-dynamics include refractory effects already.
The reason is technical. Given that the Na-current will push the membrane potential to around +30 mV,
it will take several time steps before the K-current will pull the potential to below 0 mV again. Then, if

.. code-block:: cpp

    S_.y_[ State_::V_M ] >= 0 and U_old > S_.y_[ State_::V_M ]

is the criterion for spike emission, a spike would be emitted for every time step during the downward flank of the membrane potential excursion until :math:`V_m < 0` again.
This might take the effect of the action potential shape showing one spike, but spike recorder showing several spikes with no synchronicity between them.
The refractory period in NEST's ``hh_psc_alpha`` simply suppresses these spikes during the downward flank.
A better criterion could be to not just compare V_m at two points in time but at three time steps to look for an actual maximum.
