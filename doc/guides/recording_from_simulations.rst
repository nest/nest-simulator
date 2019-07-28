Recording from simulations
==========================

*Recording devices* (or *recorders*, in short) are used to obtain and
record different observable quantities from neurons or synapses.

The recorded data is stored by *recording backend*s. Each recording
device can specify a number of recording backends to be used. The
default is to store data in memory by using the `memory` backend. The
different backends and their usage is explained in detail below
[[Anchor: Recording Backends]].

Recording devices can fundamentally be subdivided into two groups:

- **Collector devices** collect events sent to them; the archetypical
  example is the ``spike_detector``. Nodes are connected to collectors
  and the collector collects the events emitted by the nodes connected
  to it.

- **Sampling devices** actively interrogate their targets at given
  time intervals and record the data they obtain. This means that the
  sampler must be connected to the node (not the node to the sampler),
  and that the node must support the particular type of sampling.

Recording devices can only reliably record data that was generated
during the previous min_delay interval [[Link to Running
Simulations]].

.. note:

   Due to the need for internal buffering and the unpredictable order
   of thread execution, events are not necessarily recorded in
   chronological order.

Common recorder properties
--------------------------

All recorders share a set of common properties that can be set using
``SetDefaults`` on the model class or ``SetStatus`` on a device
instance:

`record_to`
  A list (default: ["memory"]) containing any combination of names of
  recording backends, indicating where to write data to. An empty
  array turns all recording of individual events off.

`label`
  A string (default: "") specifying an arbitrary textual label for the
  device.  Recording backends might use the label to generate device
  specific identifiers like filenames and such.

Sampling continuous quantities from neurons
-------------------------------------------

The universal sampling device ``multimeter`` is used available to
record analog values from neurons. Models providing such values expose
a `recordables` property that lists all recordable quantities.  This
internal property can be inspected using ``GetDefaults`` on the model
class or ``GetStatus`` on a model instance. It cannot be changed by
the user.

   ::

       In [0]: nest.GetDefaults('iaf_cond_alpha')['recordables']
       Out[0]: ['V_m', 'g_ex', 'g_in', 't_ref_remaining']

The `record_from` property of a ``multimeter`` can be set to one or
more recordable names to have these quantities sampled during
simulation.

   ::

       In[1]: mm = nest.Create('multimeter', params={'record_from': ['V_m', 'g_ex']})

The sampling interval of recordings in ms can be controlled using the
``multimeter``'s parameter `interval`.  The default value of 1 ms can
be changed by supplying it in either in the call to ``Create`` or by
using ``SetStatus`` on the model instance.

   ::

       In[2]: nest.SetStatus(mm, 'interval': 0.1})

.. note:
   
   The set of variables to record from and the recording interval must
   be set **before** the multimeter is connected to any node, and
   cannot be changed afterwards.

After configuration, a ``multimeter`` can be connected to the neurons
it should record from by using the standard ``Connect`` routine
possibly including any of its usual parameters.

   ::

       In[3]: neurons = nest.Create("iaf_psc_alpha", 100)
       In[4]: nest.Connect(mm, neurons)

.. note:

   To ease the recording of the membrane potential, a pre-configured
   ``multimeter`` is available under the name ``voltmeter``.  Its
   `record_from` property is already set to record the variable
   ``V_m`` from the neurons it is connected to.

Collect event data from neurons and synapses
--------------------------------------------

Spike detector
##############

The most universal collector device is the ``spike_detector``. It
collects and records all *spikes* it receives from neurons that are
connected to it. The spike detector records spike times with full
precision from neurons emitting precisely timed spikes. [[Link to
guide for precise spike timing]]

Each spike event received by the spike detector is immediately handed
over to the prescribed recording backend for further processing.

Any node from which spikes are to be recorded, must be connected to
the spike detector using the standard ``Connect`` command. The
connection weight and delay are ignored by the spike detector.


Correlation detector
####################

[[TODO]]

Weight recorder
###############

[[Can we include the model documentation here?

Coolest would be if just the first two paragraphs with a "Read More"
link to the full documentation or maybe just the one-line summary
after the name in the old BeginDocumentation blocks?]]



Windowing
---------

Recording devices share the start, stop, and origin parameters global to
  devices. Start and stop have the following meaning for stimulating devices
  (origin is just a global offset):
  - Collectors collect all events with timestamps T that fulfill
      start < T <= stop.
    Note that events with timestamp T == start are NOT recorded.
  - Sampling devices sample at times t = nh with
      start < t <= stop
      (t-start) mod interval == 0



Where does data end up?
-----------------------



One question only touched upon slightly is what happens with recorded
data during a simulation.

This is the responsibility of the *recording backends*. A number of
them is already included in NEST and their usage and properties are
best explained using those as examples. The names of the backends are
given in parenthesis.

  Recording devices can collect data in memory, display it on the terminal
  output or write it to file in any combination. The output format can be
  controlled by device parameters as discussed below.


  - By default, devices record to memory. If you want to record to file, it may
    be a good idea to turn off recording to memory, to avoid that you computer's
    memory fills up with gigabytes of data: << /record_to [/ascii] >>.



How to set/retrieve general per-recording-backend properties and
per-device recording-backend settings


[[Include the backend documentations here]]


Store data in main memory
#########################

The `memory` backend is the default for all recording devices.

-  After one has simulated a little, the ``events`` entry of the
   multimeter status dictionary will contain one numpy array of data for
   each recordable.


Parameter summary
+++++++++++++++++

`events`
  is a dictionary which contains the sender global IDs of recorded
  events under the key *senders* and the time of the recording in
  *times*. The meaning of *times* depends on the setting of the
  property `time_in_steps`.

`n_events`
  is the number of events collected or sampled since the last reset of
  `n_events`. By setting `n_events` to 0, all spikes recorded so far
  will be discarded from memory.

`time_in_steps`
  A boolean (default: false) specifying whether to record time in
  steps, i.e. in integer multiples of the resolution and an offset,
  rather than just in ms. Please note that a given backend may chose
  to ignore this setting.


Write data to plain text files
##############################

The `ascii` recording backend writes collected data to a plain text
ASCII file. It can be used for small to medium sized simulations,
where the ease of a simple data format outweights the benefits of
high-performance output operations.

This backend will open one file per recording device per thread on
each MPI process. This can entail a very high load on the file system
in large simulations. In case of scaling problems, the `sionlib`
backend can be a possible alternative. [[Link]]

Filenames are determined according to the following pattern:

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
followed by the time of the measurement.

If the property `time_in_steps` is set to *false* (which is the
default), time is written as a floating point number representing the
simulation time in ms. If `time_in_steps` is *true*, the time of the
event is written as a pair consisting of the integer simulation time
step and the negative offset in ms from the next grid point.

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
  A boolean (default: false) specifying whether to record time in
  steps, i.e. in integer multiples of the resolution and an offset,
  rather than just in ms. Please note that a given backend may chose
  to ignore this setting.

Write data to the terminal
##########################

[[TODO]]

.. note::
   
   Using this backend for production runs is not recommended, as it
   may produce *huge* amounts of console output and thereby might slow
   down the simulation *considerably*.

Parameter summary
+++++++++++++++++

`precision`
  controls the number of decimal places used to write decimal numbers
  to the output file.

`time_in_steps`
  A boolean (default: false) specifying whether to record time in
  steps, i.e. in integer multiples of the resolution and an offset,
  rather than just in ms. Please note that a given backend may chose
  to ignore this setting.  

Store data to an efficient binary format
########################################

 (`sionlib`)

Stream data to an arbor instance:
#################################

 (`arbor`)

Writing own recording backends
------------------------------
