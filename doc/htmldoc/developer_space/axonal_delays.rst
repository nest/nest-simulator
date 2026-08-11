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
the kernel to make sure axonal delays are always handled correctly (see Alternative implementations).

Changes to the kernel and neuron models
---------------------------------------

Introducing axonal delays changes the way the min- and max-delays must be calculated, as they are now a combination of
dendritic and axonal delays. The default value for the delay which is now referring to the dendritic delay remains 1,
while the default value for axonal_delay is set to 0. In the default case, purely dendritic delay is assumed.

The ``ArchivingNode`` was made axonal-delay-aware. Each pre-synaptic spike after which a correction could potentially follow,
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
Furthermore, neuron models must now call ``ArchivingNode::pre_run_hook_()`` in their derived pre_run_hook implementation
and call ``reset_correction_entries_stdp_ax_delay_()`` at the end of each timestep in their update implementation.
Currently, only the ``iaf_psc_alpha`` neuron model supports STDP with axonal delays.
All other neurons will act as if the delay of incoming connections was purely dendritic.

Synapse models only support dendritic delay by default. If axonal delays are required, the synapse model must be derived
from ``AxonalDelayConnection`` instead of ``Connection``. The ``AxonalDelayConnection`` is derived from ``Connection`` and adds a single
double-precision member for the axonal delay. The main differences compared to synapses with purely dendritic delays are
different handling of delays inside the send function and the addition of the ``correct_synapse_stdp_ax_delay`` which is
called by the ``ConnectionManager`` when a synapse needs to re-calculate its weight given a new post-synaptic spike and a previous pre-synaptic one.
Currently, only the ``stdp_pl_synapse_hom_ax_delay`` synapse model supports axonal delays.

Changes to the python interface
-------------------------------

In general, the kernel was made axonal-delay-aware and this is reflected in the user interface, as it is now possible
to set the ``names::dendritic_delay`` and ``names::axonal_delay`` for each synapse (given that the synapse model is
derived from ``AxonalDelayConnection``).

Remaining work
---------------


Currently, only one neuron and synapse model are supporting axonal delays. All neuron models that support STDP could
also support axonal delays, without sacrificing performance, changing their behavior, or requiring more memory, but need
to be adapted slightly (i.e., implement handle for ``CorrectionSpikeEvent``, call ``ArchivingNode::pre_run_hook_`` and call
``reset_correction_entries_stdp_ax_delay_``).

Existing STDP synapse models need one version with and one without axonal delays. Alternatively, synapse models could
be templatized to either use only dendritic or dendritic and axonal delays. However, this branching should be resolved
at compile time to not negatively impact performance.
