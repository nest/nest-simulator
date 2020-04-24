

Write data to the terminal
##########################

When initially conceiving and debugging simulations, it can be useful
to check recordings in a more ad hoc fashion. The recording backend
`screen` can be used to dump all recorded data onto the console for
quick inspection.

The first field of each record written is the node ID of the neuron
the event originated from, i.e., the *source* of the event. This is
followed by the time of the measurement, the recorded floating point
values, and the recorded integer values.

The format of the time field depends on the value of the property
``time_in_steps``. If set to *false* (which is the default), time is
written as one floating point number representing the simulation time
in ms. If ``time_in_steps`` is *true*, the time of the event is written
as a value pair consisting of the integer simulation time step and the
floating point offset in ms from the next grid point.

.. note::

   Using this backend for production runs is not recommended, as it
   may produce *huge* amounts of console output and *considerably* slow
   down the simulation.

Parameter summary
+++++++++++++++++

.. glossary::

 precision
  controls the number of decimal places used to write decimal numbers
  to the terminal.

 time_in_step
  A boolean (default: false) specifying whether to print time in
  steps, i.e., in integer multiples of the resolution and an offset,
  rather than just in ms.

