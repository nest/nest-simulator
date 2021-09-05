All about NEST 3.1
==================

This page contains a summary of all breaking and non-breaking changes
from NEST 3.0 to NEST 3.1. In addition to the `auto-generated release
notes on GitHub <https://github.com/nest/nest-simulator/releases/>`_,
this page also contains transition information that helps you to
update your simulation scripts when you come from an older version of
NEST.

If you transition from a version earlier than 3.0, please see our
extensive :doc:`transition guide from NEST 2.x to 3.0
<nest2_to_nest3/index>`.

.. contents:: On this page you'll find
   :local:
   :depth: 1


Access to kernel properties
~~~~~~~~~~~~~~~~~~~~~~~~~~~

NEST 3.1 provides a new interface to the properties of the NEST
simulation kernel. Instead of using the traditional access methods
:py:func:`.GetKernelStatus` and :py:func:`.SetKernelStatus`,
properties can now also be set and retrieved via *direct kernel
attributes*.

Where you previously had ``nest.SetKernelStatus({"resolution": 0.2})``
in your simulation script, you can now just write ``nest.resolution =
0.5``. Kernel attributes now come with their own docstrings and even
tab-completion works for them!

Co-dependent properties that have to be set together (for instance
``min_delay`` and ``max_delay``) can now be changed using the function
:py:func:`.set` on the level of the ``nest`` module.

  +-------------------------------------------------+---------------------------------------------+
  | NEST 3.0                                        | NEST 3.1                                    |
  +=================================================+=============================================+
  | ``nest.GetKernelStatus()``                      | ``nest.kernel_status``                      |
  +-------------------------------------------------+---------------------------------------------+
  | ``nest.GetKernelStatus('network_size')``        | ``nest.network_size``                       |
  +-------------------------------------------------+---------------------------------------------+
  | ``nest.SetKernelStatus({'resolution': 0.2})``   | ``nest.resolution = 0.2``                   |
  +-------------------------------------------------+---------------------------------------------+
  |  ::                                             |                                             |
  |                                                 |  ::                                         |
  |     nest.SetKernelStatus({                      |                                             |
  |         'min_delay': 0.2,                       |     nest.set(min_delay=0.2, max_delay=2.0)  |
  |         'max_delay': 2.0                        |                                             |
  |     })                                          |                                             |
  +-------------------------------------------------+---------------------------------------------+

.. admonition:: Deprecation info

      The use of the access functions :py:func:`.GetKernelStatus` and
      :py:func:`.SetKernelStatus` is now deprecated, and they will be
      removed in a future version of NEST. To avoid porting trouble
      later on, we suggest you switch to using the new shortcuts
      now.

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
| nest.Models(mtype="nodes")             | nest.node_models                                         |
+----------------------------------------+----------------------------------------------------------+
| nest.Models(mtype="synapses")          | nest.synapse_models                                      |
+----------------------------------------+----------------------------------------------------------+
| nest.Models(mtype="nodes", sel="iaf")  | list(filter(lambda x: "iaf" in x, nest.synapse_models))  |
+----------------------------------------+----------------------------------------------------------+

On the SLI level, the individual dictionaries ``connruledict``,
``growthcurvedict``, ``modeldict``, and ``synapsedict`` have been
removed. Their content is now consistently available as kernel
attributes with the names ``connection_rules``, ``growth_curves``,
``node_models``, and ``synapse_models``. Moreover, the list of
available stimulation backends has been added under the attribute
``stimulation_backends``.
