.. _managers_dev:

Managers in NEST
================

At the heart of NEST are various
managers, each responsible for specific aspects of the simulation process.


The ManagerInterface provides a common interface for all managers, defining standard methods and properties.

The KernelManager acts as the core, coordinating the other managers that specialize in different aspects of the
simulation.

:ref:`KernelManager <kernel_manager>` initializes all managers in dependency order:

- :ref:`nest::LoggingManager <logging_manager>`
- :ref:`nest::MPIManager <mpi_manager>`
- :ref:`nest::VPManager <vp_manager>`
- :ref:`nest::ModuleManager <module_manager>`
- :ref:`nest::RandomManager <random_manager>`
- :ref:`nest::SimulationManager <simulation_manager>`
- :ref:`nest::ModelRangeManager <modelrange_manager>`
- :ref:`nest::ConnectionManager <connection_manager>`
- :ref:`nest::SPManager <sp_manager>`
- :ref:`nest::EventDeliveryManager<event_delivery_manager>`
- :ref:`nest::IOManager <io_manager>`
- :ref:`nest::ModelManager <model_manager>`
- :ref:`nest::MUSICManager <music_manager>`
- :ref:`nest::NodeManager <node_manager>`



Main control flow
-----------------

1. **Initialization**:
   - The Kernel Manager initializes the simulation environment, allocating resources and setting up initial configurations.
   - The Simulation Manager sets up the simulation based on defined parameters.

2. **Node and Connection Setup**:
   - The Node Manager creates and configures nodes.
   - The Connection Manager establishes connections between nodes.

3. **Model Application**:
   - The Model Manager loads and applies models to nodes and connections.
   - The SP Manager applies synaptic plasticity rules as needed.

4. **Simulation Execution**:
   - The Simulation Manager oversees the execution phase, with the Event Delivery Manager ensuring events are processed correctly.
   - The MPI Manager handles parallel processing tasks if the simulation is distributed.

5. **Data Handling**:
   - The IO Manager reads configuration files and writes simulation results.
   - The Logging Manager captures diagnostic information for debugging.

6. **Cleanup**:
   - The Simulation Manager ensures resources are cleaned up after the simulation completes.


Advanced Architectural Features
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- **Policy-based template pattern for manager customization**:
  - The policy-based template pattern is used to customize specific manager behaviors, allowing developers to extend functionality without modifying core logic.

- **Thread-local storage for performance-critical components**:
  - Thread-local storage is employed for performance-critical components to minimize contention and improve parallel execution efficiency.

- **Double dispatch pattern for event handling**:
  - The double dispatch pattern is used for event handling, enabling dynamic and type-safe event processing across different node and connection types.

- **MPI-agnostic interfaces through manager abstraction**:
  - MPI-agnostic interfaces are provided through manager abstraction, allowing the simulation to run seamlessly with or without MPI, enhancing flexibility and portability.

- **Multiple synchronization strategies**:

  - **MPI_Barrier** is used for global synchronization to ensure all processes reach a certain point before proceeding.
  - **OpenMP locks** are employed for shared memory parallelism to manage access to shared resources.
  - **Atomic operations** are used for spike counter updates to ensure thread-safe increments without locks.
  - **RAII patterns** are used for resource management to ensure proper acquisition and release of resources.



.. toctree::
   :hidden:

   connection_manager
   event_delivery_manager
   io_manager
   mpi_manager
   node_manager
   music_manager
   random_manager
   sp_manager
   vp_manager
   logging_manager
   kernel_manager
   module_manager
   modelrange_manager
   simulation_manager
   model_manager
