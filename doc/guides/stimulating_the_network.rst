Stimulating the network
=======================

Stimulating devices inject signals into a network, either as analog
signals such a currents or as spike trains. Most stimulating devices
are implemented so that they are replicated on each virtual
process. Many, but not all devices generating noise or stochastic
spike trains provide different signals to each of their recipients;
see the documentation of the individual device.

Stimulating devices share the start, stop, and origin parameters
global to devices. Start and stop have the following meaning for
stimulating devices (origin is just a global offset):

- For spike-emitting devices, only spikes with times t that fulfill
  start < t <= stop
  are emitted. Note that spikes that have time t==start are NOT emitted.
  
- For current-emitting devices, the current is activated and
  deactivated such that the current first affects the target dynamics
  during the update step (start, start+h], i.e., an effect can be
  recorded at the earliest at time start+h. The last interval during
  which the current affects the target's dynamics is (stop-h, stop].

Common recorder properties
--------------------------
.. glossary::

 stimulus_source
   A string (default: `""`) containing the name of the stimulating
   backend, where to get the data for updating the stimulating device.
   By default the device uses only its parameters for updating this stimulus.

 label
   A string (default: `""`) specifying an arbitrary textual label for
   the device.
   The `mpi` use this label to localize the file which contains port description.

How to update stimulating devices with MPI communication?
---------------------------------------------------------

The stimulating device can be updated between each run of Nest.
Actually, only `mpi` communication can replace the default one for updating the parameters
of the stimulating devices. This backend use the label of the device to find the port description for the connection
with the external software.
The format of data receiving by Nest for updating the stimulating devices depend on the type of devices. For more
information, you should look at the documentation of each device.

.. include:: ../models/stimulating_backend_mpi.rst

.. doxygengroup:: generator
   :content-only:
