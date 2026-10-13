# -*- coding: utf-8 -*-
#
# part_1_neurons_and_simple_neural_networks.py
#
# This file is part of NEST.
#
# Copyright (C) 2004 The NEST Initiative
#
# NEST is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# NEST is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with NEST.  If not, see <http://www.gnu.org/licenses/>.

"""
.. _pynest_tutorial_1:

Part 1: Neurons and simple neural networks
==========================================

In this section we cover the first steps in using PyNEST to simulate
neuronal networks. When you have worked through this material, you will
know how to:

-  start PyNEST
-  create neurons and stimulation or recording devices
-  query and set their parameters
-  connect them to each other or to devices
-  simulate the network
-  extract the data from recording devices

For more information on the usage of PyNEST, please see the other
sections of this primer:

-  :ref:`Part 2: Populations of neurons <pynest_tutorial_2>`
-  :ref:`Part 3: Connecting networks with synapses <pynest_tutorial_3>`
-  :ref:`Part 4: Spatially structured networks <pynest_tutorial_4>`

More advanced examples can be found at :ref:`Example Networks
<pynest_examples>`, or have a look at the source directory of your NEST
installation in the subdirectory ``pynest/examples/``.
"""

###############################################################################
# PyNEST - an interface to the NEST Simulator
# -------------------------------------------
#
# .. figure:: /static/img/python_interface_ng.svg
#    :alt: Python Interface
#
#    The Python interpreter imports NEST as a package, which loads the Cython
#    extension ``nestkernel_api``, a direct binding to the NEST C++ API. This
#    C++ API provides the interface to the NEST kernel, using dedicated
#    managers for specific functions alongside classes that represent neuron,
#    device, and synapse models. When a user runs a simulation script
#    (``my-simulation.py``), its calls to the PyNEST high-level API are
#    forwarded through ``ll_api.py`` and the Cython layer directly to the C++
#    KernelManager, which controls the simulation.
#
# The NEural Simulation Tool (NEST: https://www.nest-initiative.org) is
# designed for the simulation of large heterogeneous networks of point
# neurons. It is open source software released under the GPL licence. The
# simulator comes with an interface to Python. The simulation kernel is
# written in C++ to obtain the highest possible performance for the
# simulation.
#
# You can use PyNEST interactively from the Python prompt or from within
# IPython/Jupyter. This is very helpful when you are exploring PyNEST, trying
# to learn a new functionality or debugging a routine. Once out of the
# exploratory mode, you will find it saves a lot of time to write your
# simulations in text files. These can in turn be run from the command line
# or from the Python or IPython prompt.
#
# Whether working interactively, semi-interactively, or purely executing
# scripts, the first thing that needs to happen is importing NEST's
# functionality into the Python interpreter. We also import Matplotlib, which
# we use to display the results, and reset the kernel so the notebook can be
# run repeatedly without interference from previous simulations.

import matplotlib.pyplot as plt
import nest

nest.set_verbosity("M_WARNING")
nest.ResetKernel()

###############################################################################
# In case other Python packages are required, such as `scikit-learn
# <http://scikit-learn.org/stable/index.html>`_ and `SciPy
# <https://www.scipy.org/>`_, they should be imported *before* NEST, e.g.::
#
#     import sklearn
#     import scipy
#     import nest
#
# As with every other module for Python, the available functions can be
# listed with ``dir(nest)``. If you want more information about a particular
# command, you may use Python's standard help system, which returns the help
# text (docstring) explaining its use. You can get the help page for a NEST
# object (such as a synapse or neuron model) using ``nest.help('iaf_psc_alpha')``.

###############################################################################
# Creating nodes
# --------------
#
# A neural network in NEST consists of two basic element types: nodes and
# connections. Nodes are either neurons, devices or sub-networks. Devices are
# used to stimulate neurons or to record from them. For now we will work with
# the default network structure of NEST.
#
# New nodes are created with the command :py:func:`.Create`, which takes as
# arguments the model name of the desired node type, and optionally the number
# of nodes to be created and the initialising parameters. The function returns
# a :py:class:`.NodeCollection` of handles to the new nodes, which you can
# assign to a variable for later use.
#
# As a first example, we will create a neuron of type ``iaf_psc_alpha``. This
# is an integrate-and-fire neuron with alpha-shaped postsynaptic currents. The
# function returns a NodeCollection of the ids of all the created neurons, in
# this case only one, which we store in a variable called ``neuron``.

neuron = nest.Create("iaf_psc_alpha")

###############################################################################
# We can now use the NodeCollection to access the properties of this neuron.
# Properties of nodes in NEST are generally accessed via Python dictionaries of
# key-value pairs of the form ``{key: value}``. To see all the properties a
# neuron has, you may ask it for its status with ``neuron.get()``. Many of
# these properties are not relevant for the dynamics of the neuron. If you
# already know which properties you are interested in, you can specify a key,
# or a list of keys, as an optional argument to
# :py:meth:`~.NodeCollection.get`. Here we query the value of the constant
# background current :hxt_ref:`I_e`; the result is a floating point number.

neuron.get("I_e")

###############################################################################
# Next we query the reset potential and threshold of the neuron, and receive
# the result as a dictionary.

neuron.get(["V_reset", "V_th"])

###############################################################################
# To modify the properties in the dictionary, we use
# :py:meth:`~.NodeCollection.set`. In the following example, the background
# current is set to 376.0 pA, a value causing the neuron to spike periodically.

neuron.set(I_e=376.0)

###############################################################################
# Note that NEST is type sensitive: if a particular property is of type
# ``double``, then you do need to explicitly write the decimal point. For
# example, ``neuron.set({"I_e": 376})`` (an integer) would result in an error.
# This conveniently protects us from making integer division errors, which are
# hard to catch.
#
# Another way of setting and getting parameters is to access them directly as
# attributes of the NodeCollection::
#
#     neuron.I_e = 376.0
#     neuron.I_e

###############################################################################
# Next we create a :hxt_ref:`multimeter`, a *device* we can use to record the
# membrane voltage of a neuron over time. The property ``record_from`` expects
# a list of the names of the variables we would like to record. The variables
# exposed to the multimeter vary from model to model. For a specific model, you
# can check the names of the exposed variables by looking at the neuron's
# property ``recordables``.

multimeter = nest.Create("multimeter")
multimeter.set(record_from=["V_m"])

###############################################################################
# We now create a ``spike_recorder``, another device that records the spiking
# events produced by a neuron.

spikerecorder = nest.Create("spike_recorder")

###############################################################################
# A short note on naming: here we have called the neuron ``neuron``, the
# multimeter ``multimeter`` and so on. Of course, you can assign your created
# nodes to any variable names you like, but the script is easier to read if you
# choose names that reflect the concepts in your simulation.

###############################################################################
# Connecting nodes with default connections
# ------------------------------------------
#
# Now that we know how to create individual nodes, we can start connecting them
# to form a small network. The order in which the arguments to
# :py:func:`.Connect` are specified reflects the flow of events: if the neuron
# spikes, it sends an event to the spike recorder. Conversely, the multimeter
# periodically sends requests to the neuron to ask for its membrane potential
# at that point in time. This can be regarded as a perfect electrode stuck into
# the neuron.

nest.Connect(multimeter, neuron)
nest.Connect(neuron, spikerecorder)

###############################################################################
# Now that we have connected the network, we can start the simulation. We have
# to inform the simulation kernel how long the simulation is to run. Here we
# choose 1000 ms.

nest.Simulate(1000.0)

###############################################################################
# Congratulations, you have just simulated your first network in NEST!

###############################################################################
# Extracting and plotting data from devices
# -----------------------------------------
#
# After the simulation has finished, we can obtain the data recorded by the
# multimeter. We get a dictionary with the status of the multimeter, which
# contains an entry named ``events`` that holds the recorded data. It is itself
# a dictionary with the entries :hxt_ref:`V_m` and ``times``, which we store
# separately in ``Vms`` and ``ts``.

dmm = multimeter.get()
Vms = dmm["events"]["V_m"]
ts = dmm["events"]["times"]

###############################################################################
# Now we are ready to display the membrane potential in a figure. To this end,
# we make use of ``matplotlib`` and the ``pyplot`` module.

