Those parrot neurons, or why you never connect devices to each other
--------------------------------------------------------------------

In ``receivesimple.py`` above, we used parrot neurons as
target for the input proxies, then connected the spike detector to those
neurons. Couldn’t we have connected the spike detector directly to the
proxies?

No, we could not. NEST devices are not generally implemented the same
way as neuron models, and are not designed to be connected directly to
each other. Normal neurons exist only on one node in a distributed
simulation, but devices such as the spike detector and the MUSIC event
handler are usually duplicated aross all nodes.

When you connect a set of neurons to a spike detector, you normally
don’t want all that spike data travel across the network to a single
computing node where it gets saved. It would be very inefficient.
Instead, the spike detector is duplicated on each node and each clone
saves the data from its local neurons. In the same way, the MUSIC event
handler is duplicated on all nodes, and each clone forwards only the
channels that its local targets request.

But if you connect the input proxies directly to the spike detector,
*all* channels have a local spike detector target on every computing
node. We would get duplicate spike traces, one for each MPI process on
the receiving side. To test this, replace line 21 in
``receive.py`` with

.. code-block:: python

    nest.Connect(music_in, sdetector)

Run this simulation and you will notice that the
``receive-N-0.spikes`` and
:math:``receive-N-1.spikes`` are now identical and about twice as
big as before. Collate the input files and compare again:

.. code-block::

    send.spikes                receive.spikes

    2    26.100                2       26.100
    1    27.800                2       26.100
    2    54.200                1       27.800
    1    57.600                1       27.800
    2    82.300                2       54.200
    1    87.400                2       54.200
    2    110.40                1       57.600
    1    117.20                1       57.600

Before, each output file would contain the spike events from just one of
the two channels. Now both output files contain spikes from all
channels, so all our input events are duplicated. Also, as you can see
the input and output times are now identical, since a delay is never
applied anywhere along the path from the inputs to the outputs.

The lesson is that you don’t connect two NEST devices to each other
unless the documentation specifically tells you that you can. Always add
a layer of normal neuron models, such as parrot neurons, in between.
This is true for devices in general of course, but this connection
pattern, where you want to record the MUSIC input from another
simulation, is so common that it’s worth warning for this.


