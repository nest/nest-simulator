.. _sp_manager:

SP Manager
==========

The SPManager in NEST manages structural plasticity by dynamically creating and deleting synapses during simulations
Here's a summary of what the code does:

Initialization and Finalization:
    The SPManager constructor initializes the structural plasticity update interval and other parameters.
    The initialize and finalize methods manage the setup and cleanup of synapse builders and growth curve factories.

Status Management:
    The ``get_status`` method retrieves the current status of structural plasticity and synapse configurations.
    The ``set_status`` method updates the structural plasticity parameters and synapse configurations based on a dictionary of settings.

Delay Management:
    The ``builder_min_delay`` and ``builder_max_delay`` methods return the minimum and maximum delays for synapse builders, respectively.

Disconnection:
    The disconnect method handles the disconnection of synapses between nodes, considering both local and global connections.

Structural Plasticity Updates:
    The ``update_structural_plasticity`` method updates the structural plasticity by creating and deleting synapses based on the current state of the network.
    The ``create_synapses`` and ``delete_synapses_from_pre``/``delete_synapses_from_post`` methods handle the actual creation and deletion of synapses.

Helper Functions:
    Various helper functions like ``serialize_id``, ``global_shuffle``, ``enable_structural_plasticity``, and ``disable_structural_plasticity``
    support the main functionality of the SPManager.


Key functions and their Purposes:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    :cpp:func:`set_status <nest::SPManager::set_status>`:
        Updates the structural plasticity update interval.
        Processes synapse specifications to create and validate SPBuilder instances.
        Ensures that necessary delay parameters are set if non-default delays are used.
    :cpp:func:`update_structural_plasticity <nest::SPManager::update_structural_plasticity>`:
        Iterates over all SPBuilder instances and calls update_structural_plasticity on each.
        Manages the creation and deletion of synapses based on the current state of the network.
    :cpp:func:`create_synapses <nest::SPManager::create_synapses>`:
        Creates synapses between pre-synaptic and post-synaptic elements.
        Uses helper functions like serialize_id and global_shuffle to manage the distribution of synapses.
    :cpp:func:`delete_synapses_from_pre <nest::SPManager::delete_synapses_from_pre>` and :cpp:func:`delete_synapses_from_post <nest::SPManager::delete_synapses_from_post>`:
        Deletes synapses due to the loss of pre-synaptic or post-synaptic elements.
        Communicates the changes to other nodes in a distributed environment.
    :cpp:func:`enable_structural_plasticity <nest::SPManager::enable_structural_plasticity>` and :cpp:func:`disable_structural_plasticity <nest::SPManager::disable_structural_plasticity>`:
        Enable or disable structural plasticity, with checks to ensure it's compatible with the current simulation settings.

Relationships with other managers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1

   * - Manager
     - Role in SPManager Operations
     - Example Interaction
   * - :ref:`ConnectionManager <connection_manager>`
     - Manages synapse creation and deletion
     - ``disconnect()``, ``get_targets()``, ``sp_connect()``
   * - :ref:`NodeManager <node_manager>`
     - Provides neuron-specific data (synaptic elements)
     - ``get_synaptic_elements()``
   * - :ref:`MPIManager <mpi_manager>`
     - Synchronizes structural plasticity changes across processes
     - ``communicate()``
   * - :ref:`SimulationManager <simulation_manager>`
     - Coordinates simulation time for triggering updates
     - Indirect dependency during update intervals
   * - :ref:`RandomManager <random_manager>`
     - Provides randomness for distributing new synapses
     - Random shuffling during ``create_synapses()``

Detailed Operation Sequence
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Here's a breakdown of the operations, especially focusing on the ``update_structural_plasticity`` and related methods,
which are central to the structural plasticity mechanism:

``SPManager::update_structural_plasticity()`` (Main Update Function)

* This is the entry point for updating structural plasticity. It iterates through a list of SPBuilder
  objects (``sp_conn_builders_``). For each SPBuilder, it calls ``update_structural_plasticity(SPBuilder* sp_builder)``.

* ``SPManager::update_structural_plasticity(SPBuilder* sp_builder)`` (Per-Builder Update)

  * Get Vacant and Deleted Elements:

    Calls ``get_synaptic_elements()`` for pre-synaptic elements (e.g., axons) to identify neurons with vacant synaptic
    elements (``pre_vacant_id``, ``pre_vacant_n``) and neurons that should delete synaptic elements (``pre_deleted_id``, ``pre_deleted_n``).

    Calls ``get_synaptic_elements()`` for post-synaptic elements (e.g., dendrites) to identify vacant and deleted
    elements in the post-synaptic population (``post_vacant_id``, ``post_vacant_n``, ``post_deleted_id``, ``post_deleted_n``).

  * MPI Communication (Deletion of Pre-Synaptic Elements):

    ``kernel().mpi_manager.communicate()``: Communicates the ``pre_deleted_id`` and ``pre_deleted_n`` vectors across
    all MPI processes. The results are stored in ``pre_deleted_id_global`` and ``pre_deleted_n_global``. This step ensures that
    all processes know which pre-synaptic neurons have lost synaptic elements.

  * Delete Synapses (Based on Pre-Synaptic Element Loss):

    If any pre-synaptic elements are to be deleted (``pre_deleted_id_global.size() > 0``), calls ``delete_synapses_from_pre()``.

    Calls ``get_synaptic_elements()`` for pre and post synaptic elements to update the vacant and deleted elements (Important).

  * MPI Communication (Deletion of Post-Synaptic Elements):

    ``kernel().mpi_manager.communicate()``: Communicates the ``post_deleted_id`` and ``post_deleted_n`` vectors across all MPI
    processes. The results are stored in ``post_deleted_id_global`` and ``post_deleted_n_global``. This step ensures that all
    processes know which post-synaptic neurons have lost synaptic elements.

  * Delete Synapses (Based on Post-Synaptic Element Loss):

    If any post-synaptic elements are to be deleted (``post_deleted_id_global.size() > 0``), calls delete_synapses_from_post().

    Calls ``get_synaptic_elements()`` for pre and post synaptic elements to update the vacant and deleted elements (Important).

  * MPI Communication (Vacant Elements):

    ``kernel().mpi_manager.communicate()``: Communicates the ``pre_vacant_id``, ``pre_vacant_n``, ``post_vacant_id``,
    and ``post_vacant_n`` vectors across all MPI processes. The results are stored in ``pre_vacant_id_global``,
    ``pre_vacant_n_global``, ``post_vacant_id_global``, and ``post_vacant_n_global``.

  * Create Synapses:

    If there are vacant pre-synaptic and post-synaptic elements (``pre_vacant_id_global.size() > 0`` and ``post_vacant_id_global.size() > 0``), calls ``create_synapses()``.

  * Flag Connection Changes:

    If any synapses were created or deleted, it calls ``kernel().connection_manager.set_connections_have_changed()``.

* ``SPManager::get_synaptic_elements()``

  * This function (which is not fully provided but is used to retrieve the vacant and deleted elements from global nodes) is crucial for determining which neurons are candidates for synapse creation or deletion.

* ``SPManager::delete_synapses_from_pre()``

  * This function deletes synapses based on the loss of pre-synaptic elements.

  * It calls ``kernel().connection_manager.get_targets()`` to determine the target neurons connected to the deleted pre-synaptic neurons.

  * It then iterates through the connectivity information and calls ``kernel().connection_manager.disconnect()`` to
    remove the synapses.

* ``SPManager::delete_synapses_from_post()``

  * This function deletes synapses based on the loss of post-synaptic elements. The logic is similar to
    ``delete_synapses_from_pre()``, but it handles the deletion from the perspective of the post-synaptic neuron.

* ``SPManager::create_synapses()``

  * This function creates new synapses between vacant pre-synaptic and post-synaptic elements.
  * ``serialize_id``: expands the list of ids according to the provided number of synaptic elements.
  * It shuffles the pre-synaptic and post-synaptic neuron IDs using ``global_shuffle()``.
  * It then calls ``sp_conn_builder->sp_connect()`` to actually create the new synapses.

* ``SPManager::disconnect(NodeCollectionPTR sources, ...)``

  * This function disconnects existing synapses based on a given rule.

Class Diagram
~~~~~~~~~~~~~

.. mermaid::

   classDiagram
    class ManagerInterface

    class SPManager {
        +SPManager()
        +~SPManager()
        +initialize(adjust_number_of_threads_or_rng_only: bool): void
        +finalize(adjust_number_of_threads_or_rng_only: bool): void
        +serialize_id(id: vector<size_t>&, n: vector<int>&, res: vector<size_t>&): void
        +global_shuffle(v: vector<size_t>&): void
        +global_shuffle(v: vector<size_t>&, n: size_t): void
        +new_growth_curve(name: Name): GrowthCurve*
        +is_structural_plasticity_enabled(): bool
        -structural_plasticity_update_interval_: double
        -structural_plasticity_enabled_: bool
        -sp_conn_builders_: vector<SPBuilder*>
        -growthcurve_factories_: vector<GenericGrowthCurveFactory*>
        -growthcurvedict_: DictionaryDatum
    }

    SPManager --|> ManagerInterface: extends

.. mermaid::

   classDiagram
    class SPManager {
        +growthcurvedict: DictionaryDatum
        +sp_conn_builders: SPBuilder[]
        +growthcurve_factories: GrowthCurveFactory[]
    }

    class SPBuilder {
        +synapse_model: string
        +pre_element: string
        +post_element: string
    }

    class ConnectionManager {
        +disconnect(...)
        +get_user_set_delay_extrema()
    }

    class Node {
        +synaptic_elements: DictionaryDatum
    }

    class KernelManager {
        +vp_manager: VPManager
        +node_manager: NodeManager
    }

    class GrowthCurveFactory {
        +create(): GrowthCurve
    }

    class GrowthCurve {
        +compute_growth(...)
    }

    class DictionaryDatum {
        +parameters: map<string, any>
    }

    class MPIManager {
        +communicate(...)
    }

    SPManager "1" --> "N" SPBuilder: manages
    SPManager "1" --> "1" ConnectionManager: uses
    SPManager "1" --> "1" KernelManager: depends on
    SPManager "1" --> "N" GrowthCurveFactory: configures
    SPManager "1" --> "1" DictionaryDatum: configures
    SPManager "1" --> "1" MPIManager: communicates via

    SPBuilder --> "1" GrowthCurve: uses
    SPBuilder --> "1" Node: connects to

    KernelManager --> "1" VPManager: manages
    KernelManager --> "1" NodeManager: manages

Sequence Diagram
~~~~~~~~~~~~~~~~

.. mermaid::

   sequenceDiagram
    participant SPManager
    participant SPBuilder
    participant ConnectionManager
    participant Node
    participant MPIManager
    participant KernelManager

    title Structural Plasticity Workflow

    SPManager->>SPManager: set_status(d)
    SPManager->>SPManager: updateValue< double >(d, "interval", ...)
    SPManager->>SPManager: clear sp_conn_builders_
    SPManager->>SPManager: process synapse specs
    SPManager->>SPBuilder: new SPBuilder(...)
    SPBuilder->>SPBuilder: set_name(...)
    SPBuilder->>SPBuilder: validate delays
    SPManager->>SPManager: store SPBuilder in sp_conn_builders_

    SPManager->>SPManager: update_structural_plasticity()
    SPManager->>SPManager: loop over SPBuilder instances
    SPManager->>SPManager: update_structural_plasticity(SPBuilder)
    SPManager->>Node: get_synaptic_elements(pre_name, ...)
    SPManager->>Node: get_synaptic_elements(post_name, ...)
    SPManager->>MPIManager: communicate(pre_deleted_id, ...)
    SPManager->>MPIManager: communicate(post_deleted_id, ...)
    SPManager->>SPManager: delete_synapses_from_pre(...)
    SPManager->>SPManager: delete_synapses_from_post(...)
    SPManager->>SPManager: create_synapses(...)
    SPManager->>ConnectionManager: set_connections_have_changed()

    SPManager->>SPManager: create_synapses(pre_id, ...)
    SPManager->>SPManager: serialize_id(pre_id, ...)
    SPManager->>SPManager: global_shuffle(...)
    SPManager->>SPBuilder: sp_connect(pre_id_rnd, post_id_rnd)

    SPManager->>SPManager: delete_synapses_from_pre(...)
    SPManager->>ConnectionManager: get_targets(pre_deleted_id, ...)
    SPManager->>MPIManager: communicate(connectivity, ...)
    SPManager->>SPManager: global_shuffle(global_targets, ...)
    SPManager->>SPManager: delete_synapse(snode_id, tnode_id, ...)

    SPManager->>SPManager: delete_synapse(snode_id, ...)
    SPManager->>Node: is_local_node_id(snode_id)
    SPManager->>Node: get_node_or_proxy(snode_id)
    Node->>Node: connect_synaptic_element(se_pre_name, -1)
    SPManager->>ConnectionManager: disconnect(tid, syn_id, ...)
    Node->>Node: connect_synaptic_element(se_post_name, -1)

    SPManager->>SPManager: enable_structural_plasticity()
    SPManager->>KernelManager: check thread count
    SPManager->>ConnectionManager: check keep_source_table
    SPManager->>ConnectionManager: check use_compressed_spikes
    SPManager->>SPManager: structural_plasticity_enabled_ = true

Functions
~~~~~~~~~

.. doxygenclass:: nest::SPManager
   :members:
   :private-members:
   :undoc-members:
