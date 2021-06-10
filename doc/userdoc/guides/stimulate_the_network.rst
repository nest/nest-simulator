Stimulate the network
=====================

*Stimulation devices* (also called *stimulators* or *generators*)
inject signals into a network. The most common devices for injecting
analog signals (mostly currents) are:

- :doc:`../models/ac_generator`
- :doc:`../models/dc_generator`
- :doc:`../models/noise_generator`
- :doc:`../models/step_current_generator`

The most commonly used generators for spike trains are:

- :doc:`../models/poisson_generator`
- :doc:`../models/spike_generator`

Device properties
-----------------

Each stimulation device stores the data it needs for generating the
stimuli for connected nodes. Such data might be explicit lists of
values, or parametric values as for instance amplitudes and
frequencies.

.. _sec_stimulation_backends:

As an alternative to storing device data directly, an external data
source can be configured by setting the property `stimulus_source` to
a specific recording backend. Such an external source could be another
simulator, or a generic signal generator toolkit. The stimulation
backend can be updated between each call to ``Run()``.

The format of the data that has to be received by NEST for updating
the stimulation devices depends on the exact type of device. Please
refer to the documentation of the device for details. Below is a list
of available stimulation backends:

- :doc:`../models/stimulation_backend_mpi`
