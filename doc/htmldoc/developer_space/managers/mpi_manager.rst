.. _mpi_manager:

MPI manager
===========

The MPIManager in NEST manages MPI initialization and configuration checks to ensure proper setup for distributed
simulations. It facilitates inter-process communication using MPI functions such as Allgather and Allreduce, and
synchronizes processes with barriers. Additionally, it handles node-to-process mappings and enforces correct MPI usage
to prevent runtime errors.

.. mermaid::

   classDiagram
    class ManagerInterface

    class MPIManager {
        +MPIManager()
        +~MPIManager()
        +initialize(adjust_number_of_threads_or_rng_only: bool): void
        +finalize(adjust_number_of_threads_or_rng_only: bool): void
        +set_status(params: DictionaryDatum&): void
        +get_status(status: DictionaryDatum&): void
        +init_mpi(argc: int, argv: char*[]): void
        +communicate(send_val: double, recv_buffer: vector<double>&): void
        +synchronize(): void
        +any_true(my_bool: bool): bool
        -comm: MPI_Comm
        -num_processes_: int
        -rank_: int
        -send_counts_secondary_events_in_int_per_rank_: vector<int>
        -recv_counts_secondary_events_in_int_per_rank_: vector<int>
    }

    class OffGridSpike {
        // Members and methods related to off-grid spike handling
    }

    MPIManager --|> ManagerInterface: extends
    MPIManager "1" -- "*" OffGridSpike: contains



.. doxygenclass:: nest::MPIManager
   :members:
