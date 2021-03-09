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

   
.. doxygengroup:: generator
   :content-only:
