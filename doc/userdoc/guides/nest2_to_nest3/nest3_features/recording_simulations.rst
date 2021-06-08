Recording from simulations
==========================

The ``spike_detector`` has been renamed to ``spike_recorder``. The main
rationale behind this is that the device is actually not detecting the
occurence of spikes, but rather only records them. Moreover, the new
name is more consistent with the naming of other similar devices that
also end in `_recorder`.

In NEST 2.x, all recording modalities (i.e. *screen*, *memory*, and
*files*) were handled by a single C++ class. Due to the many different
responsibilities and the resulting complexity of this class, extending
and maintaining it was rather burdensome.

With NEST 3.0 we replaced this single class by an extensible and
modular infrastructure for handling recordings: each modality is now
taken care of by a specific recording backend and each recorder can
use one of them to handle its data.

NEST 3.0 provides individual recording backends for all modalities
that were supported in NEST 2.x. Depending on the features selected
during the configuration of NEST, additional backend become available:
Support for `SIONlib <http://www.fz-juelich.de/jsc/sionlib>`_ leads to
the inclusion of a backend for writing binary files in parallel on
large clusters and supercomputers. If MPI is enabled, a special data
exchange backend is built, which is useful in co-simulation scenarios.

See the guide on :doc:`recording from simulations
<../../record_from_simulations>` for details on potentially
available recording backends.

Changes
^^^^^^^

In NEST 2.x, the recording modality was selected by either providing a
list of modalities to the `record_to` property, or by setting one or
more of the flags `to_file`, `to_memory`, or `to_screen` to *True*.

In NEST 3.0, the individual flags are gone, and the `record_to`
property now expects the name of the backend you want to use. Recording to
multiple modalities from a single device is no longer possible.
Individual devices have to be created and configured if this
functionality is needed.

The following examples assume that the variable `mm` points to a
``multimeter`` instance, i.e.,  ``mm = nest.Create('multimeter')``
was executed.


  +------------------------------------------------------+------------------------------------+
  | NEST 2.x                                             | NEST 3.0                           |
  +------------------------------------------------------+------------------------------------+
  |                                                      |                                    |
  | ::                                                   | ::                                 |
  |                                                      |                                    |
  |     nest.SetStatus(mm, {'record_to': ["file"]})      |     mm.record_to = "ascii"         |
  |     nest.SetStatus(mm, {'record_to': ["screen"]})    |     mm.record_to = "screen"        |
  |     nest.SetStatus(mm, {'record_to': ["memory"]})    |     mm.record_to = "memory"        |
  |                                                      |                                    |
  +------------------------------------------------------+------------------------------------+
  | ::                                                   | ::                                 |
  |                                                      |                                    |
  |     nest.SetStatus(mm, {'to_file': True})            |     mm.record_to = "ascii"         |
  |     nest.SetStatus(mm, {'to_screen': True})          |     mm.record_to = "screen"        |
  |     nest.SetStatus(mm, {'to_memory': True})          |     mm.record_to = "memory"        |
  |                                                      |                                    |
  +------------------------------------------------------+------------------------------------+
  |                                                      |                                    |
  | ::                                                   | ::                                 |
  |                                                      |                                    |
  |     nest.Create('spike_detector')                    |     nest.Create('spike_recorder')  |
  |                                                      |                                    |
  +------------------------------------------------------+------------------------------------+

You can retrieve the list of available backends using the following command:

 ::

    list(nest.GetKernelStatus("recording_backends").keys())

Previously, the content and formatting of any output created by a
recording device could be configured in a fine-grained fashion using
flags like `withgid`, `withtime`, `withweight`, `withport`, among others.
In many cases, this, however, lead to a confusing variety of
possible interpretations of data columns for the resulting output.

As storage space is usually not a concern nowadays, the new
infrastructure does not have this plethora of options, but rather
always writes all available data. In addition, most backends now write
the name of the recorded variable for each column as a descriptive
meta-data header prior to writing any data.

The `accumulator_mode` of the ``multimeter`` has been dropped, as it
was not used by anyone to the best of our knowledge and supporting it
made the code more complex and prone to errors. In case of high user
demand, the functionality will be re-added in form of a recording
backend.

All details about the new infrastructure can be found in the guide on
:doc:`recording from simulations <../../record_from_simulations>`.
