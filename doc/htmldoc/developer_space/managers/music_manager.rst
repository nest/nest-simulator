.. _music_manager:

MUSIC manager
=============

The MUSICManager class in NEST provides a comprehensive interface for managing interactions with the MUSIC
(MUlti-Simulation Coordinator) interface.
It handles port registration and coordinates communication between NEST and other applications via MUSIC.
It uses MPI for parallel communication and ensures proper initialization and cleanup. The code structure uses
maps to track ports and their associated handlers, allowing dynamic registration of input proxies.


Overview of steps
------------------

Initialization:

    * :cpp:func:`init_music <nest::MUSICManager::init_music>`:
      Initializes the MUSIC setup with MPI thread settings.

Runtime Management:

    * :cpp:func:`enter_runtime <nest::MUSICManager::enter_runtime>`: Enters the runtime mode, sets up ports, and initializes the runtime if not already done.
    * :cpp:func:`advance_music_time <nest::MUSICManager::advance_music_time>`: Advances the simulation time by one tick.

Port Management:

    * :cpp:func:`register_music_in_port <nest::MUSICManager::register_music_in_port>`: Registers an input port for MUSIC, tracking the number of input proxies.
    * :cpp:func:`unregister_music_in_port <nest::MUSICManager::unregister_music_in_port>`: Unregisters a port, removing it if no proxies are connected.
    * :cpp:func:`set_music_in_port_acceptable_latency <nest::MUSICManager::set_music_in_port_acceptable_latency>`: Sets the acceptable latency for a port.
    * :cpp:func:`set_music_in_port_max_buffered <nest::MUSICManager::set_music_in_port_max_buffered>`: Sets the maximum number of buffered events for a port.
    * :cpp:func:`publish_music_in_ports_ <nest::MUSICManager::publish_music_in_ports_>`: Publishes all registered input ports for external connections.

Event Handling:

    * :cpp:func:`register_music_event_in_proxy <nest::MUSICManager::register_music_event_in_proxy>`: Registers an event handler for a specific port and channel, linking it to a simulation node.
    * :cpp:func:`register_music_rate_in_proxy <nest::MUSICManager::register_music_rate_in_proxy>`: Registers a rate-based input handler for a port and channel.
    * :cpp:func:`update_music_event_handlers <nest::MUSICManager::update_music_event_handlers>`: Updates all event handlers with new time boundaries, allowing them to process events within the specified timeframe.

Relationships wth other managers and components
-----------------------------------------------


.. list-table:: Interactions with other managers and components
   :header-rows: 1

   * - **Manager/Component**
     - **Role in MUSICManager Operations**
     - **Example Interaction**
   * - :ref:`KernelManager <kernel_manager>`
     - - Initializes and finalizes the MUSICManager during simulation setup and shutdown.
     - ``MUSICManager``` is included in ``kernel_manager.h``, and its ``initialize()``/``finalize()`` methods are called.
   * - :ref:`MPIManager <mpi_manager>`
     - - Manages MPI communication and finalization.
     - ``MPI_Finalize()`` is called in ``music_finalize()``, and ``MPI_THREAD_FUNNELED`` is used in ``init_music()``.
   * - :ref:`NodeManager <node_manager>`
     - - Registers nodes (e.g., proxies) with MUSIC ports and channels.
     - ``register_music_event_in_proxy()`` and ``register_music_rate_in_proxy()`` take ``nest::Node*`` pointers.
   * - :ref:`EventDeliveryManager <event_delivery_manager>`
     - - Implicitly involved in delivering events to nodes (via ``SpikeEvent``).
     - ``MusicEventHandler::update()`` calls ``channelmap_[channel]->handle(se)``, which may involve event delivery.
   * - :ref:`ConnectionManager <connection_manager>`
     - - Provides minimum delay for rate data buffering (indirect dependency).
     - ``MusicRateInHandler`` uses ``connection_manager.get_min_delay()`` (from earlier code).
   * - **MusicEventHandler**
     - - Manages event-based communication with MUSIC ports.
     - ``MusicEventHandler`` is part of ``music_event_in_portmap_`` and handles event delivery.
   * - **MusicRateInHandler**
     - - Manages rate-based communication with MUSIC ports.
     - `MusicRateInHandler` is part of ``music_rate_in_portmap_`` and handles continuous data.


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
      MUSICManager--Node: interacts with
      MusicEventHandler--Node: interacts with
      MusicPortData--MUSICManager: part of
      MUSICManager--MusicRateInHandler: manages


Detailed operation sequence
---------------------------

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
   :protected-members:
