# Features

## General

-   Python based user interface ([PyNEST](introduction-to-pynest.md))
-   Built-in simulation language interpreter ([SLI](an-introduction-to-sli.md))
-   Multi-threading to use multi-processor machines efficiently
-   MPI-parallelism to use computer clusters and super computers

## Neuron models

-   Integrate and fire (IAF) neuron models with current based synapses (delta-,
    exponential- and alpha-function shaped)

-   Integrate and fire neuron models with conductance-based synapses

-   Adaptive-exponential integrate and fire neuron model (AdEx)
    ([Brette & Gerstner, 2005](http://jn.physiology.org/cgi/content/abstract/94/5/3637))-
    the standard in the FACETS EU project ([[1]](http://facets.kip.uni-heidelberg.de/))

-   MAT2 neuron model ([Kobayashi et al. 2009](http://www.frontiersin.org/computational_neuroscience/10.3389/neuro.10/009.2009/abstract))

-   Hodgkin-Huxley type models with one compartment

-   Neuron models with few compartments

## Synapse models

-   Static synapses
-   Spike-timing dependent plasticity (STDP)
-   Short-term plasticity ([Tsodyks et al. 2000](http://neuro.cjb.net/cgi/content/abstract/20/1/RC50))
-   Neuromodulatory synapses using dopamine

## Network models

-   Topology Module for creating complex networks ([Topology Module User Manual (PDF)](http://www.nest-simulator.org/wp-content/uploads/2014/12/NESTTopologyUserManual.pdf))

## Interoperability

-   Interface to the Multi Simulator Coordinator [MUSIC](using-nest-with-music.md)
-   Backend for the simulator-independent modeling tool [PyNN](http://neuralensemble.org/trac/PyNN/)

## Accuracy

-   Each neuron model is assigned an appropriate solver

-   [Exact Integration](http://www.springerlink.com/content/08legf57tjkc6nj0/)
    is used for suitable neuron models

-   By default spikes are restricted to the grid spanned by the computation time
    step

-   For some neuron models [spike interaction in continuous time](simulations-with-precise-spike-times.md)
    is available

## Verification

-   After installation NEST can be verified by an automatic testsuite

-   The testsuite is automatically run after each modification of the NEST
    sources. You can watch the current status on our [Continuous Integration](continuous-integration.md) system.

## Supported platforms

-   Linux
-   Mac OS X
-   Virtual machines for use under Windows
-   IBM BlueGene

By support we mean that we regularly test and use NEST on recent versions of
these systems and that NEST therefore should work on those systems. It should
not be construed as any warranty that NEST will run on any particular system.
