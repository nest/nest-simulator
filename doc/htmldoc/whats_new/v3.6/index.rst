.. _release_3.6:

What's new in NEST 3.6
======================

This page contains a summary of important breaking and non-breaking
changes from NEST 3.5 to NEST 3.6. In addition to the `release notes
on GitHub <https://github.com/nest/nest-simulator/releases/>`_, this
page also contains transition information that helps you to update
your simulation scripts when you come from an older version of NEST.

If you transition from an earlier version, please see our extensive
:ref:`transition guide from NEST 2.x to 3.0 <refguide_2_3>` and the
:ref:`list of updates for previous releases in the 3.x series <whats_new>`.

Astrocytes in NEST
------------------

Astrocytes, one of the main non-neuronal cell types in the brain,
interact with neurons through versatile cellular mechanisms and modulate neuronal
activity in a complex and not fully understood way.
We developed new NEST models to bring astrocytes and
neuron-astrocyte interactions to spiking neural networks into NEST.
Our new models support reproducible and collaborative large-scale modeling of
neuron-astrocyte circuits.

See examples using astrocyte models:

* :doc:`../../../auto_examples/astrocytes/astrocyte_single`
* :doc:`../../../auto_examples/astrocytes/astrocyte_interaction`

See model docs:

* :doc:`../../../models/index_astrocyte`

New model: glif_psc_double_alpha
--------------------------------

This model is based on the ``glif_psc`` model, but
uses the sum of two alpha functions instead of a single
alpha function as the post synaptic current input.

See example:

*  :doc:`../../../auto_examples/glif_psc_double_alpha_neuron`

See model docs:

*  :doc:`../../../models/glif_psc_double_alpha`

New way to set the volume transmitter on STDP dopamine synapse
--------------------------------------------------------------

Previously, the :doc:`volume transmitter <../../../../models/volume_transmitter>`
for the :doc:`STDP dopamine synapse <../../../../models/stdp_dopamine_synapse>` was
set by supplying it with the "naked" node ID of the volume transmitter using its
property `vt`. As it was rather inconvenient to obtain this ID and the procedure was
inconsistent with how nodes are usually passed around in NEST, this is now no longer
possible. Instead, the volume transmitter is now set by supplying a NodeCollection to
the property `volume_transmitter` of the synapse's common properties:

+--------------------------------------------------+--------------------------------------------------+
| Up to NEST 3.5                                   | from NEST 3.6 onward                             |
+==================================================+==================================================+
|  ::                                              |  ::                                              |
|                                                  |                                                  |
|     vt = nest.Create("volume_tranmitter")        |     vt = nest.Create("volume_tranmitter")        |
|     nest.SetDefaults(                            |     nest.SetDefaults(                            |
|         "stdp_dopamine_synapse",                 |         "stdp_dopamine_synapse",                 |
|         {"vt": vol.get("global_id")}             |          {"volume_transmitter": vt}              |
|     )                                            |     )                                            |
|                                                  |                                                  |
+--------------------------------------------------+--------------------------------------------------+

Changes to kernel attributes
----------------------------

The following kernel attributes were removed:

* ``sort_connections_by_source`` : Use ``use_compressed_spikes`` instead; it automatically activates connection sorting
* ``adaptive_spike_buffers`` — spike buffers are now always adaptive
* ``max_buffer_size_spike_data`` — there is no upper limit since all spikes need to be transmitted in one round

New kernel attributes that control or report spike buffer resizing:

*  ``spike_buffer_grow_extra``
*  ``spike_buffer_shrink_limit``
*  ``spike_buffer_shrink_spare``
*  ``spike_buffer_resize_log``

For details, see our :ref:`docs on the new attributes <sec_kernel_attributes>`.

Changes in NEST Server
----------------------

We improved the security in NEST Server. Now to use NEST Server, users can modify the security options.
See :ref:`section on setting these varialbles <sec_server_vars>` in our NEST Server guide.
