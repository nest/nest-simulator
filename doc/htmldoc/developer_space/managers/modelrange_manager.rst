.. _modelrange_manager:

ModelRange Manager
==================


The ModelRangeManager in NEST groups node IDs into contiguous ranges per model, enabling efficient model lookups via
binary search and minimizing storage by merging adjacent ranges. It tracks first/last node IDs and provides methods to
retrieve models for specific nodes, ensuring fast access during simulations.


.. doxygenclass:: nest::ModelRangeManager
   :members:
