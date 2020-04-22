Connect two NEST simulations using MUSIC
======================================================

Let’s look at an example of two NEST simulations connected through
MUSIC. We’ll implement the simple network in :numref:`neuronmusic3`
from :doc:`the introduction to this tutorial <music_tutorial_1>`.

We need a sending process, a receiving process and a MUSIC
configuration file:


.. code-block:: python
    :linenos:

    #!/usr/bin/env python

    import nest
    nest.SetKernelStatus({"overwrite_files": True})

    neurons = nest.Create('iaf_psc_alpha', 2, [{'I_e': 400.0}, {'I_e': 405.0}])

    music_out = nest.Create('music_event_out_proxy', 1,
        params = {'port_name':'p_out'})

    for i, n in enumerate(neurons):
        nest.Connect([n], music_out, "one_to_one",{'music_channel': i})

    sdetector = nest.Create("spike_detector")
    nest.SetStatus(sdetector, {"record_to": "ascii", "label": "send"})

    nest.Connect(neurons, sdetector)

    nest.Simulate(1000.0)

The sending process is quite straightforward. We import the NEST library
and set a useful kernel parameter. On line 6, we create two simple
intergrate-and-fire neuron models, one with a current input of 400mA,
and one with 405mA, just so they will respond differently. If you use
ipython to work interactively, you can check their current status
dictionary with ``nest.GetStatus(neurons)``. The definitive
documentation for NEST nodes is the header file, in this case
``models/iaf_psc_alpha.h`` in the NEST source.

We create a single ``music_event_out_proxy`` for our
output on line 8, and set the port name. We loop over all the neurons on
lines 11-20 and connect them to the proxy one by one, each one with a
different output channel. As we saw earlier, each MUSIC port can have
any number of channels. Since the proxy is a device, it ignores any
weight or delay settings here.

Lastly, we create a spike detector, set the parameters (which we could
have done directly in the ``Create`` call) and connect the
neurons to the spike detector so we can see what we’re sending. Then we
simulate for one second.

.. code-block:: python
    :linenos:

    #!/usr/bin/env python

    import nest
    nest.SetKernelStatus({"overwrite_files": True})

    music_in = nest.Create("music_event_in_proxy", 2,
        params = {'port_name': 'p_in'})

    for i, n in enumerate(music_in):
        nest.SetStatus([n], {'music_channel': i})

    nest.SetAcceptableLatency('p_in', 2.0)

    parrots = nest.Create("parrot_neuron", 2)

    sdetector = nest.Create("spike_detector")
    nest.SetStatus(sdetector, {"record_to": ["ascii"], "label": "receive"})

    nest.Connect(music_in, parrots, 'one_to_one', {"weight":1.0, "delay": 2.0})
    nest.Connect(parrots, sdetector)

    nest.Simulate(1000.0)

The receiving process follows the same logic, but is just a little more
involved. We create two ``music_event_in_proxy`` — one
per channel — on lines 6-7 and set the input port name. As we discussed
above, a NEST node can accept many inputs but only emit one stream of
data, so we need one input proxy per channel to be able to distinguish
the channels from each other. On lines 9-10 we set the input channel for
each input proxy.

:doc:`The SetAcceptableLatency command <music_tutorial_setlatency>` on line 12 sets the
maximum time, in milliseconds, that MUSIC is allowed to delay delivery of spikes
transmitted through the named port. This should never be more than the
*minimum* of the delays from the input proxies to their targets; that’s
the 2.0 ms we set on line 20 in our case.

On line 14 we create a set of :doc:`parrot neurons <music_tutorial_parrot>`.
They simply repeat the input they’re given. On lines 16-18 we create and
configure a spike detector to save our inputs. We connect the input proxies
one-to-one with the parrot neurons on line 20, then the parrot neurons to
the spike detector on line 21. We will discuss the reasons for this in a moment.
Finally we simulate for one second.

.. code-block:: sh

      binary=./send.py
      np=2

      [to]
      binary=./receive.py
      np=2

      from.p_out -> to.p_in [2]

The MUSIC configuration file structure is straightforward. We define one
process ``from`` and one ``to``. For each
process we set the name of the binary we wish to run and the number of
MPI processes it should use. On line 9 we finally define a connection
from output port ``p_out`` in process
``from`` to input port ``p_in`` in process
``to``, with two channels.

If our programs had taken command line options we could have added them
with the ``args`` command:



.. code-block:: sh

      binary=./send.py
      args= --option -o somefile

Run the simulation on the command line like this:

.. code-block:: sh

      mpirun -np 4 music python.music

You should get a screenful of information scrolling past, and then be
left with four new data files, named something like ``send-N-0.spikes``,
``send-N-1.spikes``, ``receive-M-0.spikes`` and ``receive-M-1.spikes``. The names
and suffixes are of course the same that we set in ``send.py`` and
``receive.py`` above. The first numeral is the node ID of the spike detector
that recorded and saved the data, and the final numeral is the rank order of
each process that generated the file.

Collate the data files:


.. code-block:: sh

      cat send-*spikes | sort -k 2 -n  >send.spikes
      cat receive-*spikes | sort -k 2 -n  >receive.spikes

We run the files together, and sort the output numerically
(:math:`-n`) by the second column (:math:`-k`). Let’s
look at the beginning of the two files side by side:


.. code-block:: sh

    send.spikes                receive.spikes

    2   26.100                 4   28.100
    1   27.800                 3   29.800
    2   54.200                 4   56.200
    1   57.600                 3   59.600
    2   82.300                 4   84.300
    1   87.400                 3   89.400
    2   110.40                 4   112.40
    1   117.20                 3   119.20

As expected, the received spikes are two milliseconds later than the
sent spikes. The delay parameter for the connection from the input
proxies to the parrot neurons in ``receive.py`` on line 20
accounts for the delay.

Also — and it may be obvious in a simple model like this — the neuron
IDs on the sending side and the IDs on the receiving side have no fixed
relationship. The sending neurons have ID 1 and 2, while the recipients
have 3 and 4. If you need to map events in one simulation to events in
another, you have to record this information by other means.

Continuous Inputs
-----------------

MUSIC can send not just spike events, but also continuous inputs and
messages. In NEST there are devices to receive, but not send, such
inputs. The NEST documentation has a few examples such as this one
below:


.. code-block:: python
    :linenos:

    #!/usr/bin/python

    import nest

    mcip = nest.Create('music_cont_in_proxy')
    nest.SetStatus(mcip, {'port_name' : 'contdata'})

    time = 0
    while time < 1000:
        nest.Simulate (10)
        data = nest.GetStatus (mcip, 'data')
        print data
        time += 10

The start mirrors our earlier receiving example: you create a continuous
input proxy (a single input in this case) and set the port name.

NEST has no general facility to actually apply continuous-valued inputs
directly into models. Its neurons deal only with spike events. To use
the input you need to create a loop on lines 9-13 where you simulate for
a short period, explicitly read the value on line 11, apply it to the
simulation model, then simulate for a period again.

People sometimes try to use this pattern to control the rate of a
Poisson generator from outside the simulation. You get the rate from
outside as a continuous value, then apply it to the Poisson generator
that in turn stimulates input neurons in your network.

The problem is that you need to suspend the simulation every cycle, drop
out to the Python interpreter, run a bit of code, then call back in to
the simulator core and restart the simulation again. This is acceptable
if you do it every few hundred or thousand milliseconds or so, but with
an input that may change every few milliseconds this becomes very, very
slow.

A much better approach is to forgo the use of the NEST Poisson
generator. Generate a Poisson sequence of spike events in the *outside*
process, and send the spike events directly into the simulation like we
did in our earlier python example. This is far more effective, and the
outside process is not limited to the generators implemented in NEST but
can create any kind of spiking input. In the next section we will take a
look at how to do this.

