.. _mpi_manager:

MPI manager
===========

The MPIManager in NEST manages MPI initialization and configuration checks to ensure proper setup for distributed
simulations. It facilitates inter-process communication using MPI functions such as Allgather and Allreduce, and
synchronizes processes with barriers. Additionally, it handles node-to-process mappings and enforces correct MPI usage
to prevent runtime errors.

.. doxygenclass:: nest::MPIManager
   :members:
