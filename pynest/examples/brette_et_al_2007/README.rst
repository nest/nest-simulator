Brette et al. 2007 Benchmarks
==============================

This directory contains Python implementations of the benchmarks from the
FACETS simulator review (Brette et al. 2007) [2]_. These benchmarks are based
on the Vogels & Abbott network model [1]_ and are designed to test different
aspects of neural network simulation:

- **Benchmark 1 (COBA)**: Conductance-based synapses with integrate-and-fire neurons
- **Benchmark 2 (CUBA)**: Current-based synapses with integrate-and-fire neurons
- **Benchmark 3 (HH-COBA)**: Conductance-based synapses with Hodgkin-Huxley neurons
- **Benchmark 4 (CUBA-PS)**: Current-based synapses with precise spiking
- **Benchmark 5 (CUBA-STDP)**: Current-based synapses with STDP plasticity

All benchmarks create sparsely coupled networks of excitatory and inhibitory
neurons which exhibit self-sustained activity after an initial stimulus.
Connections within and across both populations are created at random. Both
neuron populations receive Poissonian background input.

Files
-----

- ``brette_et_al_2007_benchmark.py``: Shared framework module containing common
  network building and simulation functions
- ``coba.py``: Benchmark 1 - Conductance-based synapses (COBA)
- ``cuba.py``: Benchmark 2 - Current-based synapses (CUBA)
- ``hh_coba.py``: Benchmark 3 - Hodgkin-Huxley neurons (HH-COBA)
- ``cuba_ps.py``: Benchmark 4 - Precise spiking (CUBA-PS)
- ``cuba_stdp.py``: Benchmark 5 - STDP plasticity (CUBA-STDP)

Usage
-----

Each benchmark can be run directly as a Python script. For example::

    python coba.py

Or from the examples directory::

    python -m brette_et_al_2007.coba

The benchmarks will print a summary including:
- Number of neurons and synapses
- Average firing rates for excitatory and inhibitory populations
- Building and simulation times

Benchmark Details
-----------------

**Benchmark 1 (COBA)**
  - Neuron model: ``iaf_cond_exp``
  - Synapse model: conductance-based (exponential)
  - Spike times: grid-constrained
  - Simulation time: 1000 ms

**Benchmark 2 (CUBA)**
  - Neuron model: ``iaf_psc_exp``
  - Synapse model: current-based (exponential)
  - Spike times: grid-constrained
  - Simulation time: 10000 ms (10x longer due to lower computational load)

**Benchmark 3 (HH-COBA)**
  - Neuron model: ``hh_cond_exp_traub``
  - Synapse model: conductance-based (exponential)
  - Spike times: grid-constrained
  - Simulation time: 1000 ms

**Benchmark 4 (CUBA-PS)**
  - Neuron model: ``iaf_psc_delta``
  - Synapse model: current-based (delta/voltage jump)
  - Spike times: off-grid (precise spiking)
  - Simulation time: 10000 ms

**Benchmark 5 (CUBA-STDP)**
  - Neuron model: ``iaf_psc_exp``
  - Synapse model: STDP (E->E), static current (others)
  - Spike times: grid-constrained
  - Simulation time: 2000 ms
  - Features: Random initial weights and membrane potentials, STDP plasticity

References
----------

.. [1] Vogels TP, Abbott LF. 2005. Signal propagation and logic gating in
       networks of integrate-and-fire neurons. Journal of Neuroscience.
       25(46):10786-10795.
       https://doi.org/10.1523/JNEUROSCI.3508-05.2005

.. [2] Brette R, Rudolph M, Carnevale T, Hines M, Beeman D, Bower JM, et al.
       2007. Simulation of networks of spiking neurons: a review of tools and
       strategies. Journal of Computational Neuroscience. 23(3):349-398.
       https://doi.org/10.1007/s10827-007-0038-6
