Scheduling and simulation flow
==============================

Introduction
------------

To drive the simulation in time, neurons and devices (*nodes*) are updated in a time-driven fashion by calling a member function on each of them in a regular interval. The spacing of the grid is called the *simulation resolution* (default 0.1ms) and can be set using `SetKernelStatus`:

    SetKernelStatus("resolution", 0.1)

In contrast to the update of nodes, an event-driven approach is used for the synapses, meaning that they are only updated when an event is transmitted through them ([Morrison et al. 2005](http://dx.doi.org/10.1162/0899766054026648)). To speed up the simulation and allow the efficient use of computer clusters, NEST uses a [hybrid parallelization strategy](../parallel_computing/index.html). The following figure shows the basic loop that is run upon a call to `Simulate`:

![Simulation Loop](../../img/simulation_loop-241x300.png)
The simulation loop. Light gray boxes denote thread parallel parts, dark gray boxes denote MPI parallel parts. U(S<sub>t</sub>) is the update operator that propagates the internal state of a neuron or device.

Simulation resolution and update interval
-----------------------------------------

Each connection in NEST has it's own specific *delay* that defines the time it takes until an event reaches the target node. We define the minimum delay *d<sub>min</sub>* as the smallest transmission delay and *d<sub>max</sub>* as the largest delay in the network. From this definition follows that no node can influence another node during at least a time of *d<sub>min</sub>*, i.e. the elements are effectively decoupled for this interval.

![Definitions of the minimimum delay and the simulation resolution.](../../img/time_definitions-300x61.png)
Definitions of minimum delay (d<sub>min</sub>) and simulation resolution (h).

Two major optimizations in NEST are built on this decoupling:

1.  Every neuron is updated in steps of the simulation resolution, but always for *d<sub>min</sub>* time in one go, as to keep neurons in cache as long as possible.
2.  MPI processes only communicate in intervals of *d<sub>min</sub>* as to minimize communication costs.

These optimizations mean that the sizes of spike buffers in nodes and the buffers for inter-process communication depend on *d<sub>min</sub>+d<sub>max</sub>* as histories that long back have to be kept. NEST will figure out the correct value of *d<sub>min</sub>* and *d<sub>max</sub>* based on the actual delays used during connection setup. Their actual values can be retrieved using `GetKernelStatus`:

    GetKernelStatus("min_delay")   # (A corresponding entry exists for max_delay)

### Setting *d<sub>min</sub>* and *d<sub>max</sub>* manually

In linear simulation scripts that build a network, simulate it, carry out some post-processing and exit, the user does not have to worry about the delay extrema *d<sub>min</sub>* and *d<sub>max</sub>* as they are set automatically to the correct values. However, NEST also allows subsequent calls to` Simulate`, which only work correctly if the content of the spike buffers is preserved over the simulations.

As mentioned above, the size of that buffer depends on *d<sub>min</sub>+d<sub>max</sub>* and the easiest way to assert its integrity is to not change its size after initialization. Thus, we freeze the delay extrema after the first call to `Simulate`. To still allow adding new connections inbetween calls to `Simulate`, the required boundaries of delays can be set manually using `SetKernelStatus `(Please note that the delay extrema are set as properties of the synapse model):

    SetDefaults("static_synapse", {"min_delay": 0.5, "max_delay": 2.5})

These settings should be used with care, though: setting the delay extrema too wide without need leads to decreased performance due to more update calls and communication cycles (small *d<sub>min</sub>*), or increased memory consumption of NEST (large *d<sub>max</sub>*).

Related topics
--------------

Please see the [FAQ on precise spike time neurons](../qa-precise-spike-times/index.html) for details about neuron update in continuous time and the [documentation on connection management](../connection_management/index.html) for how to set the delay when creating synapses.
