.. _device_index:

All about devices in NEST
=========================



Stimulating devices


Recording devices

- What kind of devices can be used to record data

  - multimeter: record analog values from neurons.

      - What kinds of values can be recorded?

        This depends on the neuron model specified. You can get a list of properties that you can
        record using the ``recordables`` property.

        ::

         >>> nest.GetDefaults('iaf_cond_alpha')['recordables']
         ['g_ex', 'g_in', 't_ref_remaining', 'V_m']

  - spike recorder: collects and records all spikes it receives from neurons that are connected to it

  - weight recorder: Recording weights from synapses


- How do I get data out

  Recorded data is handed over to recording backend


  You can specify the recording backend using ``record_to`` parameter
  Each recording backend has their own parameters

See examples
https://nest-simulator.readthedocs.io/en/latest/auto_examples/recording_demo.html
  https://nest-simulator.readthedocs.io/en/latest/auto_examples/multimeter_file.html
Basic
.. code-block::

  >>> nest.GetDefaults('iaf_cond_alpha')['recordables']
  ['g_ex', 'g_in', 't_ref_remaining', 'V_m']

  The record_from property of a multimeter (a list, empty by default) can be set to contain the name(s) of one or more of these recordables to have them sampled during simulation.

  mm = nest.Create('multimeter', 1, {'record_from': ['V_m', 'g_ex']})

``sr = nest.Create('spike_recorder', params={'record_to': 'ascii'})``

``record_to``

/bin/bash: line 1: :w: command not found
- Device properties

.. toctree::
  :maxdepth: 1
  :glob:

  *
