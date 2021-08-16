All about NEST 3.1
==================

On this page you will find a summary of all breaking and non-breaking
changes from NEST 3.0 to NEST 3.1. On top of the simple auto-generated
release notes on GitHub, this page contains transition information
that allows you to update your scripts.

If you come from a version earlier than 3.0, please see our extensive
:doc:`transition guide from NEST 2.x to 3.0 <nest2_to_nest3/index>`.

New functions
~~~~~~~~~~~~~

The two functions :py:func:`.get` and :py:func:`.set` have been added
on the level of the ``nest`` Python module. For now, they are just
shortcuts for :py:func:`.GetKernelStatus` and
:py:func:`.SetKernelStatus`. As the new functions might replace the
older ones in a future version of NEST, we advise to use the shortcuts
already now.

+--------------------------------------------+----------------------------------+
| NEST 3.0                                   | NEST 3.1                         |
+============================================+==================================+
| nest.GetKernelStatus('network_size')       | nest.get('network_size)          |
+--------------------------------------------+----------------------------------+
| nest.SetKernelStatus({'resolution': 0.2})  | nest.set({'resolution': 0.2})    |
+--------------------------------------------+----------------------------------+
