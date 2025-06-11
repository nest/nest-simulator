.. _vp_manager:

VP Manager
==========

The VPManager in NEST coordinates virtual processes (VPs) for parallel execution, distributing computational tasks
across threads and MPI ranks. It maps node IDs to VPs, ensures thread safety, and manages MPI rank assignments to enable
efficient distributed simulations. The manager provides VP-to-thread and MPI rank conversions to synchronize parallel
operations.


.. doxygenclass:: nest::VPManager
   :members:
