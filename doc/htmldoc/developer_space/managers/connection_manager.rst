.. _connection_manager:

Connection Manager
==================

The ConnectionManager in NEST efficiently manages synaptic connections between neurons and devices using source-table
structures, supporting dynamic network modifications via structural plasticity and connection rules. It tracks
connection counts, handles parallel execution resizing, and ensures disabled connections are removed to maintain network
integrity.

.. mermaid::

  classDiagram
      class ConnectionManager {
          +create_connection(Node source, Node target, std::string synapse_type)
          +delete_connection(Connection conn)
          +get_max_delay_time_() const
      }

      ConnectionManager --> SimulationManager : manages
      ConnectionManager --> NodeManager : manages

.. mermaid::

   classDiagram
    class ManagerInterface


    class ConnectionManager {
        +ConnectionManager()
        +~ConnectionManager()
        +initialize(adjust_number_of_threads_or_rng_only: bool): void
        +finalize(adjust_number_of_threads_or_rng_only: bool): void
        +connect(source: NodeCollection&, target: NodeCollection&, syn_spec: DictionaryDatum&, conn_spec: DictionaryDatum&, opts: DictionaryDatum&): void
        +connect_to_device_(source: Node&, target: Node&, s_node_id: size_t, tid: size_t, syn_id: synindex, params: DictionaryDatum&, delay: double, weight: double): void
        +connect_from_device_(source: Node&, target: Node&, tid: size_t, syn_id: synindex, params: DictionaryDatum&, delay: double, weight: double): void
        +get_num_connections_(tid: size_t, syn_id: synindex) const: size_t
        +get_source_node_id(tid: size_t, syn_index: synindex, lcid: size_t): size_t
        +get_target_node_id(tid: size_t, syn_id: synindex, lcid: size_t) const: size_t
        +remove_disabled_connections(tid: size_t): void
        +resize_connections(): void
        +restructure_connection_tables(tid: size_t): void
        -increase_connection_count(tid: size_t, syn_id: synindex): void
        -reject_last_target_data(tid: size_t): void
        -source_table_: SourceTable
        -target_table_devices_: TargetTableDevices
        -connections_: vector<vector<ConnectorBase>>
        -num_connections_: vector<vector<size_t>>
        -keep_source_table_: bool
        -has_primary_connections_: bool
        -secondary_connections_exist_: bool
        -use_compressed_spikes_: bool
        -stdp_eps_: double
    }

    ConnectionManager --|> ManagerInterface: extends

Key Components Explained:

* source_table_: Tracks outgoing connections from source nodes.
* target_table_devices_: Manages connections involving devices (e.g., spike generators).
* connections_: A 2D array of ConnectorBase pointers, storing connections per target and synapse type.
* num_connections_: Tracks the number of connections per target and synapse type.
* connect(): Creates connections using a ConnBuilder based on connection specifications.
* remove_disabled_connections(): Cleans up connections to inactive nodes.
* stdp_eps_: Parameter for Spike-Timing-Dependent Plasticity (STDP) calculations.

.. doxygenclass:: nest::ConnectionManager
   :members:
