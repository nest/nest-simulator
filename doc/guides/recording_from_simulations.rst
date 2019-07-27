Recording from simulations
==========================

*Recording devices* (or *recorders*, in short) are used to obtain and
record different quantities of neurons or synapses.

The recorded data is stored by *recording backend*s. Each recording
device can specify a number of recording backends to be used. The
default is to store data in memory by using the `memory` backend. The
different backends and their usage is explained in detail below
[[Anchor: Recording Backends]].

Recording devices can fundamentally be subdivided into two groups:

- **Collector devices** collect events sent to them; the archetypical
  example for this category is the ``spike_detector``. Nodes are
  connected to collectors and the collector collects all events
  emitted by the nodes connected to it.
- **Sampling devices** actively interrogate their targets at given
  time intervals and record the data they obtain. This means that the
  sampler must be connected to the node (not the node to the sampler),
  and that the node must support the particular type of sampling; the
  device specific documentation contains details.

Recording devices can only reliably record data that was generated
during the previous min_delay interval [[Link to Running
Simulations]].

.. note:

   Due to the need for internal buffering and the unpredictable order
   of thread execution, events are not necessarily recorded in
   chronological order. For backend-specific details, see the
   corresponding documentation.

Common recorder properties
--------------------------

All recorders share a set of common properties that can be set using
``SetStatus`` on the corresponding device:

`record_to`
  A list (default: ["memory"]) containing any combination of names of
  recording backends, indicating where to write data to. An empty
  array turns all recording of individual events off.

`label`
  A string (default: "") specifying an arbitrary textual label for the
  device.  Recording backends might use the label to generate device
  specific identifiers like filenames and such.

`time_in_steps`
  A boolean (default: false) specifying whether to output time in
  steps, i.e. in integer multiples of the resolution plus an offset,
  rather than in ms. Please note that a given backend may chose to
  ignore this setting.


Sampling continuous quantities from neurons
-------------------------------------------

All models that support analog sampling have a `recordables` property,
which lists all recordable quantities. The property can be inspected
using ``GetDefaults`` on the model or ``GetStatus`` on a model
instance.

   ::

       In [0]: nest.GetDefaults('iaf_cond_alpha')['recordables']
       Out[0]: ['V_m', 'g_ex', 'g_in', 't_ref_remaining']

The property `record_from` of the main device for analog sampling, the
 ``multimeter``, is used to set the names of the quantities to be
 sampled.

   ::

       In[1]: nest.Create('multimeter', params={'record_from': ['V_m', 'g_ex']})

All sampling devices share a common parameter `interval` that controls
the sampling interval of recordings. It defaults to 1 ms and can be
changed either during the call to ``Create`` or using ``SetStatus``.

   ::

       In[2]: nest.Create('multimeter', params={'record_from': ['V_m'], 'interval' :0.1})

.. note:
   
   The set of variables to record from and the recording interval must
   be set **before** the multimeter is connected to any node, and
   cannot be changed afterwards.

In order to make recording the membrane potential of a neuron easy, a
pre-configured ``multimeter`` is available as ``voltmeter``. It's
`record_from` property is already set to record the variable ``V_m``
from the neurons it is connected to.

[[The variable V_m ideally would link somehow to the variable name
convention, i.e. the Dayan Abbott book default names]]

Sampling devices can be connected to the neurons they record from
using the usual ``Connect`` routine with all its parameters.

   ::

       In[3]: neurons = nest.Create("iaf_psc_alpha", 100)
       In[4]: voltmeter = nest.Create("voltmeter")
       In[5]: nest.Connect(voltmeter, neurons)

Collect event data from neurons
-------------------------------

[[@Jessica: Where is the documentation of the recording devices in the
doc tree? I think that their model documentation should be included
here.]]

The most simple and universal collector device is the ``spike_detector``.


``correlation_detector``



Record weights from synapses
----------------------------

The ``weight_recorder``






Windowing
---------


[[Ideally, this would be a separate page to which this page and the
one for stimulation devices could link.]]

The operation times of recording devices can be controlled using the
flags start stop origin...


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


Write data to plain text files (`ascii`)
########################################

The `ascii` recording backend writes collected data to a plain text
ASCII file. It can be used for small to medium sized simulations,
where the ease of a simple data format outweights the benefits of
high-performance output operations.

This backend will open one file per recording device per thread on
each MPI process. This can entail a very high load on the file system
in large simulations. In case of scaling problems, the `sionlib`
backend can be a possible alternative. [[Link]]

Filenames are determined according to the following rule:

   data_path/data_prefix(label|model_name)-gid-vp.file_extension

The properties `data_path` and `data_prefix` are global kernel
properties. They can for example be set during repetitive simulation
protocols to separate the data resulting from indivitual runs. `gid`
and `vp` correspond to the global ID and the virtual process of the
recorder, respectively.

The `file_extension` can currently be only set on a per-backend basis.

The life cycle of a file always spans the period between the call to
``Prepare`` and the call to ``Cleanup``. Each call to ``Run`` between
them will use the same file.

In case, a file of the same name already exists, the ``Prepare`` call
will fail with a corresponding error message, unless the kernel
property[[link to SetKernelStatus]] `overwrite_files` is set to
*true*.

Data format
+++++++++++

The first line written to any new file is a header explaining the
position of the data entries. The header starts with a `#` character.

The first field in the output is always the global id of the neuron,
the record originates from (or the *source* neuron) followed by the
time of the measurement.

If the recorder property `time_in_steps` is set to *false* (which is
the default), time is written as a floating point number which
represents the simulation time in ms. If `time_in_steps` is `true`,
the time of the event is written as a pair consisting of the integer
simulation time step and the negative offset in ms from the next grid
point.



.. note::

   The number of decimal places for all decimal numbers in the output
   of a recording device can be controlled using the recorder property
   `precision`.





`file_extension`
  String specifying the file name extension, without leading dot. The
  default depends on the specific device. ???JME


Document that Simulate used to append to files previously unless
close_after_simulate (default:false) was set to true and will now
attempt to overwrite files now.




  See /label and /file_extension for how to change the name.


  If you later turn recording to file on again, the
  file will be overwritten, unless you have changed data_prefix,
  label, or file_extension.



`filenames`
  Array containing the filenames where data is recorded to. This array
  has one entry per local thread and is only available if /to_file is
  set to true, or if /record_to contains /to_file.


`use_gid_in_filename`
  Determines if the GID is used in the file name of the recording
  device. Setting this to false can lead to conflicting file
  names. ???JME

`label`
  String specifying an arbitrary textual label for the
  device. Recording backends might use this to generate device
  specifiers like i.e. filenames.


Write data to the terminal (`screen`)
#####################################

This is not recommended for production runs, as it may produce *huge*
amounts of output to the terminal and thereby may slow down the
simulation *considerably*.

- Write time in double, ???JME
- Allow to set the precision of time and values (default to 3 digits) ???JME


Store data in the computer memory (`memory`)
############################################

-  After one has simulated a little, the ``events`` entry of the
   multimeter status dictionary will contain one numpy array of data for
   each recordable.


  Data recorded in memory is available through the following parameter:
  /n_events      - Number of events collected or sampled. n_events can be set to
                   0, but no other value. Setting n_events to 0 will delete all
                   spikes recorded in memory. n_events will count events even
                   when not recording to memory.


  /events        - Dictionary with elements /senders (sender GID), /times (spike
                   times in ms or steps, depending on /time_in_steps) and
                   /offsets (only if /time_in_steps is true). All data stored in
                   memory is erased when /n_events is set to 0.

[[Note]]
- If using a backend that writes data toi disk, you may want to
  disable this backend in order to conserve memory
-


Store data to an efficient binary format (`sionlib`)
####################################################

Stream data to an arbor instance: (`arbor`)
###########################################





Setting backend properties
--------------------------

%  JME:- Provide NEST API + PyNEST function for accessing backend settings




Writing own recording backends
------------------------------
