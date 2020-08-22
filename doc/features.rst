Features
========

1.  NEST provides over 50 neuron models many of which have been
    published. Choose from simple integrate-and-fire neurons with
    current or conductance based synapses, over the Izhikevich or AdEx
    models, to Hodgkin-Huxley models.

2.  NEST provides over 10 synapse models, including short-term
    plasticity (Tsodyks & Markram) and different variants of
    spike-timing dependent plasticity (STDP).

3.  Nest provides a Topology Module for creating complex networks
    (`Topology Module User Manual
    (PDF) <https://www.nest-simulator.org/wp-content/uploads/2014/12/NESTTopologyUserManual.pdf>`__)

4.  NEST provides many examples that help you getting started with your
    own simulation project.

5.  NEST offers convenient and efficient commands to define and connect
    large networks, ranging from algorithmically determined connections
    to data-driven connectivity.

6.  NEST lets you inspect and modify the state of each neuron and each
    connection at any time during a simulation.

7.  NEST is fast and memory efficient. It makes best use of your
    multi-core computer and compute clusters with minimal user
    intervention.

8.  NEST runs on a wide range of UNIX-like systems, from MacBooks to
    BlueGene supercomputers.

9.  NEST has minimal dependencies. All it really needs is a C++
    compiler. Everything else is optional.

10. NEST developers are using agile `continuous
    integration <continuous-integration.md>`__-based workflows in order
    to maintain high code quality standards for correct and reproducible
    simulations.

11. NEST has one of the largest and most experienced developer
    communities of all neural simulators. NEST was first released in
    1994 under the name SYNOD and has been extended and improved ever
    since.

12. NEST is open source software and is licensed under the `GNU General
    Public License v2 or later <http://www.gnu.org/licenses/>`__.

General Features
----------------

-  Python based user interface (`PyNEST <introduction-to-pynest.md>`__)
-  Built-in simulation language interpreter
   (`SLI <an-introduction-to-sli.md>`__)
-  Multi-threading to use multi-processor machines efficiently
-  MPI-parallelism to use computer clusters and super computers

Neuron models
-------------

-  Integrate and fire (IAF) neuron models with current based synapses
   (delta-, exponential- and alpha-function shaped)

-  Integrate and fire neuron models with conductance-based synapses

-  Adaptive-exponential integrate and fire neuron model (AdEx) (`Brette
   & Gerstner,
   2005 <http://jn.physiology.org/cgi/content/abstract/94/5/3637>`__)-
   the standard in the FACETS EU project
   (`[1] <http://facets.kip.uni-heidelberg.de/>`__)

-  MAT2 neuron model (`Kobayashi et al.
   2009 <http://www.frontiersin.org/computational_neuroscience/10.3389/neuro.10/009.2009/abstract>`__)

-  Hodgkin-Huxley type models with one compartment

-  Neuron models with few compartments

Synapse models
--------------

-  Static synapses
-  Spike-timing dependent plasticity (STDP)
-  Short-term plasticity (`Tsodyks et al.
   2000 <http://neuro.cjb.net/cgi/content/abstract/20/1/RC50>`__)
-  Neuromodulatory synapses using dopamine

Network models
--------------

-  Topology Module for creating complex networks (`Topology Module User
   Manual
   (PDF) <https://www.nest-simulator.org/wp-content/uploads/2014/12/NESTTopologyUserManual.pdf>`__)

Interoperability
----------------

-  Interface to the Multi Simulator Coordinator
   `MUSIC <using-nest-with-music.md>`__
-  Backend for the simulator-independent modeling tool
   `PyNN <http://neuralensemble.org/PyNN/>`__

Accuracy
--------

-  Each neuron model is assigned an appropriate solver

-  `Exact
   Integration <http://www.springerlink.com/content/08legf57tjkc6nj0/>`__
   is used for suitable neuron models

-  By default spikes are restricted to the grid spanned by the
   computation time step

-  For some neuron models `spike interaction in continuous
   time <simulations-with-precise-spike-times.md>`__ is available

Verification
------------

-  After installation NEST can be verified by an automatic testsuite

-  The testsuite is automatically run after each modification of the
   NEST sources. You can watch the current status on our `Continuous
   Integration <continuous-integration.md>`__ system.

Supported platforms
-------------------

-  Linux
-  Mac OS X
-  Virtual machines for use under Windows
-  IBM BlueGene

By support we mean that we regularly test and use NEST on recent
versions of these systems and that NEST therefore should work on those
systems. It should not be construed as any warranty that NEST will run
on any particular system.
