.. _release_3.4:

All about NEST 3.4
==================

This page contains a summary of all breaking and non-breaking changes
from NEST 3.3 to NEST 3.4. In addition to the `auto-generated release
notes on GitHub <https://github.com/nest/nest-simulator/releases/>`_,
this page also contains transition information that helps you to
update your simulation scripts when you come from an older version of
NEST.

If you transition from a version earlier than 3.3, please see our
selection of earlier :ref:`transition guides <release_notes>`.

Deprecation information
~~~~~~~~~~~~~~~~~~~~~~~

* Model ``spike_dilutor`` is now deprecated and can only be used
  in single-threaded mode. To implement connections which transmit
  spikes with fixed probability, use ``bernoulli_synapse`` instead.
