.. _connection_manager:

Connection Manager
==================


The ConnectionManager in NEST efficiently manages synaptic connections between neurons and devices using source-table
structures, supporting dynamic network modifications via structural plasticity and connection rules. It tracks
connection counts, handles parallel execution resizing, and ensures disabled connections are removed to maintain network
integrity.

.. doxygenclass:: nest::ConnectionManager
   :members:
