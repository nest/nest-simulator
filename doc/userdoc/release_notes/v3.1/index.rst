All about NEST 3.1
==================

This page contains a summary of all breaking and non-breaking changes
from NEST 3.0 to NEST 3.1. In addition to the `auto-generated release
notes on GitHub <https://github.com/nest/nest-simulator/releases/>`_,
this page also contains transition information that helps you to
update your simulation scripts when you come from an older version of
NEST.

If you transition from a version earlier than 3.0, please see our
extensive :doc:`transition guide from NEST 2.x to 3.0
<nest2_to_nest3/index>`.

.. contents:: On this page you'll find
   :local:
   :depth: 1


Access to kernel properties
~~~~~~~~~~~~~~~~~~~~~~~~~~~

With NEST 3.1 it is easier than ever to access properties of the NEST
simulation kernel. In addition to the traditional access methods
:py:func:`.GetKernelStatus` and :py:func:`.SetKernelStatus`,
properties can now also be set and retrieved via *direct kernel
attributes*.

Where you previously had ``nest.SetKernelStatus({"resolution": 0.2})``
in your simulation script, you can now just write ``nest.resolution =
0.5``. Kernel attributes are first-class citizens of the NEST Python
module, which means that they come with their own docstrings and even
tab-completion works for them!

Co-dependent properties that have to be set together (for instance
``min_delay`` and ``max_delay``) can now be changed using the function
:py:func:`.set` on the level of the ``nest`` module.

  +-------------------------------------------------+---------------------------------------------+
  | NEST 3.0                                        | NEST 3.1                                    |
  +=================================================+=============================================+
  | ``nest.GetKernelStatus()``                      | ``nest.kernel_status``                      |
  +-------------------------------------------------+---------------------------------------------+
  | ``nest.GetKernelStatus('network_size')``        | ``nest.network_size``                       |
  +-------------------------------------------------+---------------------------------------------+
  | ``nest.SetKernelStatus({'resolution': 0.2})``   | ``nest.resolution = 0.2``                   |
  +-------------------------------------------------+---------------------------------------------+
  |  ::                                             |                                             |
  |                                                 |  ::                                         |
  |     nest.SetKernelStatus({                      |                                             |
  |         'min_delay': 0.2,                       |     nest.set(min_delay=0.2, max_delay=2.0)  |
  |         'max_delay': 2.0                        |                                             |
  |     })                                          |                                             |
  +-------------------------------------------------+---------------------------------------------+

.. admonition:: Deprecation info

      The use of the access functions :py:func:`.GetKernelStatus` and
      :py:func:`.SetKernelStatus` is now deprecated and they will be
      removed in a future version of NEST. To avoid porting trouble
      later on, we suggest you switch to using the new shortcuts
      already now.
