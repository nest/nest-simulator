.. _release_3.1:

What's new in NEST 3.1
======================

This page contains a summary of important breaking and non-breaking changes
from NEST 3.0 to NEST 3.1. In addition to the `release
notes on GitHub <https://github.com/nest/nest-simulator/releases/>`_,
this page also contains transition information that helps you to
update your simulation scripts when you come from an older version of
NEST.

If you transition from a version earlier than 3.0, please see our
extensive :ref:`transition guide from NEST 2.x to 3.0
<refguide_2_3>`.


Access to kernel properties
~~~~~~~~~~~~~~~~~~~~~~~~~~~

NEST 3.1 provides a new interface to the properties of the NEST
simulation kernel. Instead of using the traditional access methods
:py:func:`.GetKernelStatus` and :py:func:`.SetKernelStatus`,
properties can now also be set and retrieved via *direct kernel
attributes*.

Where you previously had ``nest.SetKernelStatus({"resolution": 0.2})``
in your simulation script, you can now just write ``nest.resolution =
0.2``. Kernel attributes now come with their own docstrings and even
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


Deprecation information
~~~~~~~~~~~~~~~~~~~~~~~

* The access functions :py:func:`.GetKernelStatus` and
  :py:func:`.SetKernelStatus` are deprecated. They will be
  removed in a future version of NEST. To avoid porting trouble
  later on, we suggest you switch to using the new interface
  for kernel properties now.
* Model ``pp_pop_psc_delta`` has been deprecated since 2016 and
  will be removed in NEST 3.2. Please use model ``gif_pop_psc_exp``
  instead.
* The `nest.hl_api` namespace contained the same members as `nest`
  and is being removed in NEST 3.2. All imports from `nest.hl_api`
  can be replaced by imports from `nest`.
