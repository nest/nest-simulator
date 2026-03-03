.. _release_3.8:

What's new in NEST 3.8
======================

This page contains a summary of important breaking and non-breaking
changes from NEST 3.7 to NEST 3.8. In addition to the `release notes
on GitHub <https://github.com/nest/nest-simulator/releases/>`_, this
page also contains transition information that helps you to update
your simulation scripts when you come from an older version of NEST.

If you transition from an earlier version, please see our extensive
:ref:`transition guide from NEST 2.x to 3.0 <refguide_2_3>` and the
:ref:`list of updates for previous releases in the 3.x series <whats_new>`.


NMDA dynamics after Brunel and Wang (2001)
------------------------------------------

A new integrate-and-fire model allows for the use
simplified NMDA dynamics.
It contains exponential conductance-based AMPA and GABA synapses along with NMDA synapses
weighted such that it approximates the original linear dynamics.

For futher information see:

* :doc:`/auto_examples/wang_decision_making`
* :doc:`/models/iaf_bw_2001`
* :doc:`/models/iaf_bw_2001_exact`

New "How NEST works" diagram for docs
-------------------------------------

The main page of the docs now contains a diagram highlighting the
fundamental components of creating network simulations in NEST.

* :doc:`/index`


NEST performance benchmarks
---------------------------

You can now view results of this release in benchmarks of
strong and weak scaling at various network sizes in our docs.

* :doc:`NEST performance benchmarks <benchmarks:index>`
