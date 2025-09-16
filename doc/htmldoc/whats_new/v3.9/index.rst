.. _release_3.9:

What's new in NEST 3.9
======================

This page contains a summary of important breaking and non-breaking
changes from NEST 3.8 to NEST 3.9. In addition to the `release notes
on GitHub <https://github.com/nest/nest-simulator/releases/>`_, this
page also contains transition information that helps you to update
your simulation scripts when you come from an older version of NEST.

If you transition from an earlier version, please see our extensive
:ref:`transition guide from NEST 2.x to 3.0 <refguide_2_3>` and the
:ref:`list of updates for previous releases in the 3.x series <whats_new>`.

Model improvements
------------------

* Biologically-inspired features added to e-prop plasticity

  See :doc:`/auto_examples/eprop_plasticity/index`

* The tripartite connectivity, used for Astrocytes, now supports multiple primary
  connection rules.

  See :doc:`Astrocyte examples </auto_examples/astrocytes/index>`

Documentation additions
-----------------------

* We now have a detailed description of neuron models in NEST, inclduding :ref:`update
  algorithm <neuron_update>` and :ref:`neuron types <types_neurons>`.

* We updated the :ref:`Built-in timers <built_in_timers>` section to include a graphical description of a simulation run
  focusing on timers.

* For Windows users, we added a :ref:`helpful guide <windows_install>` to get NEST running on that platform with Docker.
