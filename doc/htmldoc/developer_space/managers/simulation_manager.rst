.. _simulation_manager:

Simulation Manager
==================

The SimulationManager in NEST is responsible for controlling the simulation workflow, managing time progression, and
coordinating computational steps across nodes. It tracks simulation duration, active nodes, and parallel execution
details (e.g., MPI processes/OpenMP threads), ensuring synchronization and logging critical progress. It handles time
slicing, updates, and ensures consistent state transitions between preparation, execution, and cleanup phases.


.. doxygenclass:: nest::SimulationManager
   :members:
