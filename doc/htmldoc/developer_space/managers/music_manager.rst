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


.. list-table:: Interactions between MUSICManager and Other Managers
   :header-rows: 1

   * - **Manager/Component**
     - **Interaction with MUSICManager**
     - **Code Evidence**
   * - **KernelManager**
     - - Initializes and finalizes the MUSICManager during simulation setup and shutdown.
     - ``MUSICManager``` is included in ``kernel_manager.h``, and its ``initialize()``/``finalize()`` methods are called.
   * - **MPIManager**
     - - Manages MPI communication and finalization.
     - ``MPI_Finalize()`` is called in ``music_finalize()``, and ``MPI_THREAD_FUNNELED`` is used in ``init_music()``.
   * - **NodeManager**
     - - Registers nodes (e.g., proxies) with MUSIC ports and channels.
     - ``register_music_event_in_proxy()`` and ``register_music_rate_in_proxy()`` take ``nest::Node*`` pointers.
   * - **MusicEventHandler**
     - - Manages event-based communication with MUSIC ports.
     - ``MusicEventHandler`` is part of ``music_event_in_portmap_`` and handles event delivery.
   * - **MusicRateInHandler**
     - - Manages rate-based communication with MUSIC ports.
     - `MusicRateInHandler` is part of ``music_rate_in_portmap_`` and handles continuous data.
   * - **EventDeliveryManager**
     - - Implicitly involved in delivering events to nodes (via ``SpikeEvent``).
     - ``MusicEventHandler::update()`` calls ``channelmap_[channel]->handle(se)``, which may involve event delivery.
   * - **ConnectionManager**
     - - Provides minimum delay for rate data buffering (indirect dependency).
     - ``MusicRateInHandler`` uses ``connection_manager.get_min_delay()`` (from earlier code).


Class diagram
-------------

.. mermaid::

    classDiagram
      class MUSICManager {
        +std::map<std::string, MusicPortData> music_in_portlist_
        +std::map<std::string, MusicEventHandler> music_event_in_portmap_
        +std::map<std::string, MusicRateInHandler> music_rate_in_portmap_
        +MUSIC::Setup* music_setup
        +MUSIC::Runtime* music_runtime
        +MUSICManager()
        +void initialize(const bool)
        +void finalize(const bool)
        +void set_status(const DictionaryDatum&)
        +void get_status(DictionaryDatum&)
        +void init_music(int*, char***)
        +void enter_runtime(double)
        +void music_finalize()
        +MPI::Intracomm communicator()
        +MUSIC::Setup* get_music_setup()
        +MUSIC::Runtime* get_music_runtime()
        +void advance_music_time()
        +void register_music_in_port(std::string)
        +void unregister_music_in_port(std::string)
        +void register_music_event_in_proxy(std::string, int, nest::Node*)
        +void register_music_rate_in_proxy(std::string, int, nest::Node*)
        +void set_music_in_port_acceptable_latency(std::string, double)
        +void set_music_in_port_max_buffered(std::string, int)
        +void publish_music_in_ports_()
        +void update_music_event_handlers(Time const&, long, long)
      }

      class MusicEventHandler {
          +std::vector< nest::Node* > channelmap_
          +std::vector< std::priority_queue<double> > eventqueue_
          +std::vector<unsigned int> indexmap_
          +MUSIC::EventInput* music_port_
          +MUSIC::PermutationIndex* music_perm_ind_
          +std::string portname_
          +double acceptable_latency_
          +int max_buffered_
          +bool published_
          +MusicEventHandler()
          +MusicEventHandler(std::string, double, int)
          +~MusicEventHandler()
          +void register_channel(size_t, nest::Node*)
          +void publish_port()
          +void operator()(double, MUSIC::GlobalIndex)
          +void update(Time const&, long, long)
      }

      class MusicRateInHandler {
          // Assume similar structure to MusicEventHandler for rate-based ports
          // (specific details may vary based on implementation)
      }

      class MusicPortData {
          +size_t n_input_proxies
          +double acceptable_latency
          +int max_buffered
          +MusicPortData(size_t, double, int)
          +MusicPortData()
      }


      class Node["nest::Node"]


      MUSICManager--MusicEventHandler: manages
      MUSICManager--MusicRateInHandler: manages
      MUSICManager--Node: interacts with
      MusicEventHandler--Node: interacts with
      MusicPortData--MUSICManager: part of


Order of operations
-------------------

* Initialization ``MUSICManager::initialize(const bool)``

    This function is called to initialize the MUSICManager.

* Set up MUSIC ``MUSICManager::init_music(int* argc, char** argv[])``

    This function is called to initialize the MUSIC setup. It creates a MUSIC::Setup object with the provided arguments and MPI thread level.
    Example code:

    .. code-block:: cpp

      void MUSICManager::init_music(int* argc, char** argv[]) {
          int provided_thread_level;
          music_setup = new MUSIC::Setup(*argc, *argv, MPI_THREAD_FUNNELED, &provided_thread_level);
      }

* Enter MUSIC runtime ``MUSICManager::enter_runtime(double h_min_delay)``

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

* Update MUSIC event handlers ``MUSICManager::update_music_event_handlers(Time const& origin, const long from, const long to)``

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

* Advance MUSIC time ``MUSICManager::advance_music_time()``

    This function is called to advance the music time by calling the tick() method on the MUSIC::Runtime object.
    Example code:

    .. code-block:: cpp

        void MUSICManager::advance_music_time() {
            music_runtime->tick();
        }

* Finalize MUSIC  ``MUSICManager::music_finalize()``

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




MPI relationship
----------------

The MUSICManager and MPI work together to enable distributed simulations in NEST. The MUSICManager initializes MPI,
retrieves an MPI communicator, and ensures proper finalization. It uses MPI for communication between processes,
allowing for seamless integration of MUSIC into the simulation. This setup ensures that the simulation can scale across
multiple processors, leveraging the power of distributed computing.


Key Points:

    MPI Initialization:
        The MUSICManager initializes MPI during the simulation setup. This is typically done in the init_music method, where it creates a MUSIC::Setup object with MPI thread support.

      .. code-block:: cpp

        void MUSICManager::init_music(int* argc, char** argv[]) {
          int provided_thread_level;
          music_setup = new MUSIC::Setup(*argc, *argv, MPI_THREAD_FUNNELED, &provided_thread_level);
        }

MPI Communication:

The MUSICManager uses MPI for communication between different processes in a distributed simulation.
It retrieves an MPI communicator from the MUSIC setup and uses it for various operations, such as publishing ports and handling events.


    .. code-block:: cpp

      MPI::Intracomm MUSICManager::communicator() {
        return music_setup->communicator();
      }

Finalization:

The MUSICManager ensures that MPI is properly finalized during the simulation shutdown. This is done in the music_finalize method, which calls MPI_Finalize() if necessary.

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


Functions
---------

.. doxygenclass:: nest::MUSICManager
   :members:
   :private-members:
   :undoc-members:


deepseek
--------

* Initialization: The MUSICManager constructor initializes the necessary data structures, such as maps for managing ports
  and handlers.
* Initialization of MUSIC: The init_music function is called to initialize the MUSIC setup and runtime. This involves
  creating a MUSIC::Setup object and potentially a MUSIC::Runtime object based on the provided command-line arguments.
* Entering Runtime: The enter_runtime function is called to start the MUSIC runtime. It initializes the runtime with the
  specified minimum delay and publishes all registered ports.
* Publishing Ports: The ``publish_music_in_ports_`` function is called to ensure all registered ports are published,
  allowing them to communicate over the network.
* Updating Handlers: The ``update_music_event_handlers`` function is called to update event and rate handlers with timing
  information from the origin, from, and to nodes.
* Finalizing: The music_finalize function is called to clean up the MUSIC runtime and, if enabled, the MPI communication.
* Registering Ports and Proxies: Functions like ``register_music_in_port``, ``unregister_music_in_port``,
  ``register_music_event_in_proxy``, and ``register_music_rate_in_proxy`` are used to manage the registration of ports
  and proxies, ensuring that the simulation is properly configured.
* Setting Port Properties: Functions like ``set_music_in_port_acceptable_latency`` and ``set_music_in_port_max_buffered``
  allow fine-grained control over the behavior of each port.

code_ai
--------


    Initialization and Finalization:
        Initialization: The initialize method is called during the kernel initialization process. It sets up the necessary data structures.
        Finalization: The finalize method is called during the simulation shutdown process. It cleans up the MUSIC runtime and, if enabled, the MPI communication.
    MUSIC Setup and Runtime:
        Initialization of MUSIC: The init_music method initializes the MUSIC setup and runtime using the provided command-line arguments. It creates a MUSIC::Setup object and, if necessary, a MUSIC::Runtime object.
        Entering Runtime: The enter_runtime method is called to start the MUSIC runtime. It publishes all registered ports and logs a message indicating the start of the runtime.
    Port Management:
        Registering Ports: The register_music_in_port method registers a MUSIC input port with the manager. It increments the counter for the respective entry in the music_in_portlist_.
        Unregistering Ports: The unregister_music_in_port method unregisters a MUSIC input port. It decrements the counter and removes the entry if the counter drops to zero.
    Event Handling:
        Registering Event Proxies: The register_music_event_in_proxy method registers a node (of type music_input_proxy) with a given MUSIC port and channel. The proxy will be notified if a MUSIC event is received on the respective channel and port.
    Setting Port Properties:
        Setting Acceptable Latency: The set_music_in_port_acceptable_latency method sets the acceptable latency for a MUSIC input port.
        Setting Maximum Buffered Events: The set_music_in_port_max_buffered method sets the maximum number of buffered events for a MUSIC input port.
    Publishing Ports:
        The publish_music_in_ports_ method iterates over the music_event_in_portmap_ and music_rate_in_portmap_ maps and calls their respective publish_port() methods to send data over the network.
    Updating Handlers:
        The update_music_event_handlers method updates event and rate handlers with timing information from the origin, from, and to nodes.

reworked list reasoning + code
-------------------------------


    Constructor (MUSICManager()):
        Initializes the MUSICManager object, setting up necessary data structures.
    Initialization (initialize(const bool)):
        Initializes the MUSICManager during the kernel initialization process.
    Finalization (finalize(const bool)):
        Cleans up the MUSICManager during the simulation shutdown process.
    MUSIC Setup and Runtime:
        init_music(int* argc, char** argv[]): Initializes the MUSIC setup and runtime using the provided command-line arguments.
        enter_runtime(double h_min_delay): Starts the MUSIC runtime, publishing all registered ports and logging a message.
    Port Management:
        register_music_in_port(std::string portname): Registers a MUSIC input port with the manager.
        unregister_music_in_port(std::string portname): Unregisters a MUSIC input port.
    Event Handling:
        register_music_event_in_proxy(std::string portname, int channel, nest::Node* mp): Registers a node with a given MUSIC port and channel for event-based processing.
        publish_music_in_ports_(): Iterates over event handlers and publishes their ports.
        update_music_event_handlers(Time const& origin, const long from, const long to): Updates event handlers with timing information.
    Rate Handling:
        register_music_rate_in_proxy(std::string portname, int channel, nest::Node* mp): Registers a node with a given MUSIC port and channel for rate-based processing.
        publish_music_in_ports_(): Iterates over rate handlers and publishes their ports.
        update_music_event_handlers(Time const& origin, const long from, const long to): Updates rate handlers with timing information.
    MUSIC Event and Rate Handlers:
        MusicEventHandler: Manages event-based communication, handling spikes or events from MUSIC ports.
        MusicRateInHandler: Manages continuous data streams, like rates, mapping data from MUSIC ports into NEST's internal data structures.
    Cleanup:
        music_finalize(): Cleans up the MUSIC runtime and, if enabled, the MPI communication.
    Accessors:
        get_music_setup(): Returns a pointer to the MUSIC::Setup object.
        get_music_runtime(): Returns a pointer to the MUSIC::Runtime object.
