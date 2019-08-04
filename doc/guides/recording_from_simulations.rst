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

The time span, a recorder is actually recording, can be specified
using the additional properties `start` and `stop`, which define
activation and inactivation times of the device in ms. An additional
property `origin` is available to shift the recording window by a
certain amount, which can be useful in simulation protocols with
repeaded simulations. The follow rules apply:

- Collectors collect all events with timestamps T that fulfill
  
  ::
	
     start < T <= stop.

  Events with timestamp T == start are not recorded.


- Sampling devices sample at times *t = nh* (*h* being the simulation
  resolution) with

  ::

     start < t <= stop
     (t-start) mod interval == 0

   
Common recorder properties
--------------------------

All recorders have a set of common properties that can be set using
``SetDefaults`` on the model class or ``SetStatus`` on a device
instance:

`label`
  A string (default: `""`) specifying an arbitrary textual label for
  the device.  Recording backends might use the label to generate
  device specific identifiers like filenames and such.

`origin`:
  A positive floating point number (default : `0.0`) used as the
  reference time for `start` and `stop`.
  
`record_to`
  A string (default: `"memory"`) containing the name of the recording
  backend where to write data to. An empty string turns all recording
  of individual events off.

`start`
  A positive floating point number (default: `0.0`) specifying the
  activation time in ms, relative to `origin`.

`stop`
  A floating point number (default: `infinity`) specifying the
  deactication time in ms, relative to `origin`. The value of `stop`
  must be greater than or equal to `start`


Sampling continuous quantities from neurons
-------------------------------------------

Most sampling use cases are covered by the ``multimeter``, which
allows to record analog values from neurons. Models which have such
values expose a ``recordables`` property that lists all recordable
quantities.  This property can be inspected using ``GetDefaults`` on
the model class or ``GetStatus`` on a model instance. It cannot be
changed by the user.

::

   >>> nest.GetDefaults('iaf_cond_alpha')['recordables']
   ['g_ex', 'g_in', 't_ref_remaining', 'V_m']

The ``record_from`` property of a ``multimeter`` (a list, empty by
default) can be set to contain the name(s) of one or more of these
recordables to have them sampled during simulation.

::

   >>> mm = nest.Create('multimeter', 1, {'record_from': ['V_m', 'g_ex']})

The sampling interval for recordings (given in ms) can be controlled
using the ``multimeter`` parameter `interval`.  The default value of
1.0 ms can be changed by supplying a new value either in the call to
``Create`` or by using ``SetStatus`` on the model instance.

::

   >>> nest.SetStatus(mm, 'interval': 0.1})

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

   >>> neurons = nest.Create('iaf_psc_alpha', 100)
   >>> nest.Connect(mm, neurons)

To learn more about possible connection patterns and additional
options when using ``Connect``, see the guide on :doc:`connection
management <connection_management>`.

The abbove call to ``Connect`` would fail if the neurons would not
support the sampling of the values *V_m* and *g_ex*. It would also
fail if carried out in the wrong direction, i.e. trying to connect the
*neurons* to *mm*.

.. note::
   To ease the recording of the membrane potential, a pre-configured
   ``multimeter`` is available under the name ``voltmeter``.  Its
   `record_from` property is already set to record the variable `V_m`
   from the neurons it is connected to.

Collect event data from neurons and synapses
--------------------------------------------

.. doxygengroup:: spike_detector
   :content-only:

.. doxygengroup:: correlation_detector
   :content-only:

.. doxygengroup:: weight_recorder
   :content-only:


.. _recording_backends:

Where does data end up?
-----------------------

The way in which data is processed after a recording device has
sampled or collected it is the responsibility of the *recording
backends*.

Theoretically, recording backends are not restricted in what they do
with the data. The ones included in NEST can collect data in memory,
display it on the terminal, write it to file, or stream it out to
other applications.

To specify the recording backend for a given recording device, the
property ``record_to`` of the latter has to be set to the name of the
recording backend to be used. This can either happen already in the
call to ``Create`` or by using ``SetStatus`` on the model instance.

::

   >>> sd = nest.Create('spike_detector', params={'record_to': 'ascii'})

Storing data in memory using the `memory` backend is the default for
all recording devices as this does not require any additional setup of
data paths or filesystem permissions and allows a convenient readout
of data by the user after simulation.
   
Each recording backend provides a different set of parameters
(explained in the backend documentation below) that will be included
in the model status dictionary once the backend is set. This means
that they can only be reviewed and changed *after* the backend has
been selected.

.. note::
   Even though parameters of different recording backends may have the
   same name, they are separate entities internally. This means that a
   value that was set for a paramater of a recording device when a
   specific backend was selected has to be *set again* on the new
   backend, if the backend is changed later on.

.. _memory_backend:

Store data in main memory
#########################

When a recording device sends data to the ``memory`` backend, it is
internally stored in efficient vectors, that are made available to the
user level in the devices' status dictionary under the key `events`.

The `events` dictionary always contains the global IDs of the source
nodes of the recorded data in the field `sender`. It also always
contains the time of the recording. Depending on the setting of the
property `time_in_steps`, this time can be stored in two different
formats:

- if `time_in_steps` is `false` (which is the default) the time is
  stored as a single floating point number in the field `times`,
  interpreted as the simulation time in ms

- if `time_in_steps` is `true`, the time is stored as a pair
  consisting of the integer number of simulation time steps in units
  of the simulation resolution in `times` and the negative offset from
  the next such grid point as a floating point number in ms in
  `offset`.

All additional data collected or sampled by the recording device is
contained in the *events* dictionary in arrays named as the recordable
it came from and with the appropriate data type (either integer or
floating point).

The number of events that have been collected by the ``memory``
backend can be read out of the *n_events* entry in the status
dictionary of the recording device.

To delete data from memory between consecutive calls to the ``Run``
function in the context of :doc:`stepped simulations
<running_simulations#stepped_simulations>`, the value of *n_events*
can be set to 0. Other values cannot be set.

If the data is not deleted manually, it is kept for readout until the
next call to ``Prepare`` or ``Simulate`` and discared before any new
data is recorded.

Parameter summary
+++++++++++++++++

`events`
  A dictionary containing the recorded data in the form of one numeric
  array for each quantity measured. It always has the sender global
  IDs of recorded events under the key *senders* and the time of the
  recording, the format of which depends on the setting of
  `time_in_steps`.

`n_events`
  The number of events collected or sampled since the last reset of
  `n_events`. By setting `n_events` to 0, all events recorded so far
  will be discarded from memory.

`time_in_steps`
  A Boolean (default: *false*) specifying whether to store time in
  steps, i.e. in integer multiples of the simulation resolution (under
  the key *times* of the *events* dictionary) plus a floating point
  number for the negative offset from the next grid point in ms (under
  key *offset*), or just the simulation time in ms under key *times*.

.. _ascii_backend:

Write data to plain text files
##############################

The `ascii` recording backend writes collected data persistently to a
plain text ASCII file. It can be used for small to medium sized
simulations, where the ease of a simple data format outweights the
benefits of high-performance output operations.

This backend will open one file per recording device per thread on
each MPI process. This can entail a very high load on the file system
in large simulations. Especially on machines with distributed
filesystems using this backend can become prohibitively inefficient.
In case of experiencing such scaling problems, the :ref:`SIONlib
backend <sionlib_backend>` can be a possible alternative.

Filenames of data files are determined according to the following
pattern:

::

   data_path/data_prefix(label|model_name)-gid-vp.file_extension

The properties `data_path` and `data_prefix` are global kernel
properties. They can for example be set during repetitive simulation
protocols to separate the data originating from indivitual runs. The
`label` replaces the model name component if it is set to a non-empty
string. `gid` and `vp` denote the zero-padded global ID and virtual
process of the recorder writing the file. The filename ends in a dot
and the `file_extension`.

The life of a file starts with the call to ``Prepare`` and ends with
the call to ``Cleanup``. Data that is produced during successive calls
to ``Run`` inbetween a pair of ``Prepare`` and ``Cleanup`` calls will
be written to the same file, while the call to ``Run`` will flush all
data to the file, so it is available for immediate inspection.

In case, a file of the designated name for a new recording already
exists, the ``Prepare`` call will fail with a corresponding error
message. To instead overwrite the old file, the kernel property
`overwrite_files` can be set to *true* using ``SetKernelStatus``.  An
alternative way for avoiding name clashes is to re-set the kernel
properties `data_path` or `data_prefix`, so that another filename is
chosen.

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
written as a single floating point number representing the simulation
time in ms. If `time_in_steps` is *true*, the time of the event is
written as a pair of values consisting of the integer simulation time
step in units of the simulation resolution and the negative floating
point offset in ms from the next integer grid point.

.. note::
   The number of decimal places for all decimal numbers written can be
   controlled using the recorder property `precision`.

Parameter summary
+++++++++++++++++

`file_extension`
  A string (default: *"dat"*) that specifies the file name extension,
  without leading dot. The generic default was chosen, because the
  exact type of data cannot be known a priori.

`filenames`
  A list of the filenames where data is recorded to. This list has one
  entry per local thread and is a read-only property.

`label`
  A string (default: *""*) that replaces the model name component in
  the filename if it is set.

`precision`
  An integer (default: *3*) that controls the number of decimal places
  used to write decimal numbers to the output file.

`time_in_steps`
  A Boolean (default: *false*) specifying whether to write time in
  steps, i.e. in integer multiples of the simulation resolution plus a
  floating point number for the negative offset from the next grid
  point in ms, or just the simulation time in ms.

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
