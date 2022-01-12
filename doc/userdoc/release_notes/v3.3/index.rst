All about NEST 3.3
==================

This page contains a summary of all breaking and non-breaking changes
from NEST 3.2 to NEST 3.3. In addition to the `auto-generated release
notes on GitHub <https://github.com/nest/nest-simulator/releases/>`_,
this page also contains transition information that helps you to
update your simulation scripts when you come from an older version of
NEST.

If you transition from a version earlier than 3.0, please see our
selection of earlier :doc:`transition guides <release_notes/index>`.

.. contents:: On this page you'll find
   :local:
   :depth: 1

Model defaults
~~~~~~~~~~~~~~

The model parameter ``delta_tau`` in the ``correlation_detector``,
``correlomatrix_detector``, and ``correlospinmatrix_detector``, as
well as ``dt`` in the ``noise_generator`` are now automatically
adjusted and made compatible with a newly set simulation resolution to
avoid errors when those models are instantiated. Moreover, the default
value for ``delta_tau`` in the ``correlation_detector`` has been
changed from 1.0 to 5 times the simulation resolution, in order to be
consistent with the documentation of the device.
