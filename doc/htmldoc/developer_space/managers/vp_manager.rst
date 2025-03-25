.. _vp_manager:

VP Manager
==========

The VPManager in NEST coordinates virtual processes (VPs) for parallel execution, distributing computational tasks
across threads and MPI ranks. It maps node IDs to VPs, ensures thread safety, and manages MPI rank assignments to enable
efficient distributed simulations. The manager provides VP-to-thread and MPI rank conversions to synchronize parallel
operations.

.. mermaid::

  classDiagram
    class ManagerInterface

    class VPManager {
        +VPManager()
        +~VPManager()
        +initialize(adjust_number_of_threads_or_rng_only: bool): void
        +finalize(adjust_number_of_threads_or_rng_only: bool): void
        +get_vp() const: size_t
        +get_num_virtual_processes() const: size_t
        +get_thread_id() const: size_t
        +get_num_threads() const: size_t
        +get_mpi_rank() const: size_t
        +get_num_mpi_processes() const: size_t
        +node_id_to_vp(node_id: size_t) const: size_t
        +is_local_vp(vp: size_t) const: bool
        +vp_to_thread(vp: size_t) const: size_t
        -assert_single_threaded() const: void
        -assert_thread_parallel() const: void
        -n_threads_: size_t
        -vp_: size_t
        -mpi_manager_: MPIManager&
        -num_virtual_processes_: size_t
    }

    VPManager --|> ManagerInterface: extends

.. doxygenclass:: nest::VPManager
   :members:
