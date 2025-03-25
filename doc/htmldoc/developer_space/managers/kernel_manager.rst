.. _kernel_manager:

Kernel Manager
==============

The KernelManager in NEST manages the initialization and finalization of all simulation managers in a specific order to
resolve dependencies and ensure proper setup. It acts as a singleton, providing centralized control over configuration,
status queries, and simulation resets. Additionally, it tracks a fingerprint to detect changes and maintain consistency
across multiple runs.


.. mermaid::

   classDiagram
    class ManagerInterface


    class KernelManager {
        +static create_kernel_manager(): void
        +static destroy_kernel_manager(): void
        +static get_kernel_manager(): KernelManager&
        +initialize(adjust_number_of_threads_or_rng_only: bool): void
        +finalize(adjust_number_of_threads_or_rng_only: bool): void
        +prepare(): void
        +cleanup(): void
        +reset(): void
        +set_status(params: DictionaryDatum&): void
        +get_status(status: DictionaryDatum&): void
        +write_to_dump(msg: string): void
        -KernelManager()
        -~KernelManager()
        -fingerprint_: unsigned long
        -managers_: vector<ManagerInterface*>
        -initialized_: bool
        -dump_: ofstream
        +static kernel_manager_instance_: KernelManager
    }

    KernelManager --|> ManagerInterface: extends

Singleton Pattern: Managed via create_kernel_manager(), destroy_kernel_manager(), and get_kernel_manager().
managers_ Vector: Contains pointers to all managers in initialization/finalization order (e.g., LoggingManager, MPIManager, NodeManager).
Lifecycle Methods: initialize(), finalize(), prepare(), and cleanup() orchestrate the simulation workflow.
Fingerprint Tracking: fingerprint_ increments on initialization or changes to detect configuration updates.
Logging: dump_ handles full logging output, and write_to_dump() ensures thread-safe writes.

.. doxygenclass:: nest::KernelManager
   :members:
