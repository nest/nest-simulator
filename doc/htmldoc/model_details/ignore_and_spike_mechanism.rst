.. _ignore_and_spike_mechanism:

Ignore-and-spike mechanism for controlled benchmarking
======================================================

When comparing the efficiency of two models, runtime is often used as a benchmark. Since runtime depends on the number of spikes produced by the underlying dynamics, it can be useful to enforce the same number of spikes in both trials. The ignore-and-spike mechanism supports this by ignoring spikes from the internal dynamics and generating spikes at defined intervals. The mechanism is enabled with ``ignore_and_spike=True`` and configured with ``ignore_and_spike_offset`` and ``ignore_and_spike_interval``. Both values must be larger than zero.

On creation, the first forced spike is scheduled relative to the current simulation time using the offset. Subsequent spikes follow at the given interval. If the parameters are changed later, the mechanism restarts from the current simulation time using the new offset and interval. If the parameters remain unchanged across consecutive ``Simulate()`` calls, the spike train continues from the previous forced spike using the interval, i.e., the offset is not applied again. Thus, the offset defines the first spike after creation or parameter change, while the interval defines all following spikes until the parameters change.

Let :math:`t_0` denote the simulation time at which a neuron using the mechanism is created or its parameters changed, :math:`o` the offset, and :math:`\Delta t` the interval. The forced spike times are then

.. math::

   t_k = t_0 + o + k \Delta t, \qquad k = 0, 1, 2, \ldots

Thus, the first forced spike occurs at :math:`t_0 + o`, and each following forced spike satisfies

.. math::

   t_{k+1} - t_k = \Delta t.

Currently, the following models are endowed with this mechanism:

- `eprop_iaf_adapt_bsshslm_2020.cpp`
- `eprop_iaf_adapt.cpp`
- `eprop_iaf_bsshslm_2020.cpp`
- `eprop_iaf_psc_delta_adapt.cpp`
- `eprop_iaf_psc_delta.cpp`
- `eprop_iaf.cpp`
- `iaf_psc_delta.cpp`

For developers: All `Node`s that inherit from `ArchivingNode` automatically inherit the `IgnoreAndSpike` class. To enable this functionality for a model that is not listed above, wrap the spike-emission condition in `spike_event_is_due()`.
