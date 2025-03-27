.. _node_manager:

Node Manager
============

The NodeManager in NEST manages node creation and distribution across computational threads, ensuring parallel execution
efficiency. It tracks node IDs, handles special node types like MUSIC interfaces, and detects network changes to update
states, while storing exceptions from parallel operations for error resolution.

.. mermaid::

   classDiagram
    class ManagerInterface

    class NodeManager {
        +NodeManager()
        +~NodeManager()
        +initialize(adjust_number_of_threads_or_rng_only: bool): void
        +finalize(adjust_number_of_threads_or_rng_only: bool): void
        +add_nodes(model_id: size_t, n: long): NodeCollectionPTR
        +set_status(nodes: NodeCollection&, status_dict: DictionaryDatum&): void
        +get_status(idx: size_t): DictionaryDatum
        +get_node_by_node_id(node_id: size_t): Node*
        +prepare_nodes(): void
        +get_num_active_nodes(): size_t
        +post_run_cleanup(): void
        -add_devices_(model: Model&, min_node_id: size_t, max_node_id: size_t): void
        -add_music_nodes_(model: Model&, min_node_id: size_t, max_node_id: size_t): void
        -destruct_nodes_(): void
        -clear_node_collection_container(): void
        -local_nodes_: vector<SparseNodeArray>
        -wfr_nodes_vec_: vector<vector<Node*>>
        -num_active_nodes_: size_t
        -num_thread_local_devices_: vector<size_t>
        -have_nodes_changed_: bool
        -exceptions_raised_: vector<shared_ptr<WrappedThreadException>>
        -wfr_is_used_: bool
        -wfr_network_size_: size_t
        -sw_construction_create_: Stopwatch
    }

    NodeManager --|> ManagerInterface: extends


Key Components Explained:

* ``local_nodes_``: A thread-local array of nodes, enabling parallel execution.
* ``wfr_nodes_vec_``: Stores nodes using waveform relaxation for specific solvers.
* ``add_nodes()``: Creates nodes of a given model, distributing them across threads.
* ``have_nodes_changed_``: Flags if nodes were added/removed since the last simulation step.
* ``exceptions_raised_``: Captures exceptions from parallel node operations for later handling.

.. doxygenclass:: nest::NodeManager
   :members:
