.. _related_projects:

Related projects
================

NEST Simulator is part of a larger network of projects that focus on simulation, analysis, visualization, or modeling of
biologically realistic neural networks.

Here you can find further information about some of these projects.


NESTML
------

:doc:`NESTML <nestml:index>` allows you to modify and create models for NEST in a simplified format.

It is a domain-specific language that supports the specification of neuron and synapse
models in a precise and concise syntax, based on the syntax of Python. Model equations can either be given as a simple
string of mathematical notation or as an algorithm written in the built-in procedural language. The equations are
analyzed by the associated toolchain, written in Python, to compute an exact solution if possible or use an appropriate
numeric solver otherwise.

* :doc:`Get started with NESTML <nestml:tutorials/tutorials_list>`

* :doc:`List of available models <nestml:models_library/index>`


NEST extension module
---------------------

:doc:`The NEST extension module <extmod:index>` allows you to extend the functionality of NEST
without messing with the source code of NEST itself. It makes sharing custom extensions with other researchers easy.

* :doc:`Get started with the extension module <extmod:extension_modules>`

NEST desktop
------------

:doc:`NEST Desktop <desktop:index>` is a web-based GUI application for the NEST Simulator.
The app enables the rapid construction, parametrization, and instrumentation of neuronal network models.

* :doc:`Get started with NEST desktop <desktop:user/index>`

NEST GPU
--------

NEST GPU is a GPU-MPI library for simulation of large-scale networks of spiking neurons. It can be used in Python, in C++ and in C.

* :doc:`Get started with NEST GPU <gpu:index>`

PyNN
----

:doc:`PyNN <pynn:index>` is a simulator-independent language for building neuronal network models.

In other words, you can write the code for a model once, using the PyNN API and the Python programming language, and
then run it without modification on any simulator that PyNN supports (currently NEURON, NEST, and Brian) and on a
number of neuromorphic hardware systems.

* :doc:`Get started with PyNN <pynn:building_networks>`

Elephant
--------

:doc:`Elephant (Electrophysiology Analysis Toolkit) <elephant:index>` is an open-source, community-centered
library for the analysis of electrophysiological data in the Python programming language.

* :doc:`Get started with Elephant <elephant:tutorials>`

----

Arbor
-----

:doc:`Arbor <arbor:index>` is a high-performance library for computational neuroscience simulations with
multi-compartment, morphologically-detailed cells, from single cell models to very large networks

* :doc:`Get started with Arbor <arbor:tutorial/index>`

Neuromorphic hardware
---------------------

:doc:`SpiNNaker and BrainScaleS <neuromorph:index>` are neuromorphic computing systems, which enable
energy-efficient, large-scale neuronal network simulations with simplified spiking neuron models.
The BrainScaleS system is based on physical (analog) emulations of neuron models and offers highly accelerated
operation (:math:`10^4` x real time). The SpiNNaker system is based on a digital many-core architecture and provides
real-time operation.

* :doc:`Get started with SpiNNaker <neuromorph:mc/mc_index>`
* :doc:`Get started with BrainScaleS <neuromorph:pm/pm>`

TheVirtualBrain (TVB)
---------------------

:ref:`TVB <tvb:top_basic>` is a framework for the simulation of the dynamics of large-scale brain
networks with biologically realistic connectivity.

* :ref:`Get started with TVB <tvb:tutorial_0_gettingstarted>`

ConnPlotter
-----------

The ConnPlotter package allows you to plot connection matrices from NEST.

*  `Get started with ConnPlotter <https://github.com/nest/connplotter>`_
