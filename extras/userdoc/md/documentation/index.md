# The Neural Simulation Tool

-   *Download NEST*

    [http://nest-simulator.org/download](download.md)

    Use NEST for your research, modify and improve it

    Current Release [NEST 2.14.0 (Oct 20 2017)](download.md)

-   *Tools for modern computational neuroscience*

    Synaptic plasticity

    Topological network definition

    Precise spike timing

    MUSIC interface

-   *Correctness and release stability*

    A battery of unit tests

    Continuous integration (CI) techniques

    Regular open source releases under the terms of the GPL

## What is NEST

NEST is a simulator for spiking neural network models that focuses on the
dynamics, size and structure of neural systems rather than on the exact
morphology of individual neurons. The development of NEST is coordinated by the
[NEST Initiative](http://www.nest-initiative.org).

NEST is ideal for networks of spiking neurons of any size, for example:

1.  Models of information processing e.g. in the visual or auditory cortex of
    mammals,

2.  Models of network activity dynamics, e.g. laminar cortical networks or
    balanced random networks,

3.  Models of learning and plasticity.

Learn more about NEST:

**\>\> NEST:: documented movie ** **[(short version](http://www.youtube.com/watch?v=K7KXmIv6ROY),**
**[long version)](http://www.youtube.com/watch?v=v0YEiren7D0)\>\> NEST information brochure** **[PDF](http://www.nest-simulator.org/wp-content/uploads/2015/04/JARA_NEST_final.pdf)**

## How do I use NEST

You can use NEST either as a for the interpreted programming language Python
(PyNEST) or as a stand alone application (`nest`).
 PyNEST provides a set of commands to the Python interpreter which give you
 access to NEST's simulation kernel. With these commands, you describe and run
 your network simulation.
 You can also complement PyNEST with [PyNN](http://neuralensemble.org/trac/PyNN),
 a simulator-independent set of Python commands to formulate and run neural
 simulations. While you define your simulations in Python, the actual simulation
 is executed within NEST's highly optimized simulation kernel which is written
 in C++.

A NEST simulation tries to follow the logic of an electrophysiological
experiment that takes place inside a computer with the difference, that the
neural system to be investigated must be defined by the experimenter.

The neural system is defined by a possibly large number of neurons and their
connections. In a NEST network, different neuron and synapse models can coexist.
Any two neurons can have multiple connections with different properties. Thus,
the connectivity can in general not be described by a weight or connectivity
matrix but rather as an adjacency list.

To manipulate or observe the network dynamics, the experimenter can define
so-called devices which represent the various instruments (for measuring and
stimulation) found in an experiment. These devices write their data either to
memory or to file.

NEST is extensible and new models for neurons, synapses, and devices can be
added.

To get started with NEST, please see the [Documentation Page for Tutorials](documentation.md).

## Why should I use NEST

To learn more about the capabilities of NEST, see the [Feature summary](features.md).

1.  NEST provides over 50 neuron models many of which have been published.
    Choose from simple integrate-and-fire neurons with current or conductance
    based synapses, over the Izhikevich or AdEx models, to Hodgkin-Huxley
    models.

2.  NEST provides over 10 synapse models, including short-term plasticity
    (Tsodyks & Markram) and different variants of spike-timing dependent
    plasticity (STDP).

3.  NEST provides many examples that help you getting started with your own
    simulation project.

4.  NEST offers convenient and efficient commands to define and connect large
    networks, ranging from algorithmically determined connections to data-driven
    connectivity.

5.  NEST lets you inspect and modify the state of each neuron and each
    connection at any time during a simulation.

6.  NEST is fast and memory efficient. It makes best use of your multi-core
    computer and compute clusters with minimal user intervention.

7.  NEST runs on a wide range of UNIX-like systems, from MacBooks to
    BlueGene supercomputers.

8.  NEST has minimal dependencies. All it really needs is a C++ compiler.
    Everything else is optional.

9.  NEST developers are using agile [continuous integration](continuous-integration.md)-based
    workflows in order to maintain high code quality standards for correct and
    reproducible simulations.

10. NEST has one of the largest and most experienced developer communities of
    all neural simulators. NEST was first released in 1994 under the name SYNOD
    and has been extended and improved ever since.

11. NEST is open source software and is licensed under the [GNU General Public License v2 or later](http://www.gnu.org/licenses/).

## Please cite NEST and tell us about your work

Please cite NEST if you use it in your work.

If you use NEST 2.16.0, please cite it as **Linssen, Charl et al. (2018).
NEST 2.16.0. Zenodo. 10.5281/zenodo.1400175**. The full citation is available
in different formats on [Zenodo](https://doi.org/10.5281/zenodo.1400175).

If you use NEST 2.14.0, please cite it as **Peyser, Alexander et al. (2017).
NEST 2.14.0. Zenodo. 10.5281/zenodo.882971**. The full citation is available
in different formats on [Zenodo](https://doi.org/10.5281/zenodo.882971).

If you use NEST 2.12.0, please cite it as **Kunkel, Susanne et al. (2017).
NEST 2.12.0. Zenodo. 10.5281/zenodo.259534**. The full citation is available
in different formats on [Zenodo](https://doi.org/10.5281/zenodo.259534).

If you use NEST v2.10.0, please cite it as **Bos, Hannah et al. (2015).
NEST 2.10.0. Zenodo. 10.5281/zenodo.44222**. The full citation is available
in different formats on [Zenodo](https://doi.org/10.5281/zenodo.44222).

If you use NEST v2.8.0, please cite it as **Eppler, Jochen Martin et al. (2015).
NEST 2.8.0. Zenodo. 10.5281/zenodo.32969**. The full citation is available
in different formats on [Zenodo](https://doi.org/10.5281/zenodo.32969).

For all versions below NEST v2.8.0 and for citing NEST without referring to a
specific version, please use: [Gewaltig M-O & Diesmann M (2007) NEST (Neural
Simulation Tool) Scholarpedia
2(4):1430](http://www.scholarpedia.org/article/NEST_(Neural_Simulation_Tool)).

Here is suitable BibTeX entry:

    @ARTICLE{Gewaltig:NEST,
      author = {Marc-Oliver Gewaltig and Markus Diesmann},
      title = {NEST (NEural Simulation Tool)},
      journal = {Scholarpedia},
      year = {2007},
      volume = {2},
      pages = {1430},
      number = {4}
    }

Please get in touch with us about your publications that used NEST, we will
add them to our [publication list](publications.md), thus making them visible
to potential readers.

## NEST logo for your poster or presentation

If you like NEST, why not show it on your poster or on your slides?

<https://github.com/nest/nest-simulator/tree/master/extras/logos>

<img src="../../../logos/nest-simulated.png" width="480">

## Nest Models on [Open Source Brain](http://www.opensourcebrain.org/)

-   [Connection Set Algebra Showcase](http://www.opensourcebrain.org/projects/44)
-   [Network models of V1](http://www.opensourcebrain.org/projects/111)
-   [Self Sustained Network Activity - Destexhe 2009](http://www.opensourcebrain.org/projects/31)