plt.figure(1)
plt.plot(ts, Vms)
plt.xlabel("time (ms)")
plt.ylabel("membrane potential V_m (mV)")

###############################################################################
# We proceed analogously to obtain and display the spikes from the spike
# recorder. Here we extract the events more concisely by passing the parameter
# name to :py:meth:`~.NodeCollection.get`, which returns the ``events``
# dictionary rather than the whole status dictionary.

events = spikerecorder.get("events")
senders = events["senders"]
ts = events["times"]

plt.figure(2)
plt.plot(ts, senders, ".")
plt.show()

###############################################################################
# Recording from multiple neurons with one multimeter
# ----------------------------------------------------
#
# It is possible to collect information from multiple neurons on a single
# multimeter. This does complicate retrieving the information: the data for
# each of the n neurons will be stored and returned in an interleaved fashion.
# Luckily, Python provides us with a handy array operation to split the data
# easily: array slicing with a step (sometimes called stride).
#
# To demonstrate this, we build a fresh network with two neurons whose
# background currents differ, both recorded by the same multimeter. We reset
# the kernel so this section starts from a clean state.

nest.ResetKernel()

neuron1 = nest.Create("iaf_psc_alpha")
neuron1.set(I_e=376.0)
neuron2 = nest.Create("iaf_psc_alpha")
neuron2.set(I_e=370.0)

multimeter = nest.Create("multimeter")
multimeter.set(record_from=["V_m"])

nest.Connect(multimeter, neuron1)
nest.Connect(multimeter, neuron2)

nest.Simulate(1000.0)

###############################################################################
# To plot the result correctly, we must plot the two neuron traces separately.
# We split the interleaved data using array slicing with a step of two.

dmm = multimeter.get()

plt.figure(3)
Vms1 = dmm["events"]["V_m"][::2]  # start at index 0: till the end: each second entry
ts1 = dmm["events"]["times"][::2]
plt.plot(ts1, Vms1)
Vms2 = dmm["events"]["V_m"][1::2]  # start at index 1: till the end: each second entry
ts2 = dmm["events"]["times"][1::2]
plt.plot(ts2, Vms2)
plt.xlabel("time (ms)")
plt.ylabel("membrane potential V_m (mV)")
plt.show()

###############################################################################
# Additional information on array indexing can be found in the `NumPy
# documentation
# <https://numpy.org/doc/stable/reference/arrays.indexing.html>`_.

###############################################################################
# Connecting nodes with specific connections
# -------------------------------------------
#
# A commonly used model of neural activity is the Poisson process. We now adapt
# the previous example so that the neuron receives two Poisson spike trains,
# one excitatory and the other inhibitory. Hence, we need a new device, the
# ``poisson_generator``. After creating the neuron and devices, we create the
# two generators and set their rates to 80000 Hz and 15000 Hz, respectively.

nest.ResetKernel()

neuron = nest.Create("iaf_psc_alpha")
multimeter = nest.Create("multimeter")
multimeter.set(record_from=["V_m"])
spikerecorder = nest.Create("spike_recorder")

noise_ex = nest.Create("poisson_generator")
noise_in = nest.Create("poisson_generator")
noise_ex.set(rate=80000.0)
noise_in.set(rate=15000.0)

###############################################################################
# Since the input now comes from the generators, the constant input current is
# set to 0.

neuron.set(I_e=0.0)

###############################################################################
# Each event of the excitatory generator should produce a postsynaptic current
# of 1.2 pA amplitude, an inhibitory event of -2.0 pA. The synaptic weights are
# defined in a dictionary, which is passed to the :py:func:`.Connect` function
# using the keyword ``syn_spec`` (synapse specifications). In general, all
# parameters determining the synapse can be specified in the synapse
# dictionary, such as ``"weight"``, ``"delay"``, the synaptic model
# (``"synapse_model"``) and parameters specific to the synaptic model.

syn_dict_ex = {"weight": 1.2}
syn_dict_in = {"weight": -2.0}
nest.Connect(noise_ex, neuron, syn_spec=syn_dict_ex)
nest.Connect(noise_in, neuron, syn_spec=syn_dict_in)

nest.Connect(multimeter, neuron)
nest.Connect(neuron, spikerecorder)

###############################################################################
# The rest of the code remains as before: we simulate and plot the membrane
# potential and the spikes.

nest.Simulate(1000.0)

dmm = multimeter.get()
Vms = dmm["events"]["V_m"]
ts = dmm["events"]["times"]

plt.figure(4)
plt.plot(ts, Vms)
plt.xlabel("time (ms)")
plt.ylabel("membrane potential V_m (mV)")

events = spikerecorder.get("events")
plt.figure(5)
plt.plot(events["times"], events["senders"], ".")
plt.show()

###############################################################################
# In the next part of the introduction (:ref:`Part 2: Populations of neurons
# <pynest_tutorial_2>`) we will look at more methods for connecting many
# neurons at once.

###############################################################################
# Two connected neurons
# ---------------------
#
# There is no additional magic involved in connecting neurons. To demonstrate
# this, we start from our original example of one neuron with a constant input
# current and add a second neuron. We record the membrane potential from
# ``neuron2`` so we can observe the postsynaptic potentials caused by the
# spikes of ``neuron1``.

nest.ResetKernel()

neuron1 = nest.Create("iaf_psc_alpha")
neuron1.set(I_e=376.0)
neuron2 = nest.Create("iaf_psc_alpha")
multimeter = nest.Create("multimeter")
multimeter.set(record_from=["V_m"])

###############################################################################
# We now connect ``neuron1`` to ``neuron2``. Here the default delay of 1 ms is
# used. If the delay is specified in addition to the weight, the following
# shortcut is available: ``syn_spec={"weight": 20.0, "delay": 1.0}``.

nest.Connect(neuron1, neuron2, syn_spec={"weight": 20.0})
nest.Connect(multimeter, neuron2)

###############################################################################
# If we simulate the network and plot the membrane potential, we see the
# postsynaptic potentials of ``neuron2`` evoked by the spikes of ``neuron1``.

nest.Simulate(1000.0)

dmm = multimeter.get()
plt.figure(6)
plt.plot(dmm["events"]["times"], dmm["events"]["V_m"])
plt.xlabel("time (ms)")
plt.ylabel("membrane potential V_m (mV)")
plt.show()

###############################################################################
# Command overview
# ----------------
#
# These are the functions we introduced for the examples in this handout; the
# following sections of this introduction will add more.
#
# **Getting information about NEST**
#
# See the :ref:`Getting Help Section <command_help>`.
#
# **Nodes**
#
# - ``Create(model, n=1, params=None)``: Create ``n`` instances of type
#   ``model``. Parameters for the new nodes can be given as ``params`` (a
#   dictionary with single values or lists of size n, or a list of n
#   dictionaries). If omitted, the model's defaults are used.
# - ``get(*params, **kwargs)``: Return a dictionary with parameter values for
#   the NodeCollection it is called on. If ``params`` is a single string, a
#   list of values is returned instead.
# - ``set(params=None, **kwargs)``: Set the parameters on the NodeCollection.
#
# **Connections**
#
# This is an abbreviated version of the documentation for the
# :py:func:`.Connect` function; see :ref:`Connectivity concepts
# <connectivity_concepts>` for an introduction and examples.
#
# - ``Connect(pre, post, conn_spec=None, syn_spec=None,
#   return_synapsecollection=False)``: Connect ``pre`` neurons to ``post``
#   neurons using the specified connectivity (``"all_to_all"`` by default) and
#   synapse type (``"static_synapse"`` by default).
#
# **Simulation control**
#
# - ``Simulate(t)``: Simulate the network for ``t`` milliseconds.

###############################################################################
# References
# ----------
#
# .. [1] Gewaltig MO and Diesmann M. 2007. NEural Simulation Tool. 2(4):1430.
# .. [2] Eppler JM et al. 2009. PyNEST: A convenient interface to the NEST
#        Simulator. 2:12. 10.3389/neuro.11.012.2008.
# .. [3] Hunter JD. 2007. Matplotlib: A 2d graphics environment. 9(3):90-95.
