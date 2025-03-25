.. _random_manager:

Random Manager
==============

The RandomManager in NEST provides three RNG types (rank-synchronized, VP-synchronized, VP-specific) to manage parallel
random number generation, ensuring reproducibility and thread safety. It supports various algorithms (e.g., Mersenne
Twister, Philox) and allows configuration of seeds and types, initializing RNGs with unique seeds derived from base
values and seeder constants for consistent yet independent streams across processes and threads.

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
          +get_model_of_node_id(node_id: size_t): Model*
          +get_contiguous_node_id_range(node_id: size_t) const: const modelrange&
          +begin() const: std::vector<modelrange>::const_iterator
          +end() const: std::vector<modelrange>::const_iterator
          -modelranges_: std::vector<modelrange>
          -first_node_id_: size_t
          -last_node_id_: size_t
      }

      ModelRangeManager --|> ManagerInterface: extends

Explanation of Key Components:

    register_rng_type: A template method to register new RNG implementations (e.g., Mersenne Twister, Philox).
    rng_types_: A map storing registered RNG factories by name.
    rank_synced_rng_: A single RNG synchronized across all MPI ranks.
    vp_synced_rngs_: RNGs synchronized across threads (VPs) but consistent across ranks.
    vp_specific_rngs_: Independent RNGs for each thread (VP) and rank.
    Seeder Constants: Static values used to derive unique seeds for different RNG types.
    check_rng_synchrony: Ensures all RNGs are in sync (throws an error if not).

This structure ensures reproducible parallel simulations with thread-safe, synchronized random number generation.


.. doxygenclass:: nest::RandomManager
