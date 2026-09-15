.. _axonal_delays_dev:

Axonal Delays
=============

Adding axonal delays to NEST is non-trivial when it comes to their interaction with spike-timing dependent plasticity (STDP).
Axonal delays lower than their dendritic counterpart are non-problematic, however larger axonal delays cause causality
issues due to the way how and when pre- and post-synaptic spikes are processed in NEST by synapses implementing STDP weight dynamics.

If a pre-synaptic spike is processed at a synapse, it will also process all post-synaptic spikes that reached the synapse
between the last and current pre-synaptic spike. Weight changes due facilitation (post-synaptic spike following a
pre-synaptic one) or depression (pre-synaptic spike following a post-synaptic one) are only relevant at the time when pre-synaptic spikes
reach the synapse, as this is the only point in time when the exact weight is of importance. Post-synaptic spikes can
therefore be archived in the post-synaptic neuron until the next pre-synaptic spike is processed by the synapse.
As all pre-synaptic spikes are delivered to their target synapse and neuron right after they have been communicated,
they might be processed before they would actually reach the synapse when taking axonal delays into account.
If the axonal delay is now larger than the dendritic delay, post-synaptic
spikes occurring at time `t` will reach the synapse before pre-synaptic spikes occurring before `t`,
but might not be taken into account by the pre-synaptic spike, if it was already communicated,
and thus delivered, before `t`. Each pre-synaptic spike sent over a connection
with a predominant axonal delay must therefore also process post-synaptic spikes which have not yet occurred,
but could be emitted in the future. Multiple implementations were implemented and
benchmarked before coming to the conclusion that the implementation at hand should be used inside NEST.

The main idea of the axonal delays implementation in NEST is based on the fact, that neurons only emit few spikes per second.
It should thus be rare that a post-synaptic spike occurs right after a pre-synaptic one in the critical region before
the pre-synaptic spike reaches the synapse, but has already been processed. In typical networks, there will most likely
only be few occurrences where causality becomes an issue. In order to still guarantee correct synaptic weights,
incorrect STDP weight changes are rolled back, re-calculated, and the weight of pre-synaptic spike, which already reached
the target neuron's ring buffer, is corrected. Undoing the STDP weight changes and re-calculating them obviously comes
with a cost, however as only few such occurrences are to be expected, this solution is more efficient than restructuring
the kernel to make sure axonal delays are always handled correctly. The alternatives that were tried and rejected --
among them buffering pre-synaptic spikes until they reach the synapse, and restructuring spike communication itself --
are to be described in a forthcoming publication rather than here.

Changes to the kernel and neuron models
---------------------------------------

Introducing axonal delays changes the way the min- and max-delays must be calculated, as they are now a combination of
dendritic and axonal delays. The default value for the delay which is now referring to the dendritic delay remains 1,
while the default value for axonal_delay is set to 0. In the default case, purely dendritic delay is assumed.

The ``AxonalDelayArchivingNode`` holds the machinery for axonal delays, kept separate from the ``ArchivingNode``
and its post-synaptic spike history, which is an independent concern. Each pre-synaptic spike after which a correction could potentially follow,
will be archived in the post-synaptic neuron in a dynamic ring-buffer-like structure. Post-synaptic spikes will then
trigger a correction for all relevant pre-synaptic spikes in this buffer. The way spikes are received at a neuron is
model-dependent, as the implementation of spike accumulation and buffering until being processed might vary between
neuron models. Neurons models will therefore also have to handle correction of previously handled spikes differently.
In the simplest case, all incoming spikes to a neuron are simply accumulated in a single scalar value per time slot.
A correction of a previously handled spike would therefore just subtract the previous, wrong weight and add the new,
corrected weight. Therefore, simply sending another spike with the difference of the old and new weight would be
sufficient in this case. However, some neurons might have different buffers for spikes being sent over inhibitory and
excitatory connections, which could be distinguished by the sign of the weight. If a new spike is now sent to correct
an old one, the sign might be negative even though both the old and new weight were originally positive, the new weight
is just smaller. In such a case, the spike would be accumulated in the wrong buffer.

