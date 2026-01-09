.. _event_delivery_manager:

Event Delivery Manager
======================

The EventDeliveryManager coordinates the delivery of events (e.g., spikes, data) between nodes in NEST simulations,
managing buffers and routing events across MPI processes for distributed computing.
It handles timing through moduli-based time slicing, optimizes communication via send/receive buffers, and ensures
accurate event delivery even for off-grid spikes and secondary events like data updates.
This manager is essential for maintaining simulation accuracy and scalability, particularly in large-scale parallel
simulations with complex event routing requirements.

.. doxygenclass:: nest::EventDeliveryManager
    :members:
