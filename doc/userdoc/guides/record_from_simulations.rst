.. _record_simulations:

Record from simulations
=======================

*Recording devices* (or *recorders*, in short) are used to sample or
collect observable quantities like potentials, conductances, or spikes
from neurons and synapses.

To determine what happens to recorded data, each recording device can
specify a *recording backend* in its ``record_to`` property. The
default backend is *memory*, which stores the recorded data in memory
for later retrieval. Other backends write the data to file, to the
screen, or stream it to other applications via the network. The
different backends and their usage are explained in detail in the
section about :ref:`Recording Backends <recording_backends>`.

Recording devices can only reliably record data that was generated
during the previous simulation time step interval. See the guide about
:ref:`running simulations <run_simulations>` for details about the
temporal aspects of the simulation loop.

.. note::

   Due to the need for internal buffering and the unpredictable order
   of thread execution, events are not necessarily recorded in
   chronological order.


Recording devices can fundamentally be subdivided into two groups:

- **Collectors** gather events sent to them. Neurons are connected to
  collectors and the collector gathers the events emitted by the
  neurons connected to it.

- **Samplers** actively interrogate their targets at given time
  intervals. This means that the sampler must be connected to the
  neuron (not the neuron to the sampler), and that the neuron must
  support the particular type of sampling.


Recorders for every-day situations
----------------------------------

- :doc:`../models/multimeter`
- :doc:`../models/spike_recorder`
- :doc:`../models/weight_recorder`

.. _recording_backends:

Where does data end up?
-----------------------

After a recording device has collected or sampled data, the data is
handed to a dedicated *recording backend*, set for each recorder.
These are responsible for how the data are processed.

Theoretically, recording backends are completely free in what they do
with the data. The ones included in NEST can collect data in memory,
display it on the terminal, or write it to files.

To specify the recording backend for a given recording device, the
property ``record_to`` of the latter has to be set to the name of the
recording backend to be used. This can either happen already in the
call to :py:func:`.Create` or by using :py:func:`.SetStatus` on the model instance.


::

 sr = nest.Create('spike_recorder', params={'record_to': 'ascii'})

Storing data in memory using the `memory` backend is the default for
all recording devices as this does not require any additional setup of
data paths or filesystem permissions and allows a convenient readout
of data by the user after simulation.

Each recording backend may provide a specific set of parameters
(explained in the backend documentation below) that will be included
in the model status dictionary once the backend is set. This means
that these parameters can only be reviewed and changed *after* the
backend has been selected. In particular, recording-device specific
per-device parameters cannot be set using :py:func:`.SetDefaults`, but must
rather be supplied either in the call to :py:func:`.Create` or set on an
instance using :py:func:`.SetStatus`.

.. note::

   Even though parameters of different recording backends may have the
   same name, they are separate entities internally. This means that a
   value that was set for a parameter of a recording device when a
   specific backend was selected has to be *set again* on the new
   backend, if the backend is changed later on.

The full list of available recording backends can be obtained from the
kernel attribute ``recording_backends``.

::

   >>> print(nest.recording_backends)
   ('ascii', 'memory', 'mpi', 'screen', 'sionlib')

If a recording backend has global properties (i.e., parameters shared
by all enrolled recording devices), those can be inspected with
:py:func`.GetDefaults`

::

   >>> nest.GetDefaults("sionlib")
   {'buffer_size': 1024,
    'filename': '',
    'sion_chunksize': 262144,
    'sion_collective': False,
    'sion_n_files': 1}

Such global parameters can be set using :py:func:`.SetDefaults`

::

   >>> nest.SetDefaults('sionlib', {'buffer_size': 512})

Built-in backends
-----------------

Following is a list of built-in recording backends that come with
NEST. Please note that the availability of some of them depends on the
compile-time configuration for NEST. See the backend documentation for
details.

- :doc:`../models/recording_backend_memory`
- :doc:`../models/recording_backend_ascii`
- :doc:`../models/recording_backend_screen`
- :doc:`../models/recording_backend_sionlib`
- :doc:`../models/recording_backend_mpi`
