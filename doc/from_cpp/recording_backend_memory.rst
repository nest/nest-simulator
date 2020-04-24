

Store data in main memory
#########################

When a recording device sends data to the ``memory`` backend, it is
stored internally in efficient vectors. These vectors are made available to the
user level in the device's status dictionary under the key ``events``.

The ``events`` dictionary always contains the global IDs of the source
nodes of the recorded data in the field ``sender``. It also always
contains the time of the recording. Depending on the setting of the
property ``time_in_steps``, this time can be stored in two different
formats:

- If ``time_in_steps`` is `false` (which is the default), the time is
  stored as a single floating point number in the field ``times``,
  interpreted as the simulation time in ms

- If ``time_in_steps`` is `true`, the time is stored as a pair
  consisting of the integer number of simulation time steps in units
  of the simulation resolution in ``times`` and the negative offset from
  the next such grid point as a floating point number in ms in
  ``offset``.

All additional data collected or sampled by the recording device is
contained in the ``events`` dictionary in arrays. These data are named based on
the recordable they came from and with the appropriate data type (either integer or
floating point).

The number of events that were collected by the ``memory`` backend can
be read out of the `n_events` entry in the status dictionary of the
recording device. To delete data from memory, `n_events` can be set to
0. Other values cannot be set.

Parameter summary
+++++++++++++++++

.. glossary::

 events
   A dictionary containing the recorded data in the form of one numeric
   array for each quantity measured. It always has the sender global
   IDs of recorded events under the key ``senders`` and the time of the
   recording, the format of which depends on the setting of
   ``time_in_steps``.

 n_events
   The number of events collected or sampled since the last reset of
   `n_events`. By setting `n_events` to 0, all events recorded so far
   will be discarded from memory.

 time_in_steps
   A Boolean (default: *false*) specifying whether to store time in
   steps, i.e., in integer multiples of the simulation resolution
   (under the key ``times`` of the ``events`` dictionary) plus a
   floating point number for the negative offset from the next grid
   point in ms (under key ``offset``), or just the simulation time in
   ms under key ``times``. This property cannot be set after Simulate
   has been called.

