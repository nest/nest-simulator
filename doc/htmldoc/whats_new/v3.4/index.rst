.. _release_3.4:

What's new in NEST 3.4
======================

This page contains a summary of important breaking and non-breaking changes
from NEST 3.3 to NEST 3.4. In addition to the `release
notes on GitHub <https://github.com/nest/nest-simulator/releases/>`_,
this page also contains transition information that helps you to
update your simulation scripts when you come from an older version of
NEST.

If you transition from a version earlier than 3.3, please see our
extensive :ref:`transition guide from NEST 2.x to 3.0
<refguide_2_3>` or :ref:`release updates for previous releases in 3.x <whats_new>`.

Documentation restructuring and new theme
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

NEST documentation has a new theme! We did a major overhaul of the layout and structure of the documentation.
The changes aim to improve findability and access of content. With a more modern 
layout, our wide range of docs can be discovered more easily. 
The table of contents is simplified and the content is grouped based on topic (neurons, synapses etc)
rather than type of documentation (e.g., 'guides').


Changes in NEST behavior
~~~~~~~~~~~~~~~~~~~~~~~~

Inferred extent of spatial layers with freely placed neurons
............................................................

Spatial layers can be created by specifying only the node positions using ``spatial.free``,
without explicitly specifying the ``extent``. 
In that case, in NEST 3.4 and later, the ``extent`` will be determined by the position of the
lower-leftmost and upper-rightmost nodes in the layer; earlier versions of NEST added a hard-coded
padding to the extent. The ``center`` is computed as the midpoint between the lower-leftmost and
upper-rightmost nodes.

When creating a layer with only a single node, the ``extent`` has to be specified explicitly.


Deprecation information
~~~~~~~~~~~~~~~~~~~~~~~

* Model ``spike_dilutor`` is now deprecated and can only be used
  in single-threaded mode. To implement connections which transmit
  spikes with fixed probability, use ``bernoulli_synapse`` instead.