Instead of sending a regular ``SpikeEvent`` to signal a correction, a ``CorrectionSpikeEvent`` is sent. Overloading the handle
function now allows handling the correction in the correct way, depending on the model implementation.
Furthermore, neuron models must now call ``AxonalDelayArchivingNode::pre_run_hook_()`` in their derived pre_run_hook
implementation, call ``reset_correction_entries_stdp_ax_delay_()`` at the end of each timestep in their update
implementation, and override ``supports_axonal_delay_corrections()`` to return true.
Currently, the ``iaf_psc_alpha``, ``iaf_psc_exp`` and ``iaf_psc_delta`` neuron models support STDP with axonal
delays. Connecting a
synapse with a predominantly axonal delay to any other neuron model raises ``IllegalConnection``, rather than
silently computing wrong weights.

Synapse models only support dendritic delay by default. ``Connection`` is templated on the container that holds its
delay, and a synapse opts in purely by that template argument: ``TotalDelay`` stores one delay and throws
``BadProperty`` on any axonal or dendritic accessor, while ``AxonalDendriticDelay`` stores both. The two are defined in
``nestkernel/delay_types.h``.

**This costs no memory per synapse.** ``AxonalDendriticDelay`` does not add a field; it splits the existing 32-bit
delay word into two bitfields, ``NUM_BITS_DENDRITIC_DELAY`` (14) bits of dendritic delay and the remaining 18 bits of
axonal delay. Static asserts in ``nestkernel/connection.h`` pin both instantiations of ``Connection`` to the same
size -- 24 bytes with ``TargetIdentifierPtrRport``, 8 bytes with ``TargetIdentifierIndex`` -- so a synapse that does
not use axonal delays pays nothing at all, and one that does pays nothing either. If a change ever breaks those
asserts, it has broken the property the whole design rests on.

The consequence for the delay range is that a dendritic delay is limited to 2^14 - 1 steps. The remaining differences
compared to synapses with purely dendritic delays are the handling of delays inside the ``send`` function and the
addition of ``correct_synapse_stdp_ax_delay``, which is called by the ``ConnectionManager`` when a synapse needs to
re-calculate its weight given a new post-synaptic spike and a previous pre-synaptic one.
Currently, only the ``stdp_pl_synapse_hom_ax_delay`` synapse model supports axonal delays, together with the
``_hpc``, ``_lbl`` and ``_hpc_lbl`` variants generated from it.

Changes to the python interface
-------------------------------

In general, the kernel was made axonal-delay-aware and this is reflected in the user interface, as it is now possible
to set ``names::dendritic_delay`` and ``names::axonal_delay`` for each synapse whose delay container is
``AxonalDendriticDelay``. Those synapses take the two parameters *instead of* ``delay``, not in addition to it: passing
``delay`` to such a synapse raises, and so does passing ``axonal_delay`` or ``dendritic_delay`` to any other synapse.
The rule holds in ``Connect``, ``SetStatus`` and ``SetDefaults`` alike;
``testsuite/pytests/synapses/test_axonal_delay_user_interface.py`` is the specification.

Remaining work
---------------


Currently, three neuron models and one synapse model support axonal delays. All neuron models that support STDP could
also support axonal delays, without sacrificing performance, changing their behavior, or requiring more memory, but need
to be adapted slightly (i.e., implement handle for ``CorrectionSpikeEvent``, call
``AxonalDelayArchivingNode::pre_run_hook_``, call ``reset_correction_entries_stdp_ax_delay_``, and override
``supports_axonal_delay_corrections``).

The same applies to further STDP synapse models. Each one needs to select a delay container through its ``Connection``
template argument, which resolves the branch at compile time and so costs nothing at run time; a model that keeps
``TotalDelay`` is unaffected by the feature entirely.
