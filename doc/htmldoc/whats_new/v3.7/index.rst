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

E-prop plasticity in NEST
-------------------------

Another new NEST feature is eligibility propagation (e-prop) [1]_, a local and
online learning algorithm for recurrent spiking neural networks (RSNNs) that is
biologically plausible and approaches the performance of backpropagation through
time (BPTT). It relies on eligibility traces and neuron-specific learning
signals to compute gradients without the need for error propagation backward in
time. This approach aligns with the brain's learning mechanisms and offers a
strong candidate for efficient training of RSNNs in low-power neuromorphic
hardware.

For further information, see:

* :doc:`/auto_examples/eprop_plasticity/index`
* :doc:`/models/eprop_iaf_adapt_bsshslm_2020`
* :doc:`/models/eprop_iaf_bsshslm_2020`
* :doc:`/models/eprop_learning_signal_connection_bsshslm_2020`
* :doc:`/models/eprop_readout_bsshslm_2020`
* :doc:`/models/eprop_synapse_bsshslm_2020`



Connectivity concepts
---------------------

The documentation on :ref:`connectivity_concepts` now serves as a living reference for the connection rules defined in the article "Connectivity concepts in neuronal network modeling" [2]_.

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

New connection rule: ``pairwise_poisson``
------------------------------------------

The number of synapses between pre- and post-synaptic neurons is drawn from a Poisson distribution.
The ``pairwise_poisson`` method is adapted from the ``pairwise bernouilli`` method.


See more information:

* :ref:`pairwise_poisson`


Ignore-and-fire neuron model
----------------------------

A neuron model for generating spikes at fixed intervals, irrespective of inputs.

The ``ignore_and_fire`` neuron is primarily used for neuronal-network model verification and validation purposes
("benchmarking"), in particular, to evaluate the correctness and performance of connectivity generation and inter-neuron
communication. It permits an easy scaling of the network size and/or connectivity without affecting the output spike
statistics.

See documentation for more information:

* :doc:`/models/ignore_and_fire`

Neuron model with integrated short-term plasticity
--------------------------------------------------

The new ``iaf_tum_2000`` neuron model incoroporates the ``tsodyks_synapse`` directly
into the neuron model. In particular,
``iaf_tum_2000`` implements short-term depression and short-term facilitation based on Tsodyks et al. [3]_.
It is based on the ``iaf_psc_exp`` model.


New parameter for compartmental model
-------------------------------------

In the compartmental model ``cm_default``, the voltage initialisation of each compartment
can now be specified by the user, by adding a `v_comp` entry to the compartment parameter dictionary.

See the model documentation:

* :doc:`/models/cm_default`

New interface for NEST Extension Modules
----------------------------------------

The interface for NEST Extension Modules has been thoroughly revised. Key changes are

* All extention modules must be derived from class ``nest::NESTExtensionInterface`` provided in ``nest_extension_interface.h``.

    * In your class, you must override the ``initialize()`` method with one that registers all components provided by your module.
    * The ``nest_extension_interface.h`` header provides a large set of NEST includes, so once including it you should no longer need to include other NEST headers in most cases.

* Modules are now unloaded upon ``ResetKernel()`` and new versions of modules can be loaded after ``ResetKernel()``.
* Modules can now also register connection builders and growth curves with the kernel.

For more information, see the extension module documentation:

* :doc:`NEST Extension Module Documentation <extmod:index>`

NEST requires C++17
-------------------

From NEST 3.7 on, we use some C++17 features in NEST code. Therefore,
NEST needs to be built with a compiler that supports C++17. Most
recent C++ compilers should do so.

References
----------

.. [1] Bellec G, Scherr F, Subramoney F, Hajek E, Salaj D, Legenstein R,
       Maass W (2020). A solution to the learning dilemma for recurrent
       networks of spiking neurons. Nature Communications, 11:3625.
       https://doi.org/10.1038/s41467-020-17236-y

.. [2] Senk J, Kriener B, Djurfeldt M, Voges N, Jiang HJ, et al. (2022) Connectivity concepts in neuronal network modeling. PLOS Computational Biology 18(9): e1010086. https://doi.org/10.1371/journal.pcbi.1010086

.. [3] Tsodyks M, Uziel A, Markram H (2000). Synchrony generation in recurrent
       networks with frequency-dependent synapses. Journal of Neuroscience,
       20 RC50. URL: http://infoscience.epfl.ch/record/183402
