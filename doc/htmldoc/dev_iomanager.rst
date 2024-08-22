.. _iomanager:

Managers
========

This architecture shows a highly modular and hierarchical structure, where the KernelManager acts as the core,
coordinating the other managers that specialize in different aspects of the simulation. Each manager has a specific
role, with many of them interacting closely to facilitate complex neural network simulations.


::

           +---------------------+
           |    KernelManager     |
           +---------+-----------+
                     |
    +----------------+--------------------------+
    |                |                          |
    |          +-----v--------+          +------v-------+
    |          | ModelManager |          | NodeManager  |
    |          +-----+--------+          +------+-------+
    |                |                          |
    |      +---------v-----------+     +--------v-----------+
    |      | ModelRangeManager   |     | ConnectionManager  |
    |      +---------------------+     +--------+-----------+
    |                                          |
    +--------+---------------------+           |
             |                     |           |
 +-----------v------+       +-------v----------v---+
 |  ModuleManager   |       | EventDeliveryManager  |
 +------------------+       +-----------+----------+
                                      |
                                      |
               +------------------+   +-----------------+
               | MusicManager     |   | SimulationManager|
               +--------+---------+   +---------+--------+
                        |                     |
               +--------v---------+   +-------v-----------+
               |    MPImanager    |   | LoggingManager    |
               +--------+---------+   +---------+---------+
                        |                     |
               +--------v---------+   +-------v-----------+
               |   VPManager      |   | RandomManager     |
               +------------------+   +-------------------+

.. mermaid::

   graph TD
    A[KernelManager] --> B[ModelManager]
    A --> C[NodeManager]
    A --> D[ModuleManager]
    A --> E[MusicManager]
    A --> F[SimulationManager]
    A --> G[LoggingManager]
    A --> H[MPImanager]
    A --> I[VPManager]
    A --> J[RandomManager]

    B --> K[ModelRangeManager]
    C --> L[ConnectionManager]
    D --> M[ModuleManager]
    L --> N[EventDeliveryManager]
    F --> O[SimulationManager]

    E --> H
    I --> H
    F --> G
    L --> N


{% for item in cpp_class_list %}
{% if 'Manager' in item %}

{{ item }}

.. doxygenclass:: {{item}}
   :members:


{% endif %}
{% endfor %}


1. io_manager & io_manager_impl
Role: Handles input/output operations for the simulation, likely including file reading/writing, data serialization, etc.
Relationship: Interfaces with other managers like logging_manager and node_manager to handle data persistence and
retrieval.

2. logging_manager
Role: Manages the logging of events, errors, and other significant simulation data.
Relationship: Works with io_manager for writing logs to files and interacts with kernel_manager to ensure all relevant
events are logged.

3. event_delivery_manager & event_delivery_manager_impl
Role: Manages the delivery of events (like spikes in a neural network) between nodes.
Relationship: Closely interacts with node_manager and connection_manager to ensure events are routed correctly
through the network.

4. sp_manager & sp_manager_impl
Role: Likely manages "special" processes or "service processes," given the typical use of "sp" in similar contexts.
Relationship: Interfaces with the kernel_manager and possibly node_manager to handle specialized operations during
simulation.

5. simulation_manager
Role: Oversees the overall simulation process, including starting, stopping, and coordinating the simulation steps.
Relationship: Central to all other managers as it coordinates the simulation's execution. Relies on kernel_manager,
node_manager, and event_delivery_manager.

6. music_manager
Role: Manages integration with the MUSIC (Multi-Simulation Coordinator) framework, which allows coupling of parallel
simulations.
Relationship: Interacts with mpi_manager for distributed simulations and simulation_manager to synchronize the MUSIC
interface.

7. manager_interface
Role: Provides a generic interface that all managers likely inherit or implement.
Relationship: Serves as the base interface for most or all managers, ensuring a consistent API across the system.

8. model_manager & model_manager_impl
Role: Manages the registration and instantiation of neuron, synapse, and other models within the simulation.
Relationship: Interacts with node_manager to create nodes based on these models and with connection_manager for synapse
models.

9. connection_manager & connection_manager_impl
Role: Manages connections between nodes, ensuring proper creation and management of synapses.
Relationship: Works with node_manager to link nodes and event_delivery_manager to ensure connections correctly propagate
events.

10. vp_manager & vp_manager_impl
Role: Manages virtual processes (VPs), which are often used in simulations to handle parallel processing.
Relationship: Works with mpi_manager and simulation_manager to distribute simulation tasks across multiple virtual
processes.

11. modelrange_manager
Role: Likely manages ranges of models, possibly handling the allocation and distribution of model IDs or parameters.
Relationship: Works with model_manager to manage model parameters and ID ranges across the simulation.

12. module_manager
Role: Manages dynamic modules, which may include neuron or synapse models loaded at runtime.
Relationship: Interfaces with kernel_manager and model_manager to register and manage modules dynamically.

13. kernel_manager
Role: Central manager that coordinates other managers and oversees the overall simulation kernel.
Relationship: Interfaces with all other managers, providing global coordination and management.

14. mpi_manager & mpi_manager_impl
Role: Manages MPI communications for parallel and distributed simulations.
Relationship: Works closely with vp_manager, simulation_manager, and music_manager to handle distributed simulation tasks.

15. random_manager
Role: Manages random number generation across the simulation, ensuring consistency and reproducibility.
Relationship: Interfaces with kernel_manager and node_manager to provide random numbers where needed, such as in noise
generation or random connectivity.
