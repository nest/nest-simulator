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
:ref:`list of updates for previous releases in the 3.x series
<whats_new>`.


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
| Up to NEST 3.5                                   | from NEST 3.6 on                                 |
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


Changes in NEST Server
~~~~~~~~~~~~~~~~~~~~~~

* Improve the security in NEST Server. The user can modify the security options in environment variables:
  * Requests require Bearer tokens. By default, the authentication is on (``NEST_SERVER_DISABLE_AUTH=0``).
  * NEST Server takes also custom token (``NEST_SERVER_ACCESS_TOKEN='alongaccesstoken'``), otherwise it generates token.
  * The CORS origins are restricted. By default, the only allowed CORS origin is ``http://localhost``
    (``NEST_SERVER_CORS_ORIGINS=http://localhost``).
  * Only API calls are enabled. By default, the exec call is disabled (``NEST_SERVER_ENABLE_EXEC_CALL=0``).
  * The code execution is restricted. By default, the restriction is activated (``NEST_SERVER_DISABLE_RESTRICTION=0``).
  * For security reasons the exec call in NEST Server accepts only modules from NEST_SERVER_MODULES.
    As example: ``NEST_SERVER_MODULES='import nest; import numpy as np; from numpy import random'``