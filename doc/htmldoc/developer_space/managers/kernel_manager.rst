.. _kernel_manager:

Kernel Manager
==============

The KernelManager in NEST manages the initialization and finalization of all simulation managers in a specific order to
resolve dependencies and ensure proper setup. It acts as a singleton, providing centralized control over configuration,
status queries, and simulation resets. Additionally, it tracks a fingerprint to detect changes and maintain consistency
across multiple runs.



.. doxygenclass:: nest::KernelManager
   :members:
