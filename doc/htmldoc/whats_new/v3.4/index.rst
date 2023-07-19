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

NEST documentation has a new theme! We did a major overhaul of the
layout and structure of the documentation.  The changes aim to improve
findability and access of content. With a more modern layout, our wide
range of docs can be discovered more easily.

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

PyNEST now provides functions :py:func:`.GetSourceNodes`,
:py:func:`.GetSourcePositions`, and :py:func:`.PlotSources` which
allow you to query or plot the source neurons of a given target
neuron.

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

Likewise, the ``center`` of a layer is now automatically computed as
the midpoint between the lower-leftmost and the upper-rightmost nodes.

When creating a layer with only a single node, the ``extent`` still
has to be specified explicitly.

Disconnect with ``SynapseCollection``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

It is now possible to disconnect nodes using a ``SynapseCollection``
as argument to either :py:func:`.disconnect` or the member function
``disconnect()`` of the ``SynapseCollection``.

Removal of deprecated models
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* The models ``iaf_psc_alpha_canon`` and ``pp_pop_psc_delta`` have
  long been deprecated and were now removed from NEST. In case you
  depend on them, you will find similar functionality in the
  replacement models :doc:`iaf_psc_alpha_ps
  </models/iaf_psc_alpha_ps>` and :doc:`iaf_psc_alpha_ps
  </models/gif_pop_psc_exp>`, respectively.

* Model ``spike_dilutor`` is now deprecated and can only be used in
  single-threaded mode. To implement connections which transmit spikes
  with fixed probability, use :doc:`bernoulli_synapse
  </models/bernoulli_synapse>` instead.

Changed port of NEST Server
~~~~~~~~~~~~~~~~~~~~~~~~~~~

To avoid conflicts with other services, the default port for NEST
Server has been changed from 5000 to 52025.
