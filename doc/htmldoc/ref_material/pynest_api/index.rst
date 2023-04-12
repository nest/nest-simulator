.. _pynest_api:

PyNEST API
==========

The ``nest`` module contains methods and attributes to control the NEST kernel.
This interface is known as PyNEST. You can browse the various functions to use in your NEST script.





:doc:`Functions related to nodes <nest.lib.hl_api_nodes>`
----------------------------------------------------------


.. automodule::   nest.lib.hl_api_nodes


   .. autosummary::

      Create
      GetLocalNodeCollection
      GetNodes
      PrintNodes


:doc:`Functions related to models <nest.lib.hl_api_models>`
-----------------------------------------------------------


.. automodule::   nest.lib.hl_api_models


   .. autosummary::

      ConnectionRules
      CopyModel
      GetDefaults
      Models
      SetDefaults


:doc:`Functions related to parameters for nodes and synapses  <nest.lib.hl_api_types>`
---------------------------------------------------------------------------------------


.. automodule::   nest.lib.hl_api_types


   .. autosummary::

      CreateParameter
      serializable
      to_json


   .. autosummary::

      CollocatedSynapses
      Compartments
      Mask
      NodeCollection
      Parameter
      Receptors
      SynapseCollection

.. automodule:: nest.lib.hl_api_info

   .. autosummary::

      get_argv
      GetStatus
      SetStatus

:doc:`Functions related to connections <nest.lib.hl_api_connections>`
---------------------------------------------------------------------


.. automodule::   nest.lib.hl_api_connections


   .. autosummary::

      Connect
      Disconnect
      GetConnections

:doc:`Functions related to simulations  <nest.lib.hl_api_simulation>`
---------------------------------------------------------------------


.. automodule::   nest.lib.hl_api_simulation


   .. autosummary::

      Cleanup
      DisableStructuralPlasticity
      EnableStructuralPlasticity
      GetKernelStatus
      Install
      Prepare
      ResetKernel
      Run
      RunManager
      SetKernelStatus
      Simulate


:doc:`Functions related to spatial networks  <nest.lib.hl_api_spatial>`
------------------------------------------------------------------------

.. automodule::   nest.lib.hl_api_spatial


   .. autosummary::

      CreateMask
      Displacement
      Distance
      DumpLayerConnections
      DumpLayerNodes
      FindCenterElement
      FindNearestElement
      GetPosition
      GetSourceNodes
      GetSourcePositions
      GetTargetNodes
      GetTargetPositions
      PlotLayer
      PlotProbabilityParameter
      PlotSources
      PlotTargets
      SelectNodesByMask

.. automodule:: nest.spatial_distributions.hl_api_spatial_distributions


   .. autosummary::

      exponential
      gaussian
      gaussian2D
      gamma

.. automodule:: nest.spatial.hl_api_spatial


   .. autosummary::

      grid
      free
      pos
      source_pos
      target_pos

:doc:`Functions related to help and info  <nest.lib.hl_api_helper>`
-------------------------------------------------------------------

.. automodule::   nest.lib.hl_api_helper


   .. autosummary::

      broadcast
      deprecated
      get_parameters
      get_parameters_hierarchical_addressing
      get_unistring_type
      get_wrapped_text
      is_coercible_to_sli_array
      is_iterable
      is_literal
      is_sequence_of_connections
      is_sequence_of_node_ids
      is_string
      load_help
      model_deprecation_warning
      restructure_data
      show_deprecation_warning
      show_help_with_pager


   .. autosummary::

      SuppressedDeprecationWarning

.. automodule:: nest.lib.hl_api_info

   .. autosummary::

      authors
      help
      helpdesk
      message
      get_verbosity
      set_verbosity
      sysinfo

:doc:`Functions related to math and logic  <nest.math.hl_api_math>`
-------------------------------------------------------------------

.. automodule::   nest.math.hl_api_math


   .. autosummary::

      exp
      sin
      cos
      min
      max
      redraw

.. automodule::   nest.logic.hl_api_logic


   .. autosummary::

      conditional

:doc:`Functions related to randomizaton  <nest.random.hl_api_random>`
---------------------------------------------------------------------

.. automodule::   nest.random.hl_api_random


   .. autosummary::

      exponential
      lognormal
      normal
      uniform
      uniform_int

:doc:`Functions related to parallel computing  <nest.lib.hl_api_parallel_computing>`
------------------------------------------------------------------------------------

.. automodule::   nest.lib.hl_api_parallel_computing


   .. autosummary::

      GetLocalVPs
      NumProcesses
      Rank
      SetAcceptableLatency
      SetMaxBuffered
      SyncProcesses

:doc:`Functions related NEST server  <nest.server.hl_api_server>`
-----------------------------------------------------------------

.. automodule::   nest.server.hl_api_server


   .. autosummary::

      app
      do_exec
      set_mpi_comm
      run_mpi_app
      nestify

:doc:`Functions related to voltage trace  <nest.voltage_trace>`
----------------------------------------------------------------

.. automodule::   nest.voltage_trace


   .. autosummary::

      from_device
      from_file

:doc:`Functions related to raster plots <nest.raster_plot>`
------------------------------------------------------------

.. automodule::   nest.raster_plot


   .. autosummary::

      extract_events
      from_data
      from_device
      from_file
      from_file_numpy
      from_file_pandas

:doc:`Functions related to plotting networks  <nest.visualization>`
-------------------------------------------------------------------

.. automodule::   nest.visualization


   .. autosummary::

      plot_network

:doc:`Functions related kernel attributes  <kernel_attributes>`
---------------------------------------------------------------

.. autoclass:: nest.NestModule

     .. autosummary::

        adaptive_spike_buffers
        adaptive_target_buffers
        biological_time
        buffer_size_spike_data
        buffer_size_target_data
        connection_rules
        data_path
        data_prefix
        dict_miss_is_error
        growth_curves
        growth_factor_buffer_spike_data
        growth_factor_buffer_target_data
        keep_source_table
        kernel_status
        local_num_threads
        local_spike_counter
        max_buffer_size_spike_data
        max_buffer_size_target_data
        max_delay
        min_delay
        max_num_syn_models
        min_update_time
        ms_per_tic
        network_size
        node_models
        num_connections
        num_processes
        off_grid_spiking
        print_time
        recv_buffer_size_secondary_events
        recording_backends
        resolution
        rng_seed
        rng_type
        rng_types
        send_buffer_size_secondary_events
        sort_connections_by_source
        stimulation_backends
        structural_plasticity_synapses
        structural_plasticity_update_interval
        synapse_models
        T_max
        T_min
        tics_per_ms
        tics_per_step
        to_do
        total_num_virtual_procs
        update_time_limit
        use_compressed_spikes
        use_wfr
        wfr_comm_interval
        wfr_interpolation_order
        wfr_max_iterations
        wfr_tol



.. toctree::
   :hidden:
   :glob:

   *
