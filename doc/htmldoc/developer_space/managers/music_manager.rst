.. _music_manager:

MUSIC manager
=============

The MUSICManager class in NEST provides a comprehensive interface for managing interactions with the MUSIC interface.
It handles port registration and coordinates
communication between NEST and other simulators via MUSIC. It uses MPI for parallel communication and ensures proper
initialization and cleanup. The code structure uses maps to track ports and their associated handlers, allowing dynamic
registration of input proxies.

Relationships between MUSICManager and Other Managers
-----------------------------------------------------

.. list-table::
   :header-rows: 1

   * - Manager
     - Relevance
   * - KernelManager
     - Ensures proper initialization and finalization of the MUSIC environment within the NEST kernel.
   * - MPIManager
     - Ensures proper initialization and finalization of MPI when MUSIC is enabled.
   * - SimulationManager
     - Ensures proper synchronization and communication with MUSIC during the simulation.
   * - NodeManager
     - Registers music input proxies with specific nodes and channels.
   * - ManagerInterface
     - Ensures proper integration with the NEST framework.


Class diagram
-------------

.. mermaid::

   classDiagram
    class MUSICManager {
        +MUSICManager()
        +initialize(const bool)
        +finalize(const bool)
        +set_status(const DictionaryDatum&)
        +get_status(DictionaryDatum&)
        +init_music(int* argc, char** argv[])
        +enter_runtime(double h_min_delay)
        +music_finalize()
        +communicator()
        +get_music_setup()
        +get_music_runtime()
        +advance_music_time()
        +register_music_in_port(std::string portname)
        +unregister_music_in_port(std::string portname)
        +register_music_event_in_proxy(std::string portname, int channel, nest::Node* mp)
        +register_music_rate_in_proxy(std::string portname, int channel, nest::Node* mp)
        +set_music_in_port_acceptable_latency(std::string portname, double latency)
        +set_music_in_port_max_buffered(std::string portname, int maxbuffered)
        +publish_music_in_ports_()
        +update_music_event_handlers(Time const& origin, const long from, const long to)
    }

    class Setup {
        +MUSIC::Setup(int* argc, char** argv[], MPI_THREAD_LEVEL, int* provided_thread_level)
        +communicator()
    }

    class Runtime {
        +MUSIC::Runtime(MUSIC::Setup* setup, double step_size)
        +tick()
        +finalize()
    }

    class MusicEventHandler {
        +MusicEventHandler(std::string portname, double latency, int max_buffered)
        +register_channel(int channel, nest::Node* mp)
        +publish_port()
        +update(Time const& origin, const long from, const long to)
    }

    class MusicRateInHandler {
        +MusicRateInHandler(std::string portname)
        +register_channel(int channel, nest::Node* mp)
        +publish_port()
        +update(Time const& origin, const long from, const long to)
    }

    class MusicPortData {
        +int n_input_proxies
        +double acceptable_latency
        +int max_buffered
    }

    MUSICManager --> Setup : has
    MUSICManager --> Runtime : has
    MUSICManager --> MusicEventHandler : has
    MUSICManager --> MusicRateInHandler : has
    MUSICManager --> MusicPortData : has

Order of operations
-------------------

* Initialization (MUSICManager::initialize(const bool)):

    This function is called to initialize the MUSICManager. However, in the provided code, it currently does nothing.

* Setting Up MUSIC (MUSICManager::init_music(int* argc, char** argv[])):

    This function is called to initialize the MUSIC setup. It creates a MUSIC::Setup object with the provided arguments and MPI thread level.
    Example code:

    .. code-block:: cpp

      void MUSICManager::init_music(int* argc, char** argv[]) {
          int provided_thread_level;
          music_setup = new MUSIC::Setup(*argc, *argv, MPI_THREAD_FUNNELED, &provided_thread_level);
      }

* Entering MUSIC Runtime (MUSICManager::enter_runtime(double h_min_delay)):

    This function is called to enter the MUSIC runtime. It publishes music in ports, logs a message indicating the entry into the MUSIC runtime, and creates a MUSIC::Runtime object if it doesn't already exist.
    Example code:

    .. code-block:: cpp

        void MUSICManager::enter_runtime(double h_min_delay) {
            publish_music_in_ports_();
            std::string msg = String::compose("Entering MUSIC runtime with tick = %1 ms", h_min_delay);
            LOG(M_INFO, "MUSICManager::enter_runtime", msg);

            if (music_runtime == 0) {
                music_runtime = new MUSIC::Runtime(music_setup, h_min_delay * 1e-3);
            }
        }

* Updating MUSIC Event Handlers (MUSICManager::update_music_event_handlers(Time const& origin, const long from, const long to)):

    This function is called to update all music event and rate in handlers. It iterates through the event and rate handlers and updates them with the given origin, from, and to values.
    Example code:

    .. code-block:: cpp

        void MUSICManager::update_music_event_handlers(Time const& origin, const long from, const long to) {
            for (std::map<std::string, MusicEventHandler>::iterator it = music_event_in_portmap_.begin();
                 it!= music_event_in_portmap_.end();
                 ++it) {
                it->second.update(origin, from, to);
            }

            for (std::map<std::string, MusicRateInHandler>::iterator it = music_rate_in_portmap_.begin();
                 it!= music_rate_in_portmap_.end();
                 ++it) {
                it->second.update(origin, from, to);
            }
        }

* Advancing MUSIC Time (MUSICManager::advance_music_time()):

    This function is called to advance the music time by calling the tick() method on the MUSIC::Runtime object.
    Example code:

    .. code-block:: cpp

        void MUSICManager::advance_music_time() {
            music_runtime->tick();
        }

* Finalizing MUSIC (MUSICManager::music_finalize()):

    This function is called to finalize the MUSICManager. It finalizes the MUSIC runtime if it exists, deletes the MUSIC::Runtime object, and if MPI is enabled, it finalizes MPI.
    Example code:

    .. code-block:: cpp

      void MUSICManager::music_finalize() {
          if (music_runtime == 0) {
              music_runtime = new MUSIC::Runtime(music_setup, 1e-3);
          }

          music_runtime->finalize();
          delete music_runtime;

          #ifdef HAVE_MPI
              MPI_Finalize();
          #endif
      }

.. mermaid::

   sequenceDiagram
    participant KernelManager
    participant MUSICManager
    participant MPIManager
    participant SimulationManager
    participant NodeManager

    KernelManager->>MUSICManager: initialize()
    KernelManager->>MUSICManager: init_music(argc, argv)
    KernelManager->>MUSICManager: enter_runtime(h_min_delay)
    KernelManager->>SimulationManager: update_music_event_handlers(origin, from, to)
    KernelManager->>MUSICManager: advance_music_time()
    KernelManager->>MUSICManager: music_finalize()
    KernelManager->>MPIManager: MPI_Finalize()


Functions
---------

.. doxygenclass:: nest::MUSICManager
   :members:
   :private-members:
   :undoc-members:
