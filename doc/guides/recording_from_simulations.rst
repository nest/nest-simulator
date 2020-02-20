Recording from simulations
==========================

*Recording devices* (or *recorders*, in short) are used to sample or
collect observable quantities like potentials, conductances, or spikes
from neurons and synapses.

To determine what happens to recorded data, each recording device can
specify a *recording backend* in its ``record_to`` property. The
default backend is *memory*, which just stores the recorded data in
memory for later retrieval. Other backends write the data to file or
to the screen. The different backends and their usage is explained in
detail in the section about :ref:`Recording Backends
<recording_backends>`.

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

The time span in which a recorder is actually recording, can be specified
using the additional properties ``start`` and ``stop``. These define
activation and inactivation times of the device in ms. An additional
property ``origin`` is available to shift the recording window by a
certain amount, which can be useful in simulation protocols with
repeated simulations. The following rules apply:

- Collectors collect all events with timestamps `T` that fulfill

  ::

     start < T <= stop.

  Events with timestamp `T == start` are not recorded.


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

.. glossary::

 label
   A string (default: `""`) specifying an arbitrary textual label for
   the device.  Recording backends might use the label to generate
   device specific identifiers like filenames and such.

 n_events
   The number of events that were collected by the recorder can be
   read out of the `n_events` entry. The number of events can be reset
   to 0. Other values cannot be set.

 origin
   A positive floating point number (default : `0.0`) used as the
   reference time for `start` and `stop`.

 record_to
   A string (default: `"memory"`) containing the name of the recording
   backend where to write data to. An empty string turns all recording
   of individual events off.

 start
   A positive floating point number (default: `0.0`) specifying the
   activation time in ms, relative to `origin`.

 stop
   A floating point number (default: `infinity`) specifying the
   deactication time in ms, relative to `origin`. The value of `stop`
   must be greater than or equal to `start`

Recorders for every-day situations
----------------------------------

.. include:: ../from_cpp/multimeter.rst

.. include:: ../from_cpp/spike_detector.rst

.. include:: ../from_cpp/weight_recorder.rst

.. _recording_backends:

Where does data end up?
-----------------------

After a recording device has sampled or collected data, the *recording backends* are
responsible for how the data are processed.


Theoretically, recording backends are not restricted in what they do
with the data. The ones included in NEST can collect data in memory,
display it on the terminal, or write it to files.

To specify the recording backend for a given recording device, the
property ``record_to`` of the latter has to be set to the name of the
recording backend to be used. This can either happen already in the
call to ``Create`` or by using ``SetStatus`` on the model instance.


::

 sd = nest.Create('spike_detector', params={'record_to': 'ascii'})

Storing data in memory using the `memory` backend is the default for
all recording devices as this does not require any additional setup of
data paths or filesystem permissions and allows a convenient readout
of data by the user after simulation.

Each recording backend may provide a specific set of parameters
(explained in the backend documentation below) that will be included
in the model status dictionary once the backend is set. This means
that these parameters can only be reviewed and changed *after* the
backend has been selected. In particular, recording-device specific
per-device parameters cannot be set using ``SetDefaults``, but must
rather be supplied either in the call to ``Create`` or set on an
instance using ``SetStatus``.

.. note::

   Even though parameters of different recording backends may have the
   same name, they are separate entities internally. This means that a
   value that was set for a parameter of a recording device when a
   specific backend was selected has to be *set again* on the new
   backend, if the backend is changed later on.

The full list of available recording backends and their respective
properties can be obtained from the kernel's status dictionary.

::

   nest.GetKernelStatus("recording_backends")
   {u'ascii': {},
    u'memory': {},
    u'screen': {},
    u'sionlib': {u'buffer_size': 1024,
     u'filename': u'',
     u'sion_chunksize': 262144,
     u'sion_collective': False,
     u'sion_n_files': 1}}

The example shows that only the `sionlib` backend has backend-specific
global properties, which can be modified by supplying a nested
dictionary to ``SetKernelStatus``.

::

    nest.SetKernelStatus({"recording_backends": {'sionlib': {'buffer_size': 512}}})

.. include:: ../from_cpp/recording_backend_memory.rst

.. include:: ../from_cpp/recording_backend_ascii.rst

.. include:: ../from_cpp/recording_backend_screen.rst

.. _sionlib_backend:

.. include:: ../from_cpp/recording_backend_sionlib.rst
