<!-- TOC -->
-   [About NEST](about.md)
-   [Download](download.md)
-   [Features](features.md)
-   [Documentation](documentation.md)
    -   [Installing NEST](installation.md)
    -   [Introduction to PyNEST](introduction-to-pynest.md)
        -   [Part 1: Neurons and simple neural networks](part-1-neurons-and-simple-neural-networks.md)
        -   [Part 2: Populations of neurons](part-2-populations-of-neurons.md)
        -   [Part 3: Connecting networks with synapses](part-3-connecting-networks-with-synapses.md)
        -   [Part 4: Topologically structured networks](part-4-topologically-structured-networks.md)
    -   [Example Networks](examples/examples.md)
    -   [FAQ](frequently_asked_questions.md)
    -   [Developer Manual](http://nest.github.io/nest-simulator/)
-   [Publications](publications.md)
-   [Community](community.md)

<!-- /TOC -->
Features
========

General
-------

-   Python based user interface ([PyNEST](introduction-to-pynest.md))
-   Built-in simulation language interpreter ([SLI](an_introduction_to_sli.md "An Introduction to SLI"))
-   Multi-threading to use multi-processor machines efficiently
-   MPI-parallelism to use computer clusters and super computers

Neuron models
-------------

-   Integrate and fire (IAF) neuron models with current based synapses (delta-, exponential- and alpha-function shaped)
-   Integrate and fire neuron models with conductance-based synapses
-   Adaptive-exponential integrate and fire neuron model (AdEx) ([Brette & Gerstner, 2005](http://jn.physiology.org/cgi/content/abstract/94/5/3637)) - the standard in the FACETS EU project ([[1]](http://facets.kip.uni-heidelberg.de/))
-   MAT2 neuron model ([Kobayashi et al. 2009](http://www.frontiersin.org/computational_neuroscience/10.3389/neuro.10/009.2009/abstract))
-   Hodgkin-Huxley type models with one compartment
-   Neuron models with few compartments

Synapse models
--------------

-   Static synapses
-   Spike-timing dependent plasticity (STDP)
-   Short-term plasticity ([Tsodyks et al. 2000](http://neuro.cjb.net/cgi/content/abstract/20/1/RC50))
-   Neuromodulatory synapses using dopamine

Network models
--------------

-   Topology Module for creating complex networks ([Topology Module User Manual (PDF)](http://www.nest-simulator.org/wp-content/uploads/2014/12/NESTTopologyUserManual.pdf "NESTTopologyUserManual.pdf"))

Interoperability
----------------

-   Interface to the Multi Simulator Coordinator [MUSIC](using_nest_with_music.md "Using NEST with MUSIC")
-   Backend for the simulator-independent modeling tool [PyNN](http://neuralensemble.org/trac/PyNN/)

Accuracy
--------

-   Each neuron model is assigned an appropriate solver
-   [Exact Integration](http://www.springerlink.com/content/08legf57tjkc6nj0/) is used for suitable neuron models
-   By default spikes are restricted to the grid spanned by the computation time step
-   For some neuron models [spike interaction in continuous time](simulations-with-precise-spike-times.md) is available

Verification
------------

-   After installation NEST can be verified by an automatic testsuite
-   The testsuite is automatically run after each modification of the NEST sources. You can watch the current status on our [Continuous Integration](http://www.nest-simulator.org/continuous_integration/ "Continuous Integration") system.

Supported platforms
-------------------

-   Linux
-   Mac OS X (10.3)
-   Windows (under CygWin)
-   HP Tru64 Unix
-   Sun Solaris
-   IBM AIX
-   SGI Altix
-   IBM BlueGene/L/P/Q
