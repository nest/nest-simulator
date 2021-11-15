All about NEST 3.2
==================

This page contains a summary of all breaking and non-breaking changes
from NEST 3.1 to NEST 3.2. In addition to the `auto-generated release
notes on GitHub <https://github.com/nest/nest-simulator/releases/>`_,
this page also contains transition information that helps you to
update your simulation scripts when you come from an older version of
NEST.

If you transition from a version earlier than 3.1, please see our
selection of earlier :doc:`transition guides <release_notes/index>`.

.. contents:: On this page you'll find
   :local:
   :depth: 1


Retrieve available node and synapse models
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The PyNEST function ``Models()`` is now deprecated and will be removed
in a future version of NEST. Where you previously used the function
``nest.Models("nodes")`` to acquire the list of available node models,
you would now write ``nest.node_models`` instead. The list of
available synapse models can be retrieved using the kernel attribute
``nest.synapse_models``. Filtering can easily and explicitly be
implemented using a conditional expression in a list comprehension.

+--------------------------------------------+--------------------------------------------------+
| NEST 3.1                                   | NEST 3.2                                         |
+============================================+==================================================+
| ``nest.Models(mtype="nodes")``             | ``nest.node_models``                             |
+-----------....-----------------------------+--------------------------------------------------+
| ``nest.Models(mtype="synapses")``          | ``nest.synapse_models``                          |
+--------------------------------------------+--------------------------------------------------+
| ``nest.Models(mtype="nodes", sel="iaf")``  | ``[m for m in nest.node_models if "iaf" in m]``  |
+--------------------------------------------+--------------------------------------------------+

New kernel attributes
^^^^^^^^^^^^^^^^^^^^^

On the SLI level, the individual dictionaries ``connruledict``,
``growthcurvedict``, ``modeldict``, and ``synapsedict`` have been
removed. Their content is now consistently available as kernel
attributes with the names ``connection_rules``, ``growth_curves``,
``node_models``, and ``synapse_models``. Moreover, the list of
available stimulation backends has been added under the attribute
``stimulation_backends``.

Global properties for recording backends
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The functions :py:func`.GetDefaults` and :py:func`.SetDefaults` have
been extended to also work on the global properties of recording
backends. This new mechanism replaces backend property access via
nested dictionaries and leads to simpler and more readable code:

+----------------------------------------+------------------------------------------+
| NEST 3.1                               | NEST 3.2                                 |
+========================================+==========================================+
|  ::                                    |  ::                                      |
|                                        |                                          |
|     params = {"sion_chunksize": 1024}  |     params = {"sion_chunksize": 1024}    |
|     nest.recording_backends = {        |     nest.SetDefaults("sionlib", params)  |
|         "sionlib": params              |                                          |
|     }                                  |                                          |
|                                        |                                          |
+----------------------------------------+------------------------------------------+
|  ::                                    |  ::                                      |
|                                        |                                          |
|     nest.recording_backends["ascii"]   |     nest.GetDefaults("ascii")            |
|                                        |                                          |
+----------------------------------------+------------------------------------------+
