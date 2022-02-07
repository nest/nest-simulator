# The Neural Simulation Tool - NEST

[![Documentation](https://img.shields.io/readthedocs/nest-simulator?logo=readthedocs&logo=Read%20the%20Docs&label=Documentation)](https://nest-simulator.org/documentation)
[![CII Best Practices](https://bestpractices.coreinfrastructure.org/projects/2218/badge)](https://bestpractices.coreinfrastructure.org/projects/2218)
[![License](http://img.shields.io/:license-GPLv2+-green.svg)](http://www.gnu.org/licenses/gpl-2.0.html)
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.5886894.svg)](https://doi.org/10.5281/zenodo.5886894)

[![Latest release](https://img.shields.io/github/release/nest/nest-simulator.svg?color=brightgreen&label=latest%20release)](https://github.com/nest/nest-simulator/releases)
[![GitHub contributors](https://img.shields.io/github/contributors/nest/nest-simulator?logo=github)](https://github.com/nest/nest-simulator)
[![GitHub commit activity](https://img.shields.io/github/commit-activity/y/nest/nest-simulator?logo=github&color=%23ff6633)](https://github.com/nest/nest-simulator)

[![Ubuntu version](https://img.shields.io/badge/ubuntu-(PPA)-blue?logo=debian)](https://nest-simulator.readthedocs.io/en/latest/installation/)
[![Fedora package](https://img.shields.io/fedora/v/nest?logo=fedora)](https://src.fedoraproject.org/rpms/nest)
[![Conda version](https://img.shields.io/conda/vn/conda-forge/nest-simulator.svg?logo=conda-forge&logoColor=white)](https://anaconda.org/conda-forge/nest-simulator)
[![Homebrew version](https://img.shields.io/homebrew/v/nest.svg?logo=apple)](https://formulae.brew.sh/formula/nest)
[![Docker Image Version](https://img.shields.io/docker/v/nestsim/nest?label=docker&sort=semver&logo=docker&logoColor=white)](https://hub.docker.com/r/nestsim/nest)
[![Virtual applicance](https://img.shields.io/badge/VM-v3.1-blue?logo=CodeSandbox)](https://nest-simulator.readthedocs.io/en/latest/download.html#download-livemedia)

[![YouTube Video Views](https://img.shields.io/youtube/views/K7KXmIv6ROY?style=social)](https://www.youtube.com/results?search_query=nest-simulator+neurons)
[![Twitter Follow](https://img.shields.io/twitter/follow/nestsimulator?style=social)](https://twitter.com/nestsimulator)

NEST is a simulator for spiking neural network models that focuses on the
dynamics, size and structure of neural systems rather than on the exact
morphology of individual neurons. The development of NEST is coordinated by the
NEST Initiative. General information on the NEST Initiative can be found at
its homepage at https://www.nest-initiative.org.

NEST is ideal for networks of spiking neurons of any size, for example:

- Models of information processing e.g. in the visual or auditory cortex of
  mammals,
- Models of network activity dynamics, e.g. laminar cortical networks or
  balanced random networks,
- Models of learning and plasticity.

For copyright information please refer to the `LICENSE` file and to the
information header in the source files.

## How do I use NEST?

You can use NEST either via Python (PyNEST) or as a stand-alone application
(nest). PyNEST provides a set of commands to the Python interpreter which give
you access to NEST's simulation kernel. With these commands, you describe and
run your network simulation. You can also complement PyNEST with PyNN, a
simulator-independent set of Python commands to formulate and run neural
simulations. While you define your simulations in Python, the actual simulation
is executed within NEST's highly optimized simulation kernel which is written
in C++.

A NEST simulation tries to follow the logic of an electrophysiological
experiment that takes place inside a computer with the difference, that the
neural system to be investigated must be defined by the experimenter.

The neural system is defined by a possibly large number of neurons and their
connections. In a NEST network, different neuron and synapse models can
coexist. Any two neurons can have multiple connections with different
properties. Thus, the connectivity can in general not be described by a weight
or connectivity matrix but rather as an adjacency list.

To manipulate or observe the network dynamics, the experimenter can define
so-called devices which represent the various instruments (for measuring and
stimulation) found in an experiment. These devices write their data either to
memory or to file.

NEST is extensible and new models for neurons, synapses, and devices can be
added.

To get started with NEST, please see the [Documentation Page for
Tutorials](https://www.nest-simulator.org/documentation/).

## Why should I use NEST?

To learn more about the capabilities of NEST, please read the complete [feature
summary](https://www.nest-simulator.org/features/).

- NEST provides over 50 neuron models many of which have been published. Choose
  from simple integrate-and-fire neurons with current or conductance based
  synapses, over the Izhikevich or AdEx models, to Hodgkin-Huxley models.
- NEST provides over 10 synapse models, including short-term plasticity
  (Tsodyks & Markram) and different variants of spike-timing dependent
  plasticity (STDP).
- NEST provides many examples that help you getting started with your own
  simulation project.
- NEST offers convenient and efficient commands to define and connect large
  networks, ranging from algorithmically determined connections to data-driven
  connectivity.
- NEST lets you inspect and modify the state of each neuron and each connection
  at any time during a simulation.
- NEST is fast and memory efficient. It makes best use of your multi-core
  computer and compute clusters with minimal user intervention.
- NEST runs on a wide range of UNIX-like systems, from MacBooks to BlueGene
  supercomputers.
- NEST has minimal dependencies. All it really needs is a C++ compiler.
  Everything else is optional.
- NEST developers are using agile continuous integration-based workflows in
  order to maintain high code quality standards for correct and reproducible
  simulations.
- NEST has one of the largest and most experienced developer communities of all
  neural simulators. NEST was first released in 1994 under the name SYNOD and
  has been extended and improved ever since.

## License

NEST is open source software and is licensed under the [GNU General Public
License v2](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html) or
later.

## Installing NEST

Please see the online [NEST Installation Instructions](http://www.nest-simulator.org/installation)
to find out how to install NEST.

## Getting help

- You can run the `help` command in the NEST interpreter to find documentation
  and learn more about available commands.
- For queries regarding NEST usage, please use the [NEST users mailing
  list](https://www.nest-initiative.org/mailinglist/).
- Information on the Python bindings to NEST can be found in
  `${prefix}/share/doc/nest/README.md`.
- For those looking to extend NEST, developer documentation on [Contributing to
  NEST](https://nest-simulator.readthedocs.io/en/latest/contribute/index.html) is available.

## Citing NEST

Please cite NEST if you use it in your work.

- You can find all the information for [citing NEST here](https://nest-simulator.readthedocs.io/en/latest/citing-nest.html)


