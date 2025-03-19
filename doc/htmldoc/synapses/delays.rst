.. _delays:

Delays
======


Previously, transmission delays could only be specified with the ``delay`` parameter.

In NEST, delays are considered fully dendritic by all built-in models and therefore,
the ``delay`` parameter is still used by most models.

With the release of NEST 3.9, we now can specify delays as being axonal or dendritic with the following neuron and
synapse model.


Currently, only ``iaf_psc_alpha`` supports STDP with the ``axonal_delay`` and ``dendtritic_delay`` parameters,
and only ``stdp_pl_synapse_hom_ax_delay`` implements STDP with axonal delays.

When using ``stdp_pl_synapse_hom_ax_delay``:

- The parameter ``delay`` is no longer valid. This is to prevent ambiguity between the two types of delays.
- The parameter names ``dendritic_delay`` and ``axonal_delay`` have to be used to specify delay.
- If these parameters are not explicitly provided, then the default values are used:

  ``dendritic_delay: 1.0``  and ``axonal_delay: 0.0``.
- If only axonal delay is provided and no dendritic delay, the dendritic delay is assumed to be 0 and vice-versa.


Use of ``axonal_delay`` and ``dendritic_delay`` is the same as ``delay``:


**Using syn_spec**

.. code-block:: python

   nest.Create("iaf_psc_alpha")
   nest.Connect(neuron, neuron, syn_spec=
               {"synapse_model": "stdp_pl_synapse_hom", "delay": 1.0})

.. code-block:: python

   nest.Create("iaf_psc_alpha")
   nest.Connect(neuron, neuron, syn_spec=
               {"synapse_model": "stdp_pl_synapse_hom_ax_delay", "axonal_delay": 1.0})

**Using SetStatus**

.. code-block:: python

   conn = nest.Connect(neuron, neuron, syn_spec=
                      {"synapse_model": "stdp_pl_synapse_hom"})
   nest.SetStatus(conn, {"delay": 1.0})

.. code-block:: python

      conn = nest.Connect(neuron, neuron, syn_spec=
                         {"synapse_model": "stdp_pl_synapse_hom_ax_delay"})
      nest.SetStatus(conn, {"axonal_delay": 1.0, "dendritic_delay": 1.0})

**Using SetDefaults**

.. code-block:: python

   nest.SetDefaults("stdp_pl_synapse_hom", {"delay": 1.0})

.. code-block:: python

   nest.SetDefaults("stdp_pl_synapse_hom_ax_delay", {"axonal_delay": 1.0})


.. seealso::

   :doc:`Example using axonal delays </auto_examples/axonal_delays>`

   For details on further developments see :ref:`axonal_delays_dev`.
