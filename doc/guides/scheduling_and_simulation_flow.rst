Scheduling and simulation flow
==============================

Introduction
------------

To drive the simulation, neurons and devices (*nodes*) are updated in a
time-driven fashion by calling a member function on each of them in a
regular interval. The spacing of the grid is called the *simulation
resolution* (default 0.1ms) and can be set using ``SetKernelStatus``:

::

    SetKernelStatus("resolution", 0.1)

Even though a neuron model can use smaller time steps internally, the
membrane potential will only be visible to a ``multimeter`` on the
outside at time points that are multiples of the simulation resolution.

In contrast to the update of nodes, an event-driven approach is used for
the synapses, meaning that they are only updated when an event is
transmitted through them (`Morrison et al.
2005 <http://dx.doi.org/10.1162/0899766054026648>`_). To speed up the
simulation and allow the efficient use of computer clusters, NEST uses a
:doc:`hybrid parallelization strategy <parallel_computing>`. The
following figure shows the basic loop that is run upon a call to
``Simulate``:

.. figure:: ../_static/img/simulation_loop-241x300.png
   :alt: Simulation Loop

   Simulation Loop

The simulation loop. Light gray boxes denote thread parallel parts, dark
gray boxes denote MPI parallel parts. U(St) is the update operator that
propagates the internal state of a neuron or device.

Simulation resolution and update interval
-----------------------------------------

Each connection in NEST has it's own specific *delay* that defines the
time it takes until an event reaches the target node. We define the
minimum delay *dmin* as the smallest transmission delay and *dmax* as
the largest delay in the network. From this definition follows that no
node can influence another node during at least a time of *dmin*, i.e.
the elements are effectively decoupled for this interval.

.. figure:: ../_static/img/time_definitions-300x61.png
   :alt: Definitions of the minimimum delay and the simulation resolution

   Definitions of the minimimum delay and the simulation resolution.

Definitions of minimum delay (dmin) and simulation resolution (h).

Two major optimizations in NEST are built on this decoupling:

1. Every neuron is updated in steps of the simulation resolution, but
   always for *dmin* time in one go, as to keep neurons in cache as long
   as possible.

2. MPI processes only communicate in intervals of *dmin* as to minimize
   communication costs.

These optimizations mean that the sizes of spike buffers in nodes and
the buffers for inter-process communication depend on *dmin+dmax* as
histories that long back have to be kept. NEST will figure out the
correct value of *dmin* and *dmax* based on the actual delays used
during connection setup. Their actual values can be retrieved using
``GetKernelStatus``:

::

    GetKernelStatus("min_delay")   # (A corresponding entry exists for max_delay)

Setting *dmin* and *dmax* manually
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In linear simulation scripts that build a network, simulate it, carry
out some post-processing and exit, the user does not have to worry about
the delay extrema *dmin* and *dmax* as they are set automatically to the
correct values. However, NEST also allows subsequent calls
to\ ``Simulate``, which only work correctly if the content of the spike
buffers is preserved over the simulations.

As mentioned above, the size of that buffer depends on *dmin+dmax* and
the easiest way to assert its integrity is to not change its size after
initialization. Thus, we freeze the delay extrema after the first call
to ``Simulate``. To still allow adding new connections inbetween calls
to ``Simulate``, the required boundaries of delays can be set manually
using ``SetKernelStatus`` (Please note that the delay extrema are set as
properties of the synapse model):

::

    SetDefaults("static_synapse", {"min_delay": 0.5, "max_delay": 2.5})

These settings should be used with care, though: setting the delay
extrema too wide without need leads to decreased performance due to more
update calls and communication cycles (small *dmin*), or increased
memory consumption of NEST (large *dmax*).

Spike generation and precision
------------------------------

A neuron fires a spike when the membrane potential is above threshold at
the end of an update interval (i.e., a multiple of the simulation
resolution). For most models, the membrane potential is then reset to
some fixed value and clamped to that value during the refractory time.
This means that the last membrane potential value at the last time step
before the spike can vary, while the potential right after the step will
usually be the reset potential (some models may deviate from this). This
also means that the membrane potential recording will never show values
above the threshold. The time of the spike is always the time at *the
end of the interval* during which the threshold was crossed.

NEST also has a some models that determine the precise time of the
threshold crossing during the interval. Please see the documentation on
`precise spike time neurons <simulations-with-precise-spike-times.md>`__
for details about neuron update in continuous time and the
`documentation on connection management <connection-management.md>`__
for how to set the delay when creating synapses.
