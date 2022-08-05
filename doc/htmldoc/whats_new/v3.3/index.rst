.. _release_3.3:

What's new in NEST 3.3
=======================

This page contains a summary of important breaking and non-breaking changes
from NEST 3.2 to NEST 3.3. In addition to the `release
notes on GitHub <https://github.com/nest/nest-simulator/releases/>`_,
this page also contains transition information that helps you to
update your simulation scripts when you come from an older version of
NEST.

If you transition from a version earlier than 3.2, please see our
extensive :ref:`transition guide from NEST 2.x to 3.0 <refguide_2_3>` 
or :ref:`release updates for previous releases in 3.x<whats_new>`.


Model defaults
~~~~~~~~~~~~~~

The model parameter ``delta_tau`` in the ``correlation_detector``,
``correlomatrix_detector``, and ``correlospinmatrix_detector``, as
well as ``dt`` in the ``noise_generator`` are now automatically
adjusted and made compatible with a newly set simulation resolution to
avoid errors when those models are instantiated. Moreover, the default
value for ``delta_tau`` in the ``correlation_detector`` has been
changed from 1.0 ms to 5 times the simulation resolution, in order to
be consistent with the documentation of the device.

Retrieve available node and synapse models
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The PyNEST function ``Models()`` is now deprecated and will be removed
in a future version of NEST. Where you previously used the function
``nest.Models("nodes")`` to acquire the list of available node models,
you would now write ``nest.node_models`` instead. The list of
available synapse models can be retrieved using the kernel attribute
``nest.synapse_models``. Filtering can easily and explicitly be
implemented using a conditional expression in a list comprehension.

+--------------------------------------------+--------------------------------------------------+
| NEST 3.2                                   | NEST 3.3                                         |
+============================================+==================================================+
| ``nest.Models(mtype="nodes")``             | ``nest.node_models``                             |
+--------------------------------------------+--------------------------------------------------+
| ``nest.Models(mtype="synapses")``          | ``nest.synapse_models``                          |
+--------------------------------------------+--------------------------------------------------+
| ``nest.Models(mtype="nodes", sel="iaf")``  | ``[m for m in nest.node_models if "iaf" in m]``  |
+--------------------------------------------+--------------------------------------------------+

New kernel attributes
~~~~~~~~~~~~~~~~~~~~~

On the SLI level, the individual dictionaries ``connruledict``,
``growthcurvedict``, ``modeldict``, and ``synapsedict`` have been
removed. Their content is now consistently available as kernel
attributes with the names ``connection_rules``, ``growth_curves``,
``node_models``, and ``synapse_models``. Moreover, the list of
available stimulation backends has been added under the attribute
``stimulation_backends``.

In the course of adding the new kernel attributes, the functions
``Models()`` and ``ConnectionRules()`` of PyNEST have been marked as
deprecated and will be removed in a later version.

Global properties for recording backends
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The functions :py:func`.GetDefaults` and :py:func`.SetDefaults` have
been extended to also work on the global properties of recording
backends. This new mechanism replaces backend property access via
nested dictionaries and leads to simpler and more readable code:

+----------------------------------------+------------------------------------------+
| NEST 3.2                               | NEST 3.3                                 |
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

Compartmental models
~~~~~~~~~~~~~~~~~~~~

A compartmental modelling framework has been added. The layout of the
model is user-configurable at runtime, and can be adapted to represent any
dendritic and/or axonal structure. By default, there are two ion channels, one
Na-channel and one K-channel, and four receptor types (AMPA, GABA, NMDA and
AMPA+NMDA).
