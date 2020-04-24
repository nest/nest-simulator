

Sampling continuous quantities from neurons
###########################################

Most sampling use cases are covered by the ``multimeter``, which
allows to record analog values from neurons. Models which have such
values expose a ``recordables`` property that lists all recordable
quantities. This property can be inspected using ``GetDefaults`` on
the model class or ``GetStatus`` on a model instance. It cannot be
changed by the user.

::

   >>> nest.GetDefaults('iaf_cond_alpha')['recordables']
   ['g_ex', 'g_in', 't_ref_remaining', 'V_m']

The ``record_from`` property of a ``multimeter`` (a list, empty by
default) can be set to contain the name(s) of one or more of these
recordables to have them sampled during simulation.

::

   mm = nest.Create('multimeter', 1, {'record_from': ['V_m', 'g_ex']})

The sampling interval for recordings (given in ms) can be controlled
using the ``multimeter`` parameter `interval`. The default value of
1.0 ms can be changed by supplying a new value either in the call to
``Create`` or by using ``SetStatus`` on the model instance.

::

   nest.SetStatus(mm, 'interval': 0.1})

The recording interval must be greater than or equal to the
:doc:`simulation resolution <running_simulations>`, which defaults to
0.1 ms.

.. warning::

   The set of variables to record from and the recording interval must
   be set **before** the ``multimeter`` is connected to any neuron.
   These properties cannot be changed afterwards.

After configuration, a ``multimeter`` can be connected to the neurons
it should record from by using the standard ``Connect`` routine.

::

    neurons = nest.Create('iaf_psc_alpha', 100)
    nest.Connect(mm, neurons)

To learn more about possible connection patterns and additional
options when using ``Connect``, see the guide on :doc:`connection
management <connection_management>`.

The above call to ``Connect`` would fail if the neurons would not
support the sampling of the values *V_m* and *g_ex*. It would also
fail if carried out in the wrong direction, i.e., trying to connect the
*neurons* to *mm*.

.. note::

   A pre-configured  ``multimeter`` is available under the name ``voltmeter``.  Its
   ``record_from`` property is already set to record the variable ``V_m``
   from the neurons it is connected to.

