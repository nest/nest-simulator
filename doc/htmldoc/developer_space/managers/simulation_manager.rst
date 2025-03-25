.. _simulation_manager:

Simulation Manager
==================

The SimulationManager in NEST is responsible for controlling the simulation workflow, managing time progression, and
coordinating computational steps across nodes. It tracks simulation duration, active nodes, and parallel execution
details (e.g., MPI processes/OpenMP threads), ensuring synchronization and logging critical progress. It handles time
slicing, updates, and ensures consistent state transitions between preparation, execution, and cleanup phases.

.. .. mermaid::

  classDiagram
      class SimulationManager {
          +run(double duration)
          +has_been_simulated()
          +prepare()
          +initialize()
      }

      SimulationManager --> KernelManager : manages
      SimulationManager --> NodeManager : manages
      SimulationManager --> ConnectionManager : manages
      SimulationManager --> EventManager : manages
      SimulationManager --> DataLoggerManager : manages
      SimulationManager --> RandomNumberManager : manages
      SimulationManager --> ModuleManager : manages
      SimulationManager --> NetworkManager : manages


.. mermaid::

   classDiagram
    class ManagerInterface {
        +virtual ~ManagerInterface()
        // Other ManagerInterface methods/attributes
    }

    class SimulationManager {
        <<extends ManagerInterface>>
        +SimulationManager()
        +start_updating_()
        +has_been_prepared() bool
        +get_slice() size_t
        +get_clock() const Time&
        +run_duration() const Time
        +run_start_time() const Time
        +run_end_time() const Time
        +get_from_step() const long
        -clock_ Time
        -slice_ long
        -to_do_ long
        -to_do_total_ long
        -from_step_ long
        -to_step_ long
        -t_real_ long
        -prepared_ bool
        -simulating_ bool
        -simulated_ bool
        // Other private members like t_slice_begin_, t_slice_end_
    }

    SimulationManager --|> ManagerInterface : Inherits

.. doxygenclass:: nest::SimulationManager
   :members:
