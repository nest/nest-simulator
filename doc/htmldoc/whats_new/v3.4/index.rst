.. _release_3.4:

What's new in NEST 3.4
======================

This page contains a summary of important breaking and non-breaking
changes from NEST 3.3 to NEST 3.4. In addition to the `release notes
on GitHub <https://github.com/nest/nest-simulator/releases/>`_, this
page also contains transition information that helps you to update
your simulation scripts when you come from an older version of NEST.

If you transition from an earlier version, please see our extensive
:ref:`transition guide from NEST 2.x to 3.0 <refguide_2_3>` and the
:ref:`list of updates for previous releases in the 3.x series
<whats_new>`.

Documentation restructuring and new theme
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

NEST documentation has a new theme! We did a major overhaul of the layout and structure of the documentation.
The changes aim to improve findability and access of content. With a more modern
layout, our wide range of docs can be discovered more easily.
The table of contents is simplified and the content is grouped based on topic (neurons, synapses etc)
rather than type of documentation (e.g., 'guides').

The table of contents is simplified and the content is grouped based
on topics (neurons, synapses etc) rather than type of documentation
(e.g., 'guides').

We would be highly interested in any feedback you might have on the
new look-and-feel either on `our mailing list
<https://www.nest-simulator.org/community/>`_ or as an `issue on
GitHub
<https://github.com/nest/nest-simulator/issues/new?template=documentation_improvement.md>`_

Query spatially structured networks from target neuron perspective
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Spatial layers can be created by specifying only the node positions using ``spatial.free``,
without explicitly specifying the ``extent``.
In that case, in NEST 3.4 and later, the ``extent`` will be determined by the position of the
lower-leftmost and upper-rightmost nodes in the layer; earlier versions of NEST added a hard-coded
padding to the extent. The ``center`` is computed as the midpoint between the lower-leftmost and
upper-rightmost nodes.

Extent and center for spatial layers with freely placed neurons
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Spatial layers in NEST can be created by specifying node positions in
the call to :py:func:`.Create` using :ref:`spatial distributions <pynest_spatial>`
from ``nest.spatial``.

When using :py:class:`.spatial.free`, the layer's ``extent`` will be
determined automatically based on the positions of the lower-leftmost
and upper-rightmost nodes in the layer, if omitted. While earlier
versions of NEST added a hard-coded padding, NEST 3.4 will only use
the node positions.

* Model ``spike_dilutor`` is now deprecated and can only be used
  in single-threaded mode. To implement connections which transmit
  spikes with fixed probability, use ``bernoulli_synapse`` instead.


Changes in NEST Server
~~~~~~~~~~~~~~~~~~~~~~

* By default NEST Server runs on port 52425 (previously 5000).
* Minimize security risk in NEST Server.
  * By default exec call is disabled, only API calls are enabled.
  * The user is able to turn on exec call which means that the user is aware of the risk.
