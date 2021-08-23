All about NEST 3.1
==================

On this page you will find a summary of all breaking and non-breaking
changes from NEST 3.0 to NEST 3.1. On top of the simple auto-generated
[release notes on GitHub, this page contains transition information
that helps you to update your scripts.

If you transition from a version earlier than 3.0, please see our
extensive :doc:`transition guide from NEST 2.x to 3.0
<nest2_to_nest3/index>`.

New functions
~~~~~~~~~~~~~

With NEST 3.1 it is easier than ever to access properties of the NEST
simulation kernel. In addition to the traditional access method via
:py:func:`.GetKernelStatus` and :py:func:`.SetKernelStatus`,
properties can now be set and retrieved via direct kernel attributes.

The co-dependent properties `min_delay` and `max_delay` can now be set
using a value tuple via the new kernel attribute `delay_extrema`.

As the old access functions might get removed in a future version of
NEST, we advise to use the new shortcuts already now.

+---------------------------------------------+------------------------------------+
| NEST 3.0                                    | NEST 3.1                           |
+=============================================+====================================+
| nest.GetKernelStatus()                      | nest.kernel_status                 |
+---------------------------------------------+------------------------------------+
| nest.GetKernelStatus('network_size')        | nest.network_size                  |
+---------------------------------------------+------------------------------------+
| nest.SetKernelStatus({'resolution': 0.2})   | nest.resolution = 0.2              |
+---------------------------------------------+------------------------------------+
| nest.SetKernelStatus({                      |                                    |
|     'min_delay': 0.1,                       | nest.delay_extrema = (0.05, 2.0)   |
|     'max_delay': 2.0                        |                                    |
| })                                          |                                    |
+---------------------------------------------+------------------------------------+
