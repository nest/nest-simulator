Managers
========

This managers in NEST have a highly modular and hierarchical structure, where the KernelManager acts as the core,
coordinating the other managers that specialize in different aspects of the simulation. Each manager has a specific
role, with many of them interacting closely to facilitate complex neural network simulations.

.. mermaid::

   classDiagram
    class ManagerInterface {
        # Private constructors/assignment
        -- ManagerInterface(ManagerInterface const&)
        -- void operator=(ManagerInterface const&)
        +ManagerInterface()
        +virtual ~ManagerInterface()
    }

    class KernelManager {
        +KernelManager()
        +~KernelManager()
        +initialize()
        +finalize()
        +create_kernel_manager()
        +destroy_kernel_manager()
        +get_kernel_manager()
        # Contains all managers
        -- LoggingManager logging_manager
        -- MPIManager mpi_manager
        -- VPManager vp_manager
        -- ModuleManager module_manager
        -- RandomManager random_manager
        -- SimulationManager simulation_manager
        -- ModelRangeManager modelrange_manager
        -- ConnectionManager connection_manager
        -- SPManager sp_manager
        -- EventDeliveryManager event_delivery_manager
        -- IOManager io_manager
        -- ModelManager model_manager
        -- MUSICManager music_manager
        -- NodeManager node_manager
    }

    class LoggingManager {
        +LoggingManager()
        +~LoggingManager()
    }

    class MPIManager {
        +MPIManager()
        +~MPIManager()
    }

    class VPManager {
        +VPManager()
        +~VPManager()
    }

    class ModuleManager {
        +ModuleManager()
        +~ModuleManager()
    }

    class RandomManager {
        +RandomManager()
        +~RandomManager()
    }

    class SimulationManager {
        +SimulationManager()
        +~SimulationManager()
    }

    class ModelRangeManager {
        +ModelRangeManager()
        +~ModelRangeManager()
    }

    class ConnectionManager {
        +ConnectionManager()
        +~ConnectionManager()
    }

    class SPManager {
        +SPManager()
        +~SPManager()
    }

    class EventDeliveryManager {
        +EventDeliveryManager()
        +~EventDeliveryManager()
    }

    class IOManager {
        +IOManager()
        +~IOManager()
    }

    class ModelManager {
        +ModelManager()
        +~ModelManager()
    }

    class MUSICManager {
        +MUSICManager()
        +~MUSICManager()
    }

    class NodeManager {
        +NodeManager()
        +~NodeManager()
    }

    LoggingManager <|-- ManagerInterface
    MPIManager <|-- ManagerInterface
    VPManager <|-- ManagerInterface
    ModuleManager <|-- ManagerInterface
    RandomManager <|-- ManagerInterface
    SimulationManager <|-- ManagerInterface
    ModelRangeManager <|-- ManagerInterface
    ConnectionManager <|-- ManagerInterface
    SPManager <|-- ManagerInterface
    EventDeliveryManager <|-- ManagerInterface
    IOManager <|-- ManagerInterface
    ModelManager <|-- ManagerInterface
    MUSICManager <|-- ManagerInterface
    NodeManager <|-- ManagerInterface

    MPIManager --> LoggingManager : uses for logging
    VPManager --> MPIManager : depends on MPI setup
    NodeManager --> ModelManager : requires models
    EventDeliveryManager --> ConnectionManager : uses connections
    SimulationManager --> NodeManager : manages nodes
    MUSICManager --> NodeManager : interfaces with nodes



Documentation for Managers
--------------------------

- :ref:`nest::MPIManager <mpi_manager>`
- :ref:`nest::SPManager <sp_manager>`
- :ref:`nest::SimulationManager <simulation_manager>`
- :ref:`nest::LoggingManager <logging_manager>`
- :ref:`nest::RandomManager <random_manager>`
- :ref:`nest::ManagerInterface <manager_interface>`
- :ref:`nest::EventDeliveryManager<event_delivery_manager>`
- :ref:`nest::IOManager <io_manager>`
- :ref:`nest::ModelRangeManager <modelrange_manager>`
- :ref:`nest::KernelManager <kernel_manager>`
- :ref:`nest::VPManager <vp_manager>`
- :ref:`nest::MUSICManager <music_manager>`
- :ref:`nest::NodeManager <node_manager>`
- :ref:`nest::ModelManager <model_manager>`
- :ref:`nest::ConnectionManager <connection_manager>`


Core Simulation Infrastructure
-------------------------------

KernelManager (nestkernel/kernel_manager.*)

- Serves as central coordinator initializing all other managers
- Implements singleton pattern for global access [architecture pattern evident in header]
- Manages simulation lifecycle through initialize()/finalize() methods
- SimulationManager (nestkernel/simulation_manager.*)
- Controls simulation temporal dynamics:

.. code-block:: cpp

  void run(double time);  // Executes simulation for 'time' ms
  Interfaces with EventDeliveryManager for spike processing

* Collaborates with NodeManager for state updates
* NodeManager (nestkernel/node_manager.*)
* Maintains neuron and device instances
* Implements spatial decomposition:

.. code-block:: cpp

  vector<thread> threads_;  // Per-thread node storage
  Directly interacts with ConnectionManager for network structure

Parallel Processing System
~~~~~~~~~~~~~~~~~~~~~~~~~~

VPManager (nestkernel/vp_manager.*)

- Manages virtual processes (VPs) with thread affinity control:

.. code-block:: cpp

  void set_num_threads(size_t);  // Configures OpenMP threads
  Works with MPIManager for distributed computation

MPIManager (nestkernel/mpi_manager.*)

* Handles MPI communication patterns:

.. code-block:: cpp

  void communicate(std::vector<SpikeData>& spikes);  // Spike exchange
  Implements synchronization barriers for distributed synchronization

Network Modeling Components
~~~~~~~~~~~~~~~~~~~~~~~~~~~

ConnectionManager (nestkernel/connection_manager.*)

* Manages synaptic connections with:

.. code-block:: cpp

  std::vector<ConnectorBase*> connections_;  // Connection storage
  Uses EventDeliveryManager for spike transmission

* Interfaces with ModelManager for connection rule templates

ModelManager (nestkernel/model_manager.*)

* Maintains model prototypes using factory pattern:

.. code-block:: cpp

  template <typename ModelT>
  void register_model(const std::string& name);
  Provides model metadata to NodeManager and ConnectionManager

Event Processing Pipeline
~~~~~~~~~~~~~~~~~~~~~~~~~

EventDeliveryManager (nestkernel/event_delivery_manager.*)

* Implements spike delivery algorithm:

.. code-block:: cpp

  void deliver_events();  // Core event loop
  Uses OffGridSpike structure for precise timing:

.. code-block:: cpp

  struct OffGridSpike {
    double time;  // Precise spike time
    size_t node;  // Source node
  };

Supporting Subsystems
~~~~~~~~~~~~~~~~~~~~~

RandomManager (nestkernel/random_manager.*)

* Provides RNG services with parallel stream support:

.. code-block:: cpp

  librandom::RngPtr get_rng(size_t thread) const;
  Implements RNGManager interface for distribution control

LoggingManager (nestkernel/logging_manager.*)

* Implements hierarchical logging through:

.. code-block:: cpp

  void publish_logmsg(const LogLevel level, const std::string& message);
  Integrates with MPI rank-specific log handling

IOManager (nestkernel/io_manager.*)

* Handles input/output operations with:

.. code-block:: cpp

  void register_offline_device(const DeviceNodePTR&);
  Implements checkpoint/restore functionality

Specialized Managers
~~~~~~~~~~~~~~~~~~~~

MUSICManager (nestkernel/music_manager.*)

* Manages MUSIC API integration:

.. code-block:: cpp

  void music_instrument_control(double, double);
  Implements real-time synchronization interfaces

ModelRangeManager (nestkernel/model_range_manager.*)

* Tracks model ID ranges using:

.. code-block:: cpp

  std::map<long, ModelRange> model_ranges_;
  Provides fast lookup for node creation

Interaction Patterns
--------------------

Initialization Phase
~~~~~~~~~~~~~~~~~~~~

KernelManager initializes all managers in dependency order:

  - LoggingManager
  - MPIManager
  - VPManager
  - ModuleManager
  - RandomManager
  - SimulationManager
  - ModelRangeManager
  - ConnectionManager
  - SPManager
  - EventDeliveryManager
  - IOManager
  - ModelManager
  - MUSICManager
  - NodeManager

Simulation Execution
~~~~~~~~~~~~~~~~~~~~

SimulationManager coordinates the main loop:

.. mermaid::

  graph TD
    A[SimulationManager.run] --> B[NodeManager.prepare]
    B --> C[EventDeliveryManager.deliver_events]
    C --> D[NodeManager.update]
    D --> E{Time remaining?}
    E -- Yes --> C
    E -- No --> F[IOManager.flush]

Data Flow
~~~~~~~~~

Typical spike processing path:

``Node → EventDeliveryManager → MPIManager (if distributed) → Target Nodes``


Key design principles visible in the codebase:

* Policy-based template pattern for manager customization
* Thread-local storage for performance-critical components
* Double dispatch pattern for event handling
* MPI-agnostic interfaces through manager abstraction
* The managers employ multiple synchronization strategies:
* MPI_Barrier for global synchronization
* OpenMP locks for shared memory parallelism
* Atomic operations for spike counter updates
* RAII patterns for resource management
* This architecture enables NEST to scale to >10^6 neurons while maintaining flexibility for different simulation paradigms.


.. .. mermaid::

   %%{init: {'theme': 'base', 'themeVariables': { 'clusterBkg': '#f0f4ff'}}}%%
   graph TD
    subgraph Core[Core Coordination]
        KM[[KernelManager]] -.->|initializes| all
        MI[ManagerInterface] -->|interfaces with| KM
    end

    subgraph Simulation[Simulation Control]
        SM[[SimulationManager]] -->|drives| SM_loop[Simulation Loop]
        SM -->|updates| NM
        SM -->|triggers| EDM
        MRM[ModelRangeManager] -->|provides ID ranges| NM
    end

    subgraph Network[Network Infrastructure]
        NM[[NodeManager]] -->|manages| CM
        NM -->|creates| MM
        CM[[ConnectionManager]] -->|uses| EDM
        CM -->|applies| SPM
        MM[ModelManager] -->|prototypes| NM
        MM -->|templates| CM
    end

    subgraph Parallel[Parallelism]
        MPIM[[MPIManager]] -->|syncs| VPM
        VPM[[VPManager]] -->|assigns| ranks[MPI Ranks]
        MUSIC[[MUSICManager]] -->|integrates| MPIM
    end

    subgraph Events[Event Processing]
        EDM[[EventDeliveryManager]] -->|sends| spikes[Spikes]
        EDM -->|receives| MPIM
        SPM[[SPManager]] -->|modifies| CM
    end

    subgraph Support[Support Services]
        IOM[[IOManager]] -->|records| NM
        RM[[RandomManager]] -->|seeds| NM
        RM -->|distributes| CM
        LM[[LoggingManager]] -->|collects| logs[Logs]
    end

    %% Key Interactions
    KM --> SM
    KM --> MPIM
    KM --> VPM

    SM_loop -->|iterates| EDM
    spikes -->|via| MPIM
    MPIM -->|distributes| spikes
    NM -->|requests| RM
    CM -->|requests| RM
    IOM -->|checkpoints| SM
    MUSIC -->|time sync| SM
    SPM -->|updates| NM
    LM -->|all components| LM



