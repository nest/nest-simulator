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

::

    % create 2 neurons, get list of IDs. 
    /iaf_neuron [2] LayoutNetwork /neuron_out_net Set
    /neuron_out neuron_out_net GetGlobalNodes def

    % create output proxy. We use only one, so we use plain Create.
    /music_event_out_proxy << /port_name (p_out) >> Create /music_out Set

    % connect the neurons to the proxy, and give them a separate channel each
    neuron_out {
        /i Set /n Set
        n music_out << /music_channel i >> Connect
    } forallindexed

    1000.0 Simulate

Comments are prefixed with a “%”. On line 2-3 we create two
:math:`\texttt{iaf\_neuron}` in a subnet (nodes in SLI are organized in
a tree-like fashion), save the subnet ID in
:math:`\texttt{neuron\_out\_net}` then get an array of the neuron IDs in
the subnet and save in :math:`\texttt{neuron\_out}`.

What is the difference between :math:`\texttt{Set}` on line 2 and
:math:`\texttt{def}` on line 3? Just the order of the arguments: with
:math:`\texttt{Set}` you first give the object, then the name you want
to associate with it. With :math:`\texttt{put}` you give the name first,
then the object.  Both are used extensively so you need to be aware
of them.

On line 6 we create a MUSIC output proxy with port name
:math:`\texttt{p\_out}`. Dictionaries are bracketed with “<<” and “>>”,
and strings are bracketed with parenthesis.

On lines 9-12 we iterate over all neurons with index and store the index
in “i” and the neuron ID in “n”. Then connect each one to the output
proxy with its own music channel. Note that here we use
:math:`\texttt{Set}` to assign the neuron ID and sequence on the stack
to variables. We’d have to rotate the top stack elements if we wanted to
use :math:`\texttt{put}`.

::

    % create 2 neurons, get list of IDs.
    /music_event_in_proxy [2] LayoutNetwork /music_in_net Set
    music_in_net GetGlobalNodes /music_in Set

    % 2 parrot neurons.
    /parrot_neuron [2] LayoutNetwork /parrot_in_net Set
    parrot_in_net GetGlobalNodes /parrot_in Set

    % Create spike detector
    /spike_detector Create /sdetector Set
    sdetector << /record_to /ascii
                 /label (output)
    >> SetStatus

    % set port name and channel for all music input proxies.
    music_in {
        /i Set /m Set
        m << /port_name (p_in) /music_channel i >> SetStatus
    } forallindexed

    % set acceptable latency
    (p_in) 2.0 SetAcceptableLatency

    % connect music proxies to parrots, one to one
    music_in {
        /i Set /m Set
        m parrot_in i get 1.0 2.0 Connect
    } forallindexed

    parrot_in [sdetector]  Connect

    1000.0 Simulate

SLI, like PyNEST, has a specific function for setting the acceptable
latency, as we do on line 23. In lines 26-29 we do one-on-one
connections between the input proxies and the parrot neurons, and set
the desired delay. We iterate over all music proxies, and for each proxy
we get the corresponding element in the :math:`\texttt{parrot\_in}`
array, then connect them with weight 1.0 and delay 2.0. You must give
both parameters even though only the delay matters.

For more information on using SLI, the browser based help we mentioned
in the introduciton is quite helpful, but the best resource is the set
of example models in the NEST source code distribution. That will show
you many useful idioms and typical ways to accomplish common tasks.


