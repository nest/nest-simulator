.. _modelrange_manager:

ModelRange Manager
==================

The ModelRangeManager in NEST groups node IDs into contiguous ranges per model, enabling efficient model lookups via
binary search and minimizing storage by merging adjacent ranges. It tracks first/last node IDs and provides methods to
retrieve models for specific nodes, ensuring fast access during simulations.


.. mermaid::

  classDiagram
    class ManagerInterface

    class ModelRangeManager {
        +ModelRangeManager()
        +~ModelRangeManager()
        +initialize(adjust_number_of_threads_or_rng_only: bool): void
        +finalize(adjust_number_of_threads_or_rng_only: bool): void
        +add_range(model: size_t, first_node_id: size_t, last_node_id: size_t): void
        +is_in_range(node_id: size_t) const: bool
        +get_model_id(node_id: size_t) const: size_t
        +get_model_of_node_id(node_id: size_t): Model
        +get_contiguous_node_id_range(node_id: size_t) const: const modelrange&
        +begin() const: std::vector<modelrange>::const_iterator
        +end() const: std::vector<modelrange>::const_iterator
        -modelranges_: std::vector<modelrange>
        -first_node_id_: size_t
        -last_node_id_: size_t
    }

    ModelRangeManager --|> ManagerInterface: extends

Key Components Explained:

* modelranges_: A vector of modelrange objects, each representing a contiguous block of node IDs and their associated model.
* add_range(): Adds a new range, merging it with adjacent ranges if possible.
* get_model_id(): Uses binary search to quickly find the model ID for a given node ID.
* is_in_range(): Validates if a node ID exists within any managed range.



.. doxygenclass:: nest::ModelRangeManager
   :members:
