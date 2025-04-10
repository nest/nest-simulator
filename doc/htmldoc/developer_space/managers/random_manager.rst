.. _random_manager:

Random Manager
==============

The RandomManager in NEST provides three RNG types (rank-synchronized, VP-synchronized, VP-specific) to manage parallel
random number generation, ensuring reproducibility and thread safety. It supports various algorithms (e.g., Mersenne
Twister, Philox) and allows configuration of seeds and types, initializing RNGs with unique seeds derived from base
values and seeder constants for consistent yet independent streams across processes and threads.

.. doxygenclass:: nest::RandomManager
   :members:
