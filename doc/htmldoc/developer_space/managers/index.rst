Managers
========

This architecture shows a highly modular and hierarchical structure, where the KernelManager acts as the core,
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

.. mermaid::

   sequenceDiagram
    participant KernelManager
    participant SimulationManager
    participant NodeManager
    participant ConnectionManager
    participant EventDeliveryManager
    participant IOManager
    participant ModelManager
    participant MUSICManager

    KernelManager->>SimulationManager: Start Simulation
    SimulationManager->>NodeManager: Update nodes
    NodeManager->>ModelManager: Get model parameters
    NodeManager->>ConnectionManager: Send events
    ConnectionManager->>EventDeliveryManager: Deliver events
    EventDeliveryManager->>NodeManager: Notify target nodes
    SimulationManager->>IOManager: Record data
    SimulationManager->>MUSICManager: Send/receive external data
    SimulationManager->>KernelManager: Simulation complete



Documentation for Managers
~~~~~~~~~~~~~~~~~~~~~~~~~~

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
