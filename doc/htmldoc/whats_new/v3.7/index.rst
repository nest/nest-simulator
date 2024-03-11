.. _release_3.7:

What's new in NEST 3.7
======================

This page contains a summary of important breaking and non-breaking
changes from NEST 3.6 to NEST 3.7. In addition to the `release notes
on GitHub <https://github.com/nest/nest-simulator/releases/>`_, this
page also contains transition information that helps you to update
your simulation scripts when you come from an older version of NEST.

If you transition from an earlier version, please see our extensive
:ref:`transition guide from NEST 2.x to 3.0 <refguide_2_3>` and the
:ref:`list of updates for previous releases in the 3.x series <whats_new>`.


NEST requires C++17
-------------------

From NEST 3.7 on, we use some C++17 features in NEST code. Therefore,
NEST needs to be built with a compiler that supports C++17. Most
recent C++ compilers should do so.

Tripartite connectivity in NEST
-------------------------------

NEST now supports creation of connections involving three populations
of neurons: a pre-synaptic, a post-synaptic and a third-factor
population. At present, as single tripartite connection rule is
available, ``tripartite_bernoulli_with_pool``. Tripartite connections
are created with the new :py:func:`.TripartiteConnect` function. The first
use case for tripartite connections are networks containing astrocyte
populations.

See examples using astrocyte models:

* :doc:`../../../auto_examples/astrocytes/astrocyte_small_network`
* :doc:`../../../auto_examples/astrocytes/astrocyte_brunel`

See connectivity documentation:

* :ref:`tripartite_connectivity`


E-prop plasticity in NEST
-------------------------

Another new NEST feature is eligibility propagation (e-prop) [1]_, a local and
online learning algorithm for recurrent spiking neural networks (RSNNs) that
serves as a biologically plausible approximation to backpropagation through time
(BPTT). It relies on eligibility traces and neuron-specific learning signals to
compute gradients without the need for error propagation backward in time. This
approach aligns with the brain's learning mechanisms and offers a strong
candidate for efficient training of RSNNs in low-power neuromorphic hardware.

For further information, see:

* :doc:`/auto_examples/eprop_plasticity/index`
* :doc:`/models/index_e-prop plasticity`

.. [1] Bellec G, Scherr F, Subramoney F, Hajek E, Salaj D, Legenstein R,
       Maass W (2020). A solution to the learning dilemma for recurrent
       networks of spiking neurons. Nature Communications, 11:3625.
       https://doi.org/10.1038/s41467-020-17236-y
