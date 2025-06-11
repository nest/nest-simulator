.. _node_manager:

Node Manager
============

The NodeManager in NEST manages node creation and distribution across computational threads, ensuring parallel execution
efficiency. It tracks node IDs, handles special node types like MUSIC interfaces, and detects network changes to update
states, while storing exceptions from parallel operations for error resolution.


.. doxygenclass:: nest::NodeManager
   :members:
