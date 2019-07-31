Recording from simulations
==========================

*Recording devices* (or *recorders*, in short) are used to sample or
collect observable quantities like potentials, conductances or spikes
from neurons and synapses.

To determine what happens to recorded data, each recording device can
specify a *recording backend* in its ``record_to`` property. The default
backend is *memory*, which just stores the recorded data in memory for
later retrieval. Other backends write the data to file or screen, or
stream the data out to other applications.  The different backends and
their usage is explained in detail in the section about
:ref:`Recording Backends <recording_backends>`.

Recording devices can fundamentally be subdivided into two groups:

- **Collectors** collect events sent to them. Neurons are connected to
  collectors and the collector collects the events emitted by the
  neurons connected to it.

- **Samplers** actively interrogate their targets at given time
  intervals. This means that the sampler must be connected to the
  neuron (not the neuron to the sampler), and that the neuron must
  support the particular type of sampling.

Recording devices can only reliably record data that was generated
during the previous simulation time step interval. See the guide about
:doc:`running simulations <running_simulations>` for details about the
temporal aspects of the simulation loop.

.. note::
   Due to the need for internal buffering and the unpredictable order
   of thread execution, events are not necessarily recorded in
   chronological order.

Common recorder properties
--------------------------

All recorders two common properties that can be set using
``SetDefaults`` on the model class or ``SetStatus`` on a device
instance:

`record_to`
  A string (default: ``"memory"``) containing the name of the recording
  backend where to write data to. An empty string turns all recording
  of individual events off.

`label`
  A string (default: ``""``) specifying an arbitrary textual label for the
  device.  Recording backends might use the label to generate device
  specific identifiers like filenames and such.

Windowing
#########

Recording devices share the start, stop, and origin parameters global
to devices. Start and stop have the following meaning for stimulating
devices (origin is just a global offset):

  
- Collectors collect all events with timestamps T that fulfill

  ::

     start < T <= stop.
 
  Note that events with timestamp T == start are NOT recorded.
  
- Sampling devices sample at times t = nh with

  ::

     start < t <= stop
     (t-start) mod interval == 0

  
Sampling continuous quantities from neurons
-------------------------------------------

In most cases, the universal sampling device ``multimeter`` is used to
record analog values from neurons. Models providing such values expose
a ``recordables`` property that lists all recordable quantities.  This
internal property can be inspected using ``GetDefaults`` on the model
class or ``GetStatus`` on a model instance. It cannot be changed by
the user.

::

   In [0]: nest.GetDefaults('iaf_cond_alpha')['recordables']
   Out[0]: ['g_ex', 'g_in', 't_ref_remaining', 'V_m']

The ``record_from`` property of a ``multimeter`` (a list) can be set
to the name(s) of one or more of these recordables to have them
sampled during simulation.

::

   In[1]: mm = nest.Create('multimeter', 1, {'record_from': ['V_m', 'g_ex']})

The sampling interval for recordings (given in ms) can be controlled
using the ``multimeter``'s parameter ``interval``.  The default value
of 1 ms can be changed by supplying it either in the call to
``Create`` or by using ``SetStatus`` on the model instance.

::

   In[2]: nest.SetStatus(mm, 'interval': 0.1})

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

   In[3]: neurons = nest.Create("iaf_psc_alpha", 100)
   In[4]: nest.Connect(mm, neurons)

To learn more about possible connection patterns and other options
when using ``Connect``, see the guide on :doc:`connection management
<connection_management>`.

The abbove call to ``Connect`` would fail if the neurons would not
support the sampling of the values *V_m* and *g_ex*. It would also
fail if carried out in the wrong direction, i.e. trying to connect the
*neurons* to *mm*.

.. note::
   To ease the recording of the membrane potential, a pre-configured
   ``multimeter`` is available under the name ``voltmeter``.  Its
   ``record_from`` property is already set to record the variable
   ``V_m`` from the neurons it is connected to.

Collect event data from neurons and synapses
--------------------------------------------

Spike detector
##############

The most universal collector device is the ``spike_detector``. It
collects and records all *spikes* it receives from neurons that are
connected to it. Each spike received by the spike detector is
immediately handed over to the prescribed recording backend for
further processing.

Any node from which spikes are to be recorded, must be connected to
the spike detector using the standard ``Connect`` command. The
connection weight and delay are ignored by the spike detector.

::

   In[5]: sd = nest.Create("spike_detector")
   In[6]: nest.Connect(neurons, sd)

The call to ``Connect`` in the example above would fail, if the
*neurons* would not be sending ``SpikeEvent``s during a
simulation. Likewise, a reversed connection direction (i.e. connecting
*sd* to *neurons*) would fail.

.. note::
   The spike detector records spike times with full precision from
   neurons emitting :doc:`precisely timed spikes
   <simulations_with_precise_spike_time>`.

Correlation detector
####################

**TODO: include the model documentation here**

Weight recorder
###############

**TODO: include the model documentation here**

  
.. _recording_backends:

Where does data end up?
-----------------------

The way, data is processed after the recording device sampled or
collected it is the responsibility of the *recording backends*.

Theoretically, recording backends can do with the data whatever their
author wants them to do. The ones included in NEST can collect data in
memory, display it on the terminal, write it to file, or stream it out
to other applications.

.. _memory_backend:

Store data in main memory
#########################

The ``memory`` backend is the default for all recording devices as it
does not require a setup of data paths or permissions and allows to
read data out in a convenient fashion.

After one has simulated a little, the ``events`` entry of the
multimeter status dictionary will contain one numpy array of data for
each recordable.

The data is added to vectors, made available in a sub-dictionary of
the recorder's status dictionary called ``events``. It contains the
recorded data in the form of vectors. 


DATA LIFE SPAN


. The interpretation of the field `time` depends on the value of the
property `time_in_steps`. With the default setting (*false*), the
*times* field contains the simulation time in ms as a floating point



After one has simulated a little, the ``events`` entry of the
multimeter status dictionary will contain one numpy array of data for
each recordable.

   value. If it is set to *true*, the field *times* contains the time in integer mudepends
   on the setting of the property `time_in_steps`.

The data is added to vectors, made available in a sub-dictionary of
the recorder's status dictionary called ``events``. It contains the
recorded data in the form of vectors. 


. The interpretation of the field `time` depends on the value of the
property `time_in_steps`. With the default setting (*false*), the
*times* field contains the simulation time in ms as a floating point



 If set to *false* (which is the default), time is
written as one floating point number representing the simulation time
in ms. If `time_in_steps` is *true*, the time of the event is written
as a value pair consisting of the integer simulation time step and the
floating point offset in ms from the next grid point.

   value. If it is set to *true*, the field *times* contains the time in integer mudepends
   on the setting of the property `time_in_steps`.

Parameter summary
+++++++++++++++++

`events`
  is a dictionary containing the recorded data in the form of one
  numeric array for each quantity measured. It always has the sender
  global IDs of recorded events under the key *senders* and the time
  of the recording, the format of which depends on the setting of
  `time_in_steps`.

`n_events`
  is the number of events collected or sampled since the last reset of
  `n_events`. By setting `n_events` to 0, all spikes recorded so far
  will be discarded from memory.

`time_in_steps`
  is a Boolean (default: *false*) specifying whether to store time in
  steps, i.e. in integer multiples of the simulation resolution (under
  the key *times* of the *events* dictionary) plus a floating point
  number for the negative offset from the next grid point in ms (under
  key *offset*), or just the simulation time in ms under key *times*.

.. _ascii_backend:
  
Write data to plain text files
##############################

The `ascii` recording backend writes collected data to a plain text
ASCII file. It can be used for small to medium sized simulations,
where the ease of a simple data format outweights the benefits of
high-performance output operations.

This backend will open one file per recording device per thread on
each MPI process. This can entail a very high load on the file system
in large simulations. In case of scaling problems, the :ref:`SIONlib
backend <sionlib_backend>` can be a possible alternative.

Filenames are determined according to the following pattern:

::

   data_path/data_prefix(label|model_name)-gid-vp.file_extension

The properties `data_path` and `data_prefix` are global kernel
properties. They can for example be set during repetitive simulation
protocols to separate the data resulting from indivitual runs. The
`label` replaces the model name component if it is set to a non-empty
string. `gid` and `vp` correspond to the global ID and the virtual
process of the recorder writing the file. The filename ends in a dot
and the `file_extension`.

The life of a file starts with the call to ``Prepare`` and ends with
the call to ``Cleanup``. Data that is produced during successive calls
to ``Run`` inbetween one pair of ``Prepare`` and ``Cleanup`` calls
will be written to the same file.

In case, a file of the same name already exists, the ``Prepare`` call
will fail with a corresponding error message, unless the kernel
property[[link to SetKernelStatus]] `overwrite_files` is set to
*true*.

Data format
+++++++++++

The first line written to any new file is an informational header
containing field names for the different data columns. The header
starts with a `#` character.

The first field of each record written is the global id of the neuron
the event originated from, i.e. the *source* of the event. This is
followed by the time of the measurement, the recorded floating point
values and the recorded integer values.

The format of the time field depends on the value of the property
`time_in_steps`. If set to *false* (which is the default), time is
written as one floating point number representing the simulation time
in ms. If `time_in_steps` is *true*, the time of the event is written
as a value pair consisting of the integer simulation time step and the
floating point offset in ms from the next grid point.

.. note::

   The number of decimal places for all decimal numbers in the output
   can be controlled using the recorder property `precision`.

Parameter summary
+++++++++++++++++

`file_extension`
  specifies the file name extension, without leading dot. As the exact
  type of data cannot be known a priori, the default extension is
  simply *.dat*.

`filenames`
  contains the filenames where data is recorded to. This list has one
  entry per local thread. This is a read-only property.

`label`
  replaces the model name component in the filename if it is set to a
  non-empty string.

`precision`
  controls the number of decimal places used to write decimal numbers
  to the output file.

`time_in_steps`
  A boolean (default: false) specifying whether to write time in
  steps, i.e. in integer multiples of the resolution and an offset,
  rather than just in ms.

.. _screen_backend:
  
Write data to the terminal
##########################

When initially conceiving and debugging simulations, it can be useful
to check recordings in a more ad hoc fashion. The recording backend
`screen` can be used to dump all recorded data onto the console for
quick inspection.

The first field of each record written is the global id of the neuron
the event originated from, i.e. the *source* of the event. This is
followed by the time of the measurement, the recorded floating point
values and the recorded integer values.

The format of the time field depends on the value of the property
`time_in_steps`. If set to *false* (which is the default), time is
written as one floating point number representing the simulation time
in ms. If `time_in_steps` is *true*, the time of the event is written
as a value pair consisting of the integer simulation time step and the
floating point offset in ms from the next grid point.

.. note::
   
   Using this backend for production runs is not recommended, as it
   may produce *huge* amounts of console output and thereby might slow
   down the simulation *considerably*.

Parameter summary
+++++++++++++++++

`precision`
  controls the number of decimal places used to write decimal numbers
  to the terminal.

`time_in_steps`
  A boolean (default: false) specifying whether to print time in
  steps, i.e. in integer multiples of the resolution and an offset,
  rather than just in ms.

.. _sionlib_backend:
  
Store data to an efficient binary format
########################################

 (`sionlib`)

.. _arbor_backend:
 
Stream data to an arbor instance:
#################################

 (`arbor`)


 
Writing own recording backends
------------------------------
