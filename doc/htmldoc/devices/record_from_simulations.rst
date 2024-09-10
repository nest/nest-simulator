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

.. _recording_backends:

What values can I record?
-------------------------

This depends on neuron or synapse model specified.

You can get a list of properties that you can record using the ``recordables`` property.

        ::

         >>> nest.GetDefaults("iaf_cond_alpha")["recordables"]
         ["g_ex", "g_in", "t_ref_remaining", "V_m"]


Recorders available in NEST
~~~~~~~~~~~~~~~~~~~~~~~~~~~

- :doc:`/models/multimeter`


- :doc:`/models/spike_recorder`


- :doc:`/models/weight_recorder`

Check out the following examples to see how the recorders are used:

- :doc:`/auto_examples/recording_demo`
- :doc:`/auto_examples/multimeter_file`
- :doc:`/auto_examples/urbanczik_synapse_example` uses all 3 recorders.



Where does data end up?
-----------------------

After a recording device has collected or sampled data, the data is
handed to a dedicated *recording backend*, set for each recorder.
These are responsible for how the data are processed.

To specify the recording backend for a given recording device, the
property ``record_to`` of the latter has to be set to the name of the
recording backend to be used. This can either happen already in the
call to :py:func:`.Create` or by using :py:func:`.SetStatus` on the model instance.


::

 sr = nest.Create("spike_recorder", params={"record_to": "memory"})

Storing data in memory using the `memory` backend is the default for
all recording devices as this does not require any additional setup of
data paths or filesystem permissions and allows a convenient readout
of data by the user after simulation.

For example, you can use the ``events`` property to get the data output from the `memory` backend
from any of the recorders:

::

   >>> spike_recorder.get("events")
   {"senders": array([1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]),
   "times": array([ 2.1,  4.4,  6.7,  8.9, 11.1, 13.3, 15.5, 17.7, 19.9, 22.1, 24.3, 26.5, 28.7])}

Additional properties can be set, depending on the recorder and recording backend used.

For example:

::

   mm = nest.Create( "multimeter",
      params={"interval": 0.1, "record_from": ["V_m", "g_ex", "g_in"], "record_to": "ascii", "label": "my_multimeter"},
      )


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
   ("ascii", "memory", "mpi", "screen", "sionlib")

If a recording backend has global properties (i.e., parameters shared
by all enrolled recording devices), those can be inspected with
:py:func:`.GetDefaults`

::

   >>> nest.GetDefaults("sionlib")
   {"buffer_size": 1024,
    "filename": "",
    "sion_chunksize": 262144,
    "sion_collective": False,
    "sion_n_files": 1}

Such global parameters can be set using :py:func:`.SetDefaults`

::

   >>> nest.SetDefaults("sionlib", {"buffer_size": 512})

Built-in backends
-----------------

Following is a list of built-in recording backends that come with
NEST. Please note that the availability of some of them depends on the
compile-time configuration for NEST. See the backend documentation for
details.

.. include:: ../models/recording_backend_memory.rst
.. include:: ../models/recording_backend_ascii.rst
.. include:: ../models/recording_backend_screen.rst
.. include:: ../models/recording_backend_sionlib.rst
.. include:: ../models/recording_backend_mpi.rst
