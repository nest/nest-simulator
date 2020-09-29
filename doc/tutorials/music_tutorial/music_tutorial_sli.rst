MUSIC with SLI
==============

SLI is the built-in scripting language in NEST. It is a stack-based
language loosely modelled on PostScript. It is quite cumbersome to work
with, is not extensively documented, and has quite a few corner cases,
design issues and unintended behaviours. But it is small, already built
into NEST and is much more memory efficient than using Python. If your
models are very large and memory is tight, or you are using a system
where Python isn’t available, then SLI is the way to go.

We won’t discuss the code extensively as learning SLI is really outside
the scope of this tutorial. The code follows the same structure as the
other examples, and should be straightforward to follow. But we will
give a few pointers for how to connect things with MUSIC.

The SLI version of the sending process file from
:doc:`Part 2 of the MUSIC tutorial <music_tutorial_2>`, *sender.sli*, is outlined
below. Comments are prefixed with a “%”.

::

    % create 2 neurons, get NodeCollection representing IDs.
    /NUM_NEURONS 2 def
    /iaf_psc_alpha NUM_NEURONS Create /neuron_out Set

    % create output proxy.
    /music_event_out_proxy << /port_name (p_out) >> Create /music_out Set

    % connect the neurons to the proxy, and give them a separate channel each
    [NUM_NEURONS] Range
    {
        /index Set
        neuron_out [index] Take music_out << /rule /one_to_one >> << /music_channel index 1 sub >> Connect
    } forall

    1000.0 Simulate

On line 2-3 we create two `iaf_psc_alpha` in a NodeCollection and save it in `neuron_out`.

The difference between ``def`` on line 2 and
``Set`` on line 3 is the order of the arguments: with
``Set`` you first give the object, then the name you want
to associate with it. With ``def`` you give the name first,
then the object.  Both are used extensively so you need to be aware
of them.

On line 6 we create a MUSIC output proxy with port name
`p_out`. Dictionaries are bracketed with “<<” and “>>”,
and strings are bracketed with parenthesis.

On lines 9-13 we iterate over the range of all neurons and store the index
in `index`. Then we connect each neuron in the NodeCollection to the output
proxy with its own music channel. To get the individual node we use ``Take``.
Note that we use ``Set`` to assign the index on the stack
to a variable. We’d have to rotate the top stack elements if we wanted to
use ``def``.

For the receiving SLI file, *receiver.sli*, we have:

::

    % Create 2 MUSIC nodes, get NodeCollection representing IDs.
    /NUM_NODES 2 def
    /music_event_in_proxy NUM_NODES Create /music_in Set

    % Create 2 parrot neurons.
    /parrot_neuron NUM_NODES Create /parrot_in Set

    % Create spike recorder
    /spike_recorder Create /sr Set
    sr << /record_to /ascii
                 /label (output)
    >> SetStatus

    % set port name and channel for all music input proxies.
    music_in
    {
      /music_node Set
      /channel music_node 1 sub def
      music_node << /port_name (p_in) /music_channel channel >> SetStatus
    } forall

    % set acceptable latency
    (p_in) 2.0 SetAcceptableLatency

    % connect music proxies to parrots, one to one
    music_in parrot_in << /rule /one_to_one >> << /delay 2.0 >> Connect

    parrot_in sr Connect

    1000.0 Simulate

SLI, like PyNEST, has a specific function for setting the acceptable
latency, as we do on line 23. In line 26 we do a one-to-one
connection between the input proxies and the parrot neurons, and set
the desired delay.

For the MUSIC configuration file, we now need to use `binary=nest` to make it
run with nest, and pass the correct files as arguments:

.. code-block:: sh

        [from]
            binary=nest
            np=2
            args=send.sli

        [to]
            binary=nest
            np=2
            args=receive.sli

        from.p_out -> to.p_in [2]

For more information on using SLI, the browser based help we mentioned
in the introduction is quite helpful, but the best resource is the set
of example models in the NEST source code distribution. That will show
you many useful idioms and typical ways to accomplish common tasks.

.. note::

   Please note that MUSIC and the recording backend for Arbor are mutually exclusive
   and cannot be enabled at the same time.
