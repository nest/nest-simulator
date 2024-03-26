.. _release_3.5:

What's new in NEST 3.5
======================

This page contains a summary of important breaking and non-breaking
changes from NEST 3.4 to NEST 3.5. In addition to the `release notes
on GitHub <https://github.com/nest/nest-simulator/releases/>`_, this
page also contains transition information that helps you to update
your simulation scripts when you come from an older version of NEST.

If you transition from an earlier version, please see our extensive
:ref:`transition guide from NEST 2.x to 3.0 <refguide_2_3>` and the
:ref:`list of updates for previous releases in the 3.x series
<whats_new>`.


NEST supports the SONATA format
-------------------------------

The SONATA (Scalable Open Network Architecture TemplAte) format provides a framework
for storage and exchange of network models and simulation configurations.

NEST now supports building and simulating networks of point neurons described by
this SONATA format.

See our docs to learn more:

* The :ref:`nest_sonata` for all the details
* An :doc:`example SONATA script <../../../../auto_examples/sonata_example/sonata_network>`
* PyNEST API documentation for the :py:class:`.SonataNetwork` class


Run PyNEST examples as notebooks - installation free
----------------------------------------------------

Using the EBRAINS JupyterHub service, you can now
run the PyNEST examples as Jupyter Notebooks with a click of a button.

No need to install NEST or other packages, the EBRAINS environment has
everything you already need.

Explore the :ref:`pynest_examples` and try it out!

New docs for high performance computing (HPC)
---------------------------------------------

We have new documentation all about optmizing performance of NEST on HPC systems.

Learn about creating a job script, MPI processes and threading. We also have new info on
benchmarking NEST.

Check it out:

* :ref:`optimize_performance`
* :ref:`benchmark`

New model: spike_train_injector
-------------------------------

The :doc:`spike_train_injector <../../../../models/spike_train_injector>` emits spikes at prescribed spike times which are given as an array.

We recommend its use in multi-threaded simulations where spike-emitting neurons, in a somewhat large external population, are modeled on an individual basis.

It was created to prevent an unwanted increase in memory consumption with replication at each virtual process, which
happened when external neurons were modeled as a ``spike_generator``.

