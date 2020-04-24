

.. _ascii_backend:

Write data to plain text files
##############################

The `ascii` recording backend writes collected data persistently to a
plain text ASCII file. It can be used for small to medium sized
simulations, where the ease of a simple data format outweighs the
benefits of high-performance output operations.

This backend will open one file per recording device per thread on
each MPI process. This can cause a high load on the file system
in large simulations. This backend can become prohibitively inefficient,
particularly on machines with distributed filesystems.
In case you experience such scaling problems,  the :ref:`SIONlib
backend <sionlib_backend>` may be a possible alternative.

Filenames of data files are determined according to the following
pattern:

::

   data_path/data_prefix(label|model_name)-node_id-vp.file_extension

The properties ``data_path`` and ``data_prefix`` are global kernel
properties. They can, for example, be set during repetitive simulation
protocols to separate the data originating from individual runs. The
``label`` replaces the model name component if it is set to a non-empty
string. ``node_id`` and ``vp`` denote the zero-padded global ID and virtual
process of the recorder writing the file. The filename ends in a dot
and the ``file_extension``.

The life of a file starts with the call to ``Prepare`` and ends with
the call to ``Cleanup``. Data that is produced during successive calls
to ``Run`` in between a pair of ``Prepare`` and ``Cleanup`` calls will
be written to the same file, while the call to ``Run`` will flush all
data to the file, so it is available for immediate inspection.

When creating a new recording, if the file name already
exists, the ``Prepare`` call will fail with a corresponding error
message. To instead overwrite the old file, the kernel property
``overwrite_files`` can be set to *true* using ``SetKernelStatus``. An
alternative way for avoiding name clashes is to re-set the kernel
properties ``data_path`` or ``data_prefix``, so that another filename is
chosen.

Data format
+++++++++++

Any file written by the `ascii` recording backend starts with an
informational header. The first header line contains the NEST version,
with which the file was created, followed by the version of the
recording backend in the second. The third line describes the data by
means of the field names for the different columns. All lines of the
header start with a `#` character.

The first field of each record written is the node ID of the neuron
the event originated from, i.e., the *source* of the event. This is
followed by the time of the measurement, the recorded floating point
values and the recorded integer values.

The format of the time field depends on the value of the property
``time_in_steps``. If set to *false* (which is the default), time is
written as a single floating point number representing the simulation
time in ms. If ``time_in_steps`` is *true*, the time of the event is
written as a pair of values consisting of the integer simulation time
step in units of the simulation resolution and the negative floating
point offset in ms from the next integer grid point.

.. note::

   The number of decimal places for all decimal numbers written can be
   controlled using the recorder property ``precision``.

Parameter summary
+++++++++++++++++

.. glossary::

 file_extension
   A string (default: *"dat"*) that specifies the file name extension,
   without leading dot. The generic default was chosen, because the
   exact type of data cannot be known a priori.

 filenames
   A list of the filenames where data is recorded to. This list has one
   entry per local thread and is a read-only property.

 label
   A string (default: *""*) that replaces the model name component in
   the filename if it is set.

 precision
   An integer (default: *3*) that controls the number of decimal places
   used to write decimal numbers to the output file.

 time_in_steps
   A Boolean (default: *false*) specifying whether to write time in
   steps, i.e., in integer multiples of the simulation resolution plus
   a floating point number for the negative offset from the next grid
   point in ms, or just the simulation time in ms. This property
   cannot be set after Simulate has been called.

