.. _device_index:

All about devices in NEST
=========================


What kind of values can be recorded?
------------------------------------

This depends on neuron or synapse model specified.

You can get a list of properties that you can record using the ``recordables`` property.

        ::

         >>> nest.GetDefaults('iaf_cond_alpha')['recordables']
         ['g_ex', 'g_in', 't_ref_remaining', 'V_m']


- multimeter: record analog values from neurons.

- spike recorder: collects and records all spikes it receives from neurons that are connected to it

- weight recorder: Recording weights from synapses

How can I get data out?
-----------------------

We have several ways you can access the data you recorded, these are referred to as ``recording_backends``

- output to Ascii file
- output to memory
- output to screen
- output to SionLib
- output to MPI


You can specify the recording backend using ``record_to`` parameter

Each recording backend has their own parameters

By default the data will record to "memory"
Stimulating devices


Get spike data out, membrane potential, synaptic conductances, synaptic weights



Recording devices

- What kind of devices can be used to record data

- What kinds of values can be recorded?

- How do I get data out


  ::

      nest.Create("spike_recorder", 1, {"record_to": "ascii", "time_in_steps": True}
      sr =nest.Create("spike_recorder", 1, {"record_to": "memory", "time_in_steps": True}


data recorded by recording backend ascii (time_in_steps=True)
[['sender' 'time_step' 'time_offset']
 ['1' '21' '0.000']
 ['1' '44' '0.000']
 ['1' '67' '0.000']
 ['1' '89' '0.000']
 ['1' '111' '0.000']
 ['1' '133' '0.000']
 ['1' '155' '0.000']
 ['1' '177' '0.000']
 ['1' '199' '0.000']
 ['1' '221' '0.000']
 ['1' '243' '0.000']
 ['1' '265' '0.000']
 ['1' '287' '0.000']]

simulation resolution in ms: 0.1
data recorded by recording backend ascii (time_in_steps=False)
[['sender' 'time_ms']
 ['1' '2.100']
 ['1' '4.400']
 ['1' '6.700']
 ['1' '8.900']
 ['1' '11.100']
 ['1' '13.300']
 ['1' '15.500']
 ['1' '17.700']
 ['1' '19.900']
 ['1' '22.100']
 ['1' '24.300']
 ['1' '26.500']
 ['1' '28.700']]
simulation resolution in ms: 0.1

data recorded by recording backend memory (time_in_steps=False)
{'senders': array([1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]),
'times': array([ 2.1,  4.4,  6.7,  8.9, 11.1, 13.3, 15.5, 17.7, 19.9, 22.1, 24.3, 26.5, 28.7])}
data recorded by recording backend memory (time_in_steps=True)
{'offsets': array([0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.]),
'senders': array([1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]),
'times': array([ 21,  44,  67,  89, 111, 133, 155, 177, 199, 221, 243, 265, 287])}

See examples

https://nest-simulator.readthedocs.io/en/latest/auto_examples/recording_demo.html
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
