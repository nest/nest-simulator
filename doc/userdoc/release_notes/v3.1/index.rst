
Changed and removed functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``GetDefaults()`` and ``SetDefaults()``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The functions :py:func`.GetDefaults` and :py:func`.SetDefaults` have
been extended to also work on the global properties of recording
backends. This new mechanism replaces backend property access via
nested dictionaries by a much simpler and more readable syntax:

+-------------------------------------+--------------------------------------+
| NEST 3.0                            | NEST 3.1                             |
+=====================================+======================================+
| nest.SetKernelStatus({              |                                      |
|     "recording_backends:" {         |                                      |
|         "sionlib": {                | params = {"sion_chunksize": 1024})   |
|             "sion_chunksize": 1024  | nest.SetDefaults('sionlib', params)  |
|         }                           |                                      |
|     }                               |                                      |
| })                                  |                                      |
+-------------------------------------+--------------------------------------+
| nest.GetKernelStatus(               | nest.GetDefaults('ascii')            |
|     "recording_backends")["ascii"]  |                                      |
+-------------------------------------+--------------------------------------+

Component dictionaries and ``Models()``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The function ``Models()`` has been removed from the ``nest`` Python
module. Where you previously would have used ``nest.Models("nodes")``
to acquire the list of available node models, you would now run
``nest.node_models`` instead. The list of available synapse models can
be retrieved using ``nest.synapse_models``.

+----------------------------------------+----------------------------------------------------------+
| NEST 3.0                               | NEST 3.1                                                 |
+========================================+==========================================================+
| nest.Models(mtype="nodes")             | nest.get('node_models')                                  |
+----------------------------------------+----------------------------------------------------------+
| nest.Models(mtype="synapses")          | nest.get('synapse_models')                               |
+----------------------------------------+----------------------------------------------------------+
| nest.Models(mtype="nodes", sel="iaf")  | list(filter(lambda x: "iaf" in x, nest.synapse_models))  |
+----------------------------------------+----------------------------------------------------------+

On the SLI level, the individual dictionaries ``connruledict``,
``growthcurvedict``, ``modeldict``, and ``synapsedict`` have been
removed. Their content is now consistently available as lists in the
kernel status dictionary under the keys ``connection_rules``,
``growth_curves``, ``node_models``, and ``synapse_models``. Moreover,
he list of available stimulation backends has been added under the key
``stimulation_backends``.