.. mermaid::

   %%{init: {'theme': 'base', 'themeVariables': { 'fontSize': '14px'}}}%%
   graph TD
    %% Core Structure
    KM[[KernelManager]] -->|initializes/manages| SM
    KM -->|initializes| MPIM
    KM -->|configures| VPM
    KM -.->|interface| MI[ManagerInterface]

    %% Simulation Flow
    SM[[SimulationManager]] -->|controls| NM
    SM -->|triggers| EDM
    SM -->|coordinates| IOM
    SM -->|syncs with| MUSIC

    NM[[NodeManager]] -->|manages nodes| CM
    NM -->|uses| MRM[ModelRangeManager]
    NM -->|creates from| MM[[ModelManager]]

    CM[[ConnectionManager]] -->|handles events via| EDM
    CM -->|applies| SPM[[SPManager]]

    %% Parallelism
    MPIM[[MPIManager]] <-->|spike exchange| EDM
    VPM[[VPManager]] -->|assigns| ranks[MPI Ranks]
    MPIM -->|syncs| VPM

    %% Support Services
    RM[[RandomManager]] -->|provides RNGs| NM
    RM -->|seeds| CM
    IOM[[IOManager]] -->|records data| SM
    LM[[LoggingManager]] -.->|logs from| all

    classDef core fill:#f4e3d7,stroke:#f28c18;
    classDef sim fill:#d7f4e3,stroke:#18f26c;
    classDef net fill:#e3d7f4,stroke:#8c18f2;
    classDef par fill:#d7e3f4,stroke:#187df2;
    classDef supp fill:#f4d7e3,stroke:#f2188c;

    class KM,MI core;
    class SM,EDM sim;
    class NM,CM,MM,MRM,SPM net;
    class MPIM,VPM,MUSIC par;
    class RM,IOM,LM supp;

1. nest::MPIManager

   Role: Manages Message Passing Interface (MPI) communications between different processes in a parallel simulation.

   Interactions: Interacts with other managers that require parallel processing, such as SimulationManager and NodeManager, to ensure synchronization and data exchange across processes.

2. nest::SPManager

   Role: Manages the scheduling and execution of synaptic events.

   Interactions: Works closely with EventDeliveryManager to ensure that synaptic events are delivered at the correct simulation time.

3. nest::SimulationManager

   Role: Controls the overall simulation process, including initialization, running, and finalization of simulations.

   Interactions: Coordinates with almost all other managers to ensure the simulation runs smoothly. It interacts with MPIManager for parallel execution, EventDeliveryManager for event handling, and KernelManager for core simulation tasks.

4. nest::LoggingManager

   Role: Manages logging and debugging information during the simulation.

   Interactions: Interacts with various managers to collect and log relevant information, ensuring that the logging process does not interfere with the simulation's performance.

5. nest::RandomManager

   Role: Manages random number generation, which is crucial for stochastic processes in neural simulations.

   Interactions: Provides random numbers to other managers and components that require stochastic behavior, such as NodeManager and ConnectionManager.

6. nest::ManagerInterface

   Role: Provides a common interface for all managers, defining standard methods and properties.

   Interactions: Acts as a base class or interface for other managers, ensuring consistency in their design and functionality.

7. nest::EventDeliveryManager

   Role: Manages the delivery of events to neurons and synapses during the simulation.

   Interactions: Works closely with SPManager for synaptic events and SimulationManager to ensure events are delivered at the correct simulation time.

8. nest::IOManager

   Role: Manages input and output operations, including reading configuration files and writing simulation results.

   Interactions: Interacts with SimulationManager to handle I/O tasks at appropriate times during the simulation.

9. nest::ModelRangeManager

   Role: Manages the range of models available in the simulation, including neuron and synapse models.

   Interactions: Works with ModelManager to provide the available models and their configurations.

10. nest::KernelManager

    Role: Manages the core simulation kernel, including time management and state updates.

    Interactions: Interacts with SimulationManager to control the simulation loop and with EventDeliveryManager to handle event processing.

11. nest::VPManager

    Role: Manages virtual processes, which are essential for parallel simulations.

    Interactions: Works with MPIManager to distribute tasks across virtual processes and ensure synchronization.

12. nest::MUSICManager

    Role: Manages interactions with the MUSIC (Multi-Simulation Coordinator) framework, which allows NEST to communicate with other simulators.

    Interactions: Interacts with SimulationManager and EventDeliveryManager to coordinate events and data exchange with external simulators.

13. nest::NodeManager

    Role: Manages the creation, configuration, and deletion of nodes (neurons and devices) in the simulation.

    Interactions: Works with ConnectionManager to handle connections between nodes and with RandomManager for stochastic node properties.

14. nest::ModelManager

    Role: Manages the different models used in the simulation, including neuron and synapse models.

    Interactions: Interacts with ModelRangeManager to provide model configurations and with NodeManager to apply models to nodes.

15. nest::ConnectionManager

    Role: Manages the connections between nodes, including synaptic connections.

    Interactions: Works with NodeManager to establish and manage connections and with EventDeliveryManager to handle event delivery across connections.

.. mermaid::

   %%{init: {'theme': 'base', 'themeVariables': { 'fontSize': '14px'}}}%%
   graph TD
    KM[[KernelManager]] -->|initializes/manages| SM[[SimulationManager]]
    KM -->|initializes| MPIM[[MPIManager]]
    KM -->|configures| VPM[[VPManager]]
    KM -.->|interface| MI[ManagerInterface]

    classDef core fill:#f4e3d7,stroke:#f28c18;
    class KM,MI core;

Description:

The KernelManager is the central coordinator that initializes and manages other managers.

ManagerInterface provides a common abstraction for all managers.


.. mermaid::

  %%{init: {'theme': 'base', 'themeVariables': { 'fontSize': '14px'}}}%%
  graph TD
    SM[[SimulationManager]] -->|controls| NM[[NodeManager]]
    SM -->|triggers| EDM[[EventDeliveryManager]]
    SM -->|coordinates data| IOM[[IOManager]]
    SM -->|syncs with| MUSIC[[MUSICManager]]

    NM -->|uses ID ranges from| MRM[[ModelRangeManager]]
    NM -->|creates nodes from prototypes in| MM[[ModelManager]]

    CM[[ConnectionManager]] -->|handles events via| EDM
    CM -->|applies rules from| SPM[[SPManager]]

    classDef sim fill:#d7f4e3,stroke:#18f26c;
    class SM,EDM sim;
    class NM,CM,MM,MRM,SPM sim;

Description:

SimulationManager drives the simulation by coordinating node updates, event delivery, and data recording.

NodeManager interacts with ModelRangeManager and ModelManager to manage neuron/device instances.

ConnectionManager handles synaptic connections and applies rules from SPManager.


.. mermaid::

   %%{init: {'theme': 'base', 'themeVariables': { 'fontSize': '14px'}}}%%
   graph TD
    MPIM[[MPIManager]] <-->|spike exchange| EDM[[EventDeliveryManager]]
    MPIM -->|syncs processes| VPM[[VPManager]]
    VPM -->|assigns threads/ranks to| ranks[MPI Ranks]

    MUSIC[[MUSICManager]] -->|integrates with MPI communication| MPIM

    classDef par fill:#d7e3f4,stroke:#187df2;
    class MPIM,VPM,MUSIC par;

Description:

MPIManager handles distributed communication (spike exchange) and synchronizes processes.

VPManager assigns threads or MPI ranks for parallel execution.

MUSICManager integrates external MUSIC API for real-time communication.

.. mermaid::

   %%{init: {'theme': 'base', 'themeVariables': { 'fontSize': '14px'}}}%%
   graph TD
    RM[[RandomManager]] -->|provides RNG streams to nodes in| NM[[NodeManager]]
    RM -->|seeds RNGs for connections in| CM[[ConnectionManager]]

    IOM[[IOManager]] -->|records simulation data from nodes in| NM
    IOM -->|coordinates checkpointing with simulation state in| SM[[SimulationManager]]

    LM[[LoggingManager]] -.->|collects logs from all components| All

classDef supp fill:#f4d7e3,stroke:#f2188c;
class RM,IOM,LM,All supp;


Description:

RandomManager provides random number generators for nodes and connections.

IOManager handles data recording and checkpointing during simulations.

LoggingManager collects logs from all components for debugging and monitoring.

Summary of Graphs
Core Structure: Focuses on initialization and management by the KernelManager.

Simulation Flow: Covers the main simulation workflow involving nodes, connections, and event delivery.

Parallelism: Explains how MPI-based distributed processing and thread management work together.

Support Services: Highlights auxiliary services like random number generation, logging, and data I/O.
