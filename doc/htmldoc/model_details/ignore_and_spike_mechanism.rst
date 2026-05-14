.. _ignore_and_spike_mechanism:

Ignore-and-spike mechanism for controlled benchmarking
======================================================

When comparing the efficiency of two models, runtime is often used as a benchmark. Since runtime depends on the number of spikes produced by the underlying dynamics, it can be useful to enforce the same number of spikes in both trials. The ignore-and-spike mechanism supports this by ignoring spikes from the internal dynamics and generating spikes at defined intervals. The mechanism is enabled with `ignore_and_spike=True` and configured with `ignore_and_spike_offset` and `ignore_and_spike_interval`. Both values must be larger than zero.

On creation, the first forced spike is scheduled relative to the current simulation time using the offset. Subsequent spikes follow at the given interval. If the parameters are changed later, the mechanism restarts from the current simulation time using the new offset and interval. If the parameters remain unchanged across consecutive `Simulate()` calls, the spike train continues from the previous forced spike using the interval, i.e., the offset is not applied again. Thus, the offset defines the first spike after creation or update, while the interval defines all following spikes until the parameters change.
