Getting Started
================

A quick overview of simulating neural networks
------------------------------------------------

A NEST simulation tries to follow the logic of an electrophysiological
experiment - the difference being it takes place inside the computer
rather than in the physical world.

In NEST, the neural system is a collection of **nodes** and their `Connections`_.
Nodes correspond to ``neurons`` and ``devices`` and connections by ``synapses``.
Different neuron and synapse models can coexist in the same network.

To measure or observe the network activity, you can define
so-called `Devices`_ that represent the various instruments (for measuring and
stimulating) found in an experiment. These devices write their data either to
memory or to file.

The network and its configuration are defined at the level
of the simulation language interpreter (SLI) as well as the PyNEST level.

Check out our :doc:`PyNEST tutorial <tutorials/index>`,
which will explain how to build your first neural network simulation in NEST.


.. sidebar:: See Also

    * :doc:`List of Models in NEST <models/index>`
    * :doc:`Create your own model <models/create_model>`
    * :doc:`Examples of Network Models <examples/index>`


How do I use NEST?
-------------------
As the experimenter, you need a clear idea of *what* you want to learn from the experiment.
In the context of a network `Simulation`_, this means that you have to know
*which input* you want to give to your network and *which output* you're interested in.

You can use NEST either with Python (PyNEST) or as a stand alone application (
``nest``).
PyNEST provides a :doc:`set of commands <ref_material/index>` to the Python interpreter which give you
access to NEST's simulation kernel. With these commands, you describe and run
your network simulation.

A basic network setup in PyNEST
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can use PyNEST interactively from the Python prompt or from within ipython.
This is very helpful when you are exploring PyNEST, trying to learn a new
functionality or debugging a routine. Once out of the exploratory mode, you will
find it saves a lot of time to write your simulations in text files. These can
in turn be run from the command line or from the Python or ipython prompt.

Fundamentally, you can build a basic network with the following functions::

    # Create the models you want to simulate
    neuron = nest.Create("model_name")

    # Create the device to stimulate or measure the simulation
    device = nest.Create("device_name")

    # Modify properties of the neuron and device
    nest.SetStatus(neuron, {"key" : value})
    nest.SetStatus(device, {"key" : value})

    # Tell NEST how they are connected to each other (synapse properties can be
    # added here)
    nest.Connect(device, neuron, syn_spec={"key": [value1, value2]})

    # Simulate network providing a specific timeframe.
    nest.Simulate(time_in_ms)

NEST is extensible and new models for neurons, synapses, and devices can be
added. You can find out how to :doc:`create your own model <models/create_model>`
using NESTML and c++.


Connections
~~~~~~~~~~~~

Connections between nodes (neurons, devices or synapses) define possible channels for interactions between
them. A connection between two nodes is established, using the command
``Connect``.

Each connection has two basic parameters, *weight* and *delay*. The weight
determines the strength of the connection, the delay determines how long an
event needs to travel from the sending to the receiving node. The delay must be
a positive number greater or equal to the simulation stepsize and is given in
ms.

Devices
~~~~~~~~

Devices are network nodes which provide input to the network or record its
output. They encapsulate the stimulation and measurement process. If you want
to extract certain information from a simulation, you need a device which is
able to deliver this information. Likewise, if you want to send specific input
to the network, you need a device which delivers this input.

Devices have a built-in timer which controls the period they are active. Outside
this interval, a device will remain silent. The timer can be configured using
the command ``SetStatus``.

Simulation
~~~~~~~~~~~~~

NEST simulations are time driven. The simulation time proceeds in discrete steps
of size ``dt``, set using the property ``resolution`` of the root node. In each time
slice, all nodes in the system are updated and pending events are delivered.

The simulation is run by calling the command ``Simulate(t)``, where ``t`` is the
simulation time in milliseconds. See below for list of physical units in NEST.

Physical units in NEST
-----------------------

-   time - ms
-   voltage - mV
-   capacitance - pF
-   current - pA
-   conductance - nS
-   Spike rates (e.g. poisson\_generator) - spikes/s
-   modulation frequencies (e.g. ac\_generator) - Hz

Next Steps
-----------

* :doc:`Download <download>` and :doc:`Install NEST <installation/index>`
* Follow the :doc:`PyNEST tutorial <tutorials/index>` and simulate a neural network

