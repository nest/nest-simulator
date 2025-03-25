.. _event_delivery_manager:

Event Delivery Manager
======================

The EventDeliveryManager coordinates the delivery of events (e.g., spikes, data) between nodes in NEST simulations,
managing buffers and routing events across MPI processes for distributed computing.
It handles timing through moduli-based time slicing, optimizes communication via send/receive buffers, and ensures
accurate event delivery even for off-grid spikes and secondary events like data updates.
This manager is essential for maintaining simulation accuracy and scalability, particularly in large-scale parallel
simulations with complex event routing requirements.

.. mermaid::

  classDiagram
    class EventDeliveryManager {
      +bool off_grid_spiking_
      +std::vector<long> moduli_
      +std::vector<SpikeData> send_buffer_spike_data_
      +std::vector<SpikeData> recv_buffer_spike_data_
      +std::vector<int> send_buffer_secondary_events_
      +void deliver_events(size_t tid)
      +void gather_spike_data_()
      +void configure_spike_data_buffers()
      +bool get_off_grid_communication()
      +void set_off_grid_communication(bool)
      }
    class ManagerInterface {
      // Base class, can leave empty if no details needed
      }
    EventDeliveryManager --|> ManagerInterface


.. mermaid::

  classDiagram
      class EventDeliveryManager {
          +initialize(bool adjust_number_of_threads_or_rng_only)
          +finalize(bool)
          +updateValue(dict, names::off_grid_spiking, bool off_grid_spiking_)
          -emitted_spikes_register_: std::vector<SpikeDataWithRank>*
          -off_grid_emitted_spikes_register_: std::vector<OffGridSpikeDataWithRank>*
          -send_recv_buffer_shrink_limit_: double
      }

      EventDeliveryManager --> SpikeDataWithRank
      EventDeliveryManager --> OffGridSpikeDataWithRank


.. doxygenclass:: nest::EventDeliveryManager
    :members:
