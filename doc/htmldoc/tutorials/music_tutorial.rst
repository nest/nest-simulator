.. _music_tutorial_1:

Introduction to the MUSIC Interface
===================================

The `MUSIC interface <http://software.incf.org/software/music>`_, a
standard by the INCF, allows the transmission of data between applications
at runtime. It can be used to couple NEST with other simulators, with
applications for stimulus generation and data analysis and visualization and
with custom applications that also use the MUSIC interface.

Setup of System
-----------------
To use MUSIC with NEST, we first need to ensure MUSIC is installed on our system
and NEST is configured properly.

Please install MUSIC using the instructions on `the MUSIC website <https://github.com/INCF/MUSIC>`_.

In the installation of NEST, you need to add the following configuration option to
your CMake.

.. code-block:: sh

    cmake -Dwith-music=[ON </path/to/music>]
    make
    make install

A Quick Introduction to NEST and MUSIC
--------------------------------------

In this tutorial, we will show you how to use the MUSIC library together
with NEST. We will cover how to use the library from PyNEST and from the
SLI language interface. In addition, we'll introduce the use of MUSIC in
a C++ application and how to connect such an application to a NEST
simulation.

Our aim is to show practical examples of how to use MUSIC, and
highlight common pitfalls that can trip the unwary. Also, we assume only
a minimal knowledge of Python, C++ and (especially) SLI, so the examples
will favour clarity and simplicity over elegance and idiomatic
constructs.

.. sidebar:: Jump to

    :ref:`Part 1 - Connect 2 NEST simulations <music_tutorial_2>`



While the focus here is on MUSIC, we need to know a few things about how
NEST works in order to understand how MUSIC interacts with it.

The Basics of NEST
~~~~~~~~~~~~~~~~~~

A NEST network consists of three types of elements: neurons, devices,
and connections between them.

Neurons are the basic building blocks, and in NEST they are generally
spiking point neuron models. Devices are supporting units that for
instance generate inputs to neurons or record data from them. The
Poisson spike generator, the spike recorder recording device, and the
MUSIC input and output proxies are all devices. Neurons and devices are
collectively called nodes, and are connected using connections.

Connections are unidirectional and carry events between nodes. Each
neuron can get multiple input connections from any number of other
neurons. Neuron connections typically carry spike events, but other
kinds of events, such as voltages and currents, are also available for
recording devices. Synapses are not independent nodes, but are part of
the connection. Synapse models will typically modify the weight or
timing of the spike sent on to the neuron. All connections have a
synapse, by default the :hxt_ref:`static_synapse`.

.. _neuronpic:

.. figure:: neuron.png
   :width: 200px
   :align: center

   A: Two connected neurons :math:`N_a` and :math:`N_b`, with a
   synapse :math:`S` and a receptor :math:`R`. A spike with weight
   :math:`W_a` is generated at :math:`t_0`. B: The spike traverses the
   synapse and is added to the queue in the receptor. C: The receptor
   processes the spike at time :math:`t_0 + d`.

Connections have a delay and a weight. All connections are implemented
on the receiving side, and the interpretation of the parameters is
ultimately up to the receiving node. In :numref:`neuronpic` A, neuron
:math:`N_a` has sent a spike to :math:`N_b` at time :math:`t`, over a
connection with weight :math:`w_a` and delay :math:`d`. The spike is
sent through the synapse, then buffered on the receiving side until
:math:`t+d` (:numref:`neuronpic` B). At that time it's handed over to the
neuron model receptor that converts the spike event to a current and
applies it to the neuron model (:numref:`neuronpic` C).


Adding MUSIC connections
------------------------

In NEST you use MUSIC with a pair of extra devices called *proxies* that
create a MUSIC connection between them across simulations. The pair
effectively works just like a regular connection within a single
simulation. Each connection between MUSIC proxies is called a *port*,
and connected by name in the MUSIC configuration file.

Each MUSIC port can carry multiple numbered *channels*. The channel is
the smallest unit of transmission, in that you can distinguish data
flowing in different channels, but not within a single channel.
Depending on the application a port may have one or many channels, and a
single channel can carry the events from one single neuron model or the
aggregate output of many neurons.

.. _neuronmusic1:

.. figure:: neuronmusic1.png
   :width: 200px
   :align: center

   A: Two connected neurons :math:`N_a` and :math:`N_b`, with delay
   :math:`d_n` and weight :math:`w_n`. B: We've added a MUSIC connection
   with an output proxy :math:`P_a` on one end, and an input proxy
   :math:`P_b` on the other.

In :numref:`neuronmusic1` A we see a regular NEST connection between
two neurons :math:`N_a` and :math:`N_b`. The connection carries a weight
:math:`w_n` and a delay :math:`d_n`. In :numref:`neuronmusic1` B we
have inserted a pair of MUSIC proxies into the connection, with an
output proxy :math:`P_a` on one end, and input proxy :math:`P_b` on the
other.

As we mentioned above, MUSIC proxies are devices, not regular neuron
models. Like most devices, proxies ignore weight and delay parameters on
incoming connections. Any delay applied to the connection from
:math:`N_a` to the output proxy :math:`P_a` is thus silently ignored.
MUSIC makes the inter-simulation transmission delays invisible to the
models themselves, so the connection from :math:`P_a` to :math:`P_b` is
effectively zero. The total delay and weight of the connection from
:math:`N_a` to :math:`N_b` is thus that set on the :math:`P_b` to
:math:`N_b` connection.

.. _neuronmusic3:

.. figure:: neuronmusic3.png
   :width: 200px
   :align: center

   A MUSIC connection with two outputs and two inputs. A single output
   proxy sends two channels of data to an input event handler that
   divides the channels to the two input proxies. They connect the
   recipient neuron models.

When we have multiple channels, the structure looks something like in
:numref:`neuronmusic3`. Now we have two neurons :math:`N_{a1}` and
:math:`N_{a2}` that we want to connect to :math:`N_{b1}` and
:math:`N_{b2}` respectively. As we mentioned above, NEST devices can
accept connections from multiple separate devices, so we only need one
output proxy :math:`P_a`. We connect each input to a different channel.

Nodes can only output one connection stream, so on the receiving side we
need one input proxy :math:`P_b` per input. Internally, there is a
single MUSIC event handler device :math:`Ev` that accepts all inputs
from a port, then sends the appropriate channel inputs to each input
proxy. These proxies each connect to the recipient neurons as above.

Publication
-----------

Djurfeldt M. et al. 2010. Run-time interoperability between neuronal
network simulators based on the music framework. Neuroinformatics.
8(1):43–60. `DOI: 10.1007/s12021-010-9064-z <https://link.springer.com/article/10.1007/s12021-010-9064-z>`_.
.. _music_tutorial_2:

Connect two NEST simulations using MUSIC
========================================

Let's look at an example of two NEST simulations connected through
MUSIC. We'll implement the simple network in :numref:`neuronmusic3`
from :ref:`the introduction to this tutorial <music_tutorial_1>`.

We need a sending process, a receiving process and a MUSIC
configuration file.

To try out the example, save the following sending process code in a Python file
called *send.py*.


.. code-block:: python

    #!/usr/bin/env python3

    import nest
    nest.overwrite_files = True

    neurons = nest.Create('iaf_psc_alpha', 2, {'I_e': [400.0, 405.0]})

    music_out = nest.Create('music_event_out_proxy', 1, {'port_name':'p_out'})

    for i, neuron in enumerate(neurons):
        nest.Connect(neuron, music_out, "one_to_one", {'music_channel': i})

    srecorder = nest.Create("spike_recorder")
    srecorder.set(record_to="ascii", label="send")

    nest.Connect(neurons, srecorder)

    nest.Simulate(1000.0)

The sending process is quite straightforward. We import the NEST library
and set a useful kernel parameter. On line 6, we create two simple
intergrate-and-fire neuron models, one with a current input of 400mA,
and one with 405mA, just so they will respond differently. If you use
ipython to work interactively, you can check their current status
dictionary with ``neurons.get()``. The definitive
documentation for NEST models is the header file, in this case
``models/iaf_psc_alpha.h`` in the NEST source.

We create a single ``music_event_out_proxy`` for our
output on line 8, and set the port name. We loop over all the neurons on
lines 10-11 and connect them to the proxy one by one, each one with a
different output channel. As we saw earlier, each MUSIC port can have
any number of channels. Since the proxy is a device, it ignores any
weight or delay settings here.

Lastly, we create a spike recorder, set the parameters (which we could
have done directly in the :py:func:`.Create` call) and connect the
neurons to the spike recorder so we can see what we're sending. Then we
simulate for one second.

For the receiving process script, *receive.py* we do:

.. code-block:: python
    :linenos:

    #!/usr/bin/env python3

    import nest
    nest.overwrite_files = True

    music_in = nest.Create("music_event_in_proxy", 2, {'port_name': 'p_in'})

    music_in.music_channel = [c for c in range(len(music_in))]

    nest.SetAcceptableLatency('p_in', 2.0)

    parrots = nest.Create("parrot_neuron", 2)

    srecorder = nest.Create("spike_recorder")
    srecorder.set(record_to="ascii", label="receive")

    nest.Connect(music_in, parrots, 'one_to_one', {"weight":1.0, "delay": 2.0})
    nest.Connect(parrots, srecorder)

    nest.Simulate(1000.0)

The receiving process follows the same logic, but is just a little more
involved. We create two ``music_event_in_proxy`` — one
per channel — on line 6 and set the input port name. As we discussed
above, a NEST node can accept many inputs but only emit one stream of
data, so we need one input proxy per channel to be able to distinguish
the channels from each other. On line 8 we set the input channel for
each input proxy.

:ref:`The SetAcceptableLatency command <music_latency>` on line 10 sets the
maximum time, in milliseconds, that MUSIC is allowed to delay delivery of spikes
transmitted through the named port. This should never be more than the
*minimum* of the delays from the input proxies to their targets; that's
the 2.0 ms we set on line 10 in our case.

On line 12 we create a set of :ref:`parrot neurons <music_parrot>`.
They simply repeat the input they're given. On lines 14-15 we create and
configure a spike recorder to save our inputs. We connect the input proxies
one-to-one with the parrot neurons on line 17, then the parrot neurons to
the spike recorder on line 18. We will discuss the reasons for this in a moment.
Finally we simulate for one second.

Lastly, we have the MUSIC configuration file *python.music*:

.. code-block:: sh

      [from]
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
``receive.py`` above. The first numeral is the node ID of the spike recorder
that recorded and saved the data, and the final numeral is the rank order of
each process that generated the file.

Collate the data files:


.. code-block:: sh

      cat send-*spikes | sort -k 2 -n  >send.spikes
      cat receive-*spikes | sort -k 2 -n  >receive.spikes

We run the files together, and sort the output numerically
(:math:`-n`) by the second column (:math:`-k`). Let's
look at the beginning of the two files side by side:


.. code-block::

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
proxies to the parrot neurons in ``receive.py`` on line 10
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

    #!/usr/bin/python3

    import nest

    mcip = nest.Create('music_cont_in_proxy')
    mcip.port_name = 'contdata'

    time = 0
    while time < 1000:
        nest.Simulate (10)
        data = mcip.get('data')
        print(data)
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
did in our earlier Python example. This is far more effective, and the
outside process is not limited to the generators implemented in NEST but
can create any kind of spiking input. In the next section we will take a
look at how to do this.
.. _music_tutorial_3:

MUSIC Connections in C++ and Python
===================================

The C++ interface
-----------------

The C++ interface is the lowest-level interface and what you would use
to implement a MUSIC interface in simulators. But it is not a
complicated API, so you can easily use it for your own applications that
connect to a MUSIC-enabled simulation.

Let's take a look at a pair of programs that send and receive spikes.
These can be used as inputs or outputs to the NEST models we created
above with no change to the code. C++ code tends to be somewhat
longwinded so we only show the relevant parts here. The C++ interface is
divided into a setup phase and a runtime phase. You can see the setup below.

.. code-block:: cpp

    MPI::Intracomm comm;

    int main(int argc, char **argv)
    {
        MUSIC::Setup* setup = new MUSIC::Setup (argc, argv);
        comm = setup->communicator();

        double simt;                // read simulation time from the
        setup->config ("simtime", &simt);       // MUSIC configuration file

        MUSIC::EventOutputPort *outdata =       // set output port
        setup->publishEventOutput("p_out");

        int nProcs = comm.Get_size();       // Number of mpi processes
        int rank = comm.Get_rank();         // I am this process

        int width = 0;              // Get number of channels
        if (outdata->hasWidth()) {          // from the MUSIC configuration
        width = outdata->width();
        }
        // divide output channels evenly among MPI processes
        int nLocal = width / nProcs;    // Number of channels per process
        int rest = width % nProcs;
        int firstId = nLocal * rank;    // index of lowest ID
        if (rank < rest) {
        firstId += rank;
        nLocal += 1;
        } else
        firstId += rest;

        MUSIC::LinearIndex outindex(firstId, nLocal);   // Create local index
        outdata->map(&outindex, MUSIC::Index::GLOBAL);  // apply index to port

        [ ... continued below ... ]
    }

At lines 5-6 we initialize MUSIC and MPI. The communicator is common to
all processes running under MUSIC, and you’d use it instead of
``COMM_WORLD`` for your :hxt_ref:`MPI` processing.

Lines 7 and 8 illustrate something we haven't discussed so far. We can
set and read free parameters in the MUSIC configuration file. We can for
instance use that to set the simulation time like we do here; although
this is of limited use with a NEST simulation as you can't read these
configuration parameters from within NEST.

We set up an event output port and name it on line 11 and 12, then get
the number of MPI processes and our process rank for later use. In lines
17-19 we read the number of channels specified for this port in the
configuration file. We don't need to set the channels explicitly
beforehand like we do in the NEST interface.

We need to tell MUSIC which channels should be processed by what MPI
processes. Lines 22-29 are the standard way to create a linear index map
from channels to MPI processes. It divides the set of channels into
equal-sized chunks, one per MPI process. If channels don't divide evenly
into processes, the lower-numbered ranks each get an extra channel.
``firstId`` is the index of the lowest-numbered channel for
the current MPI process, and ``nLocal`` is the number of
channels allocated to it.

On lines 31 and 32 we create the index map and then apply it to the
output port we created in line 11. The ``Index::GLOBAL``
parameter says that each rank will refer to its channels by its global
ID number. We could have used ``Index::LOCAL`` and each
rank would refer to their own channels starting with 0. The linear index
is the simplest way to map channels, but there is a permutation index
type that lets you do arbitrary mappings if you want to.

The ``map`` method actually has one more optional argument:
the ``maxBuffered`` argument. Normally MUSIC decides on its
own how much event data to buffer on the receiving side before actually
transmitting it. It depends on the connection structure, the amount of
data that is generated and other things. But if you want, you can set
this explicitly:

.. code-block:: cpp

    outdata->map(&outindex, MUSIC::Index::GLOBAL, maxBuffered)

With a ``maxBuffered`` value of 1, for instance, MUSIC will
send emitted spike events every cycle. With a value of 2 it would send
data every other cycle. This parameter can be necessary if the receiving
side is time-sensitive (perhaps the input controls some kind of physical
hardware), and the data needs to arrive as soon as possible.

.. code-block:: cpp


    [ ... continued from above ... ]

    // Start runtime phase
    MUSIC::Runtime runtime = MUSIC::Runtime(setup, TICK);
    double tickt =  runtime.time();

    while (tickt < simt) {
    for (int idx = firstId; idx<(firstId+nLocal); idx++) {
        // send poisson spikes to every channel.
        send_poisson(outdata, RATE*(idx+1), tickt, idx);
    }
    runtime.tick();         // Give control to MUSIC
    tickt = runtime.time();
    }
    runtime.finalize();         // clean up and end

    }

    double frand(double rate) {return -(1./rate)*log(random()/double(RAND_MAX));}

    void send_poisson(MUSIC::EventOutputPort* outport,
              double rate, double tickt, int index) {
        double t = frand(rate);
        while (t<TICK) {
        outport -> insertEvent(tickt+t, MUSIC::GlobalIndex(index));
        t = t + frand(rate);
        }
    }

The runtime phase is short. On line 4 we create the MUSIC runtime
object, and let it consume the setup. In the runtime loop on lines 7-14
we output data, then give control to MUSIC by its
``tick()`` function so it can communicate, until the
simulation time exceeds the end time.

``runtime.time()`` on lines 5 and 13 gives us the current
time according to MUSIC. In lines 8-10 we loop through the channel
indexes corresponding to our own rank (that we calculated during setup),
and call a function defined from line 20 onwards that generates a
poisson :hxt_ref:`spike train` with the rate we request.

The actual event insertion happens on line 24, and we give it the time
and the global index of the channel we target. The loop on line 8 loops
through only the indexes that belong to this rank, but that is only for
performance. We could loop through all channels and send events to all
of them if we wanted; MUSIC will silently ignore any events targeting a
channel that does not belong to the current rank.

``runtime.tick()`` gives control to MUSIC. Any inserted
events will be sent to their destination, and any new incoming events
will be received and available once the method returns. Be aware that
this call is blocking and could take an arbitrary amount of time, if
MUSIC has to wait for another simulation to catch up. If you have other
time-critical communications you will need to put them in a different
thread.

Once we reach the end of the simulation we call
``runtime.finalize()``. Music will shut down the
communications and clean up after itself before exiting.

.. code-block:: cpp

    MPI::Intracomm comm;
    FILE *fout;

    struct eventtype  {
        double t;
        int id;
    };
    std::queue <eventtype> in_q;

    class InHandler : public MUSIC::EventHandlerGlobalIndex {
        public:
        void operator () (double t, MUSIC::GlobalIndex id) {
            struct eventtype ev = {t, (int)id};
            in_q.push(ev);
        }
    };

    int main(int argc, char **argv)
    {
        MUSIC::Setup* setup = new MUSIC::Setup (argc, argv);
        comm = setup->communicator();

        double simt;
        setup->config ("simtime", &simt);

        MUSIC::EventInputPort *indata =
        setup->publishEventInput("p_in");

        InHandler inhandler;

        [ ... get processes, rank and channel width as in send.cpp ... ]

        char *fname;
        int dummy = asprintf(&fname, "output-%d.spk", rank);
        fout = fopen(fname, "w");

        [ ... calculate channel allocation as in send.cpp ... ]

        MUSIC::LinearIndex inindex(firstId, nLocal);
        indata->map(&inindex, &inhandler, IN_LATENCY);
    }

The setup phase for the reveiving application is mostly the same as the
sending one. The main difference is that we receive events through a
callback function that we provide. During communication, MUSIC will call
that function once for every incoming event, and that function stores
those events until MUSIC is done and we can process them.

For storage we define a structure to hold time stamp and ID pairs on
lines 4-7, and a queue of such structs on line 8. Lines 10-14 defines
our callback function. The meat of it is lines 13-14, where we create a
new event struct instance with the time stamp and ID we received, then
push the structure onto our queue.

The actual setup code follows the same pattern as before: we create a
setup object, get ourself a communicator, read any config file
parameters and create a named input port. We also declare an instance of
our callback event handler on line 29. We get our process and rank
information and calculate our per-rank channel allocation in the exact
same way as before.

The map for an input port that we create on line 40 needs two additional
parameters that the output port map did not. We give it a reference to
our callback function that we defined earlier. When events appear on the
port, they get passed to the callback function. It also has an optional
latency parameter. This is the same latency that we set with the
separate :py:func:`.SetAcceptableLatency` function in the NEST
example earlier, and it works the same way. Just remember that the MUSIC
unit of time is seconds, not milliseconds.

.. code-block:: cpp


    int main(int argc, char **argv)
    {
        MUSIC::Runtime runtime = MUSIC::Runtime(setup, TICK);
        double tickt = runtime.time();

        while (tickt < simt) {
        runtime.tick();     // Give control to MUSIC
        tickt = runtime.time();
        while (!in_q.empty()) {
            struct eventtype ev = in_q.front();
            fprintf (fout, "%d\t%.4f\n", ev.id, ev.t);
            in_q.pop();
        }
        }
        fclose(fout);
        runtime.finalize();
    }

The runtime is short. As before, we create a runtime object that consumes
the setup, then we loop until the MUSIC time exceeds our simulation
time. We call ``runtime.tick()`` each time through the loop
on line 8 and we process received events after the call to
``tick()``. If you had a process with both sending and
receiving ports, you would submit the sending data before the
``tick()`` call, and process the receiving data after it in
the same loop.

The ``in_q`` input queue we defined earlier holds any new
input events. We take the first element on line 10, then process it — we
write it out to a file — and finally pop it off the queue. When the
queue is empty we're done and go back around the main loop again.

Lastly we call ``runtime.finalize()`` as before.

Building the Code
~~~~~~~~~~~~~~~~~

We have to build our ``C++`` code. The example code is
already set up for the GNU Autotools, just to show how to do this for a
MUSIC project. There's only two build-related files we need to care
about (all the rest are autogenerated), ``configure.ac``
and ``Makefile.am``.

.. code-block::

    AC_INIT(simple, 1.0)
    AC_PREREQ([2.59])
    AM_INIT_AUTOMAKE([1.11 -Wall subdir-objects no-define foreign])
    AC_LANG([C++])
    AC_CONFIG_HEADERS([config.h])
    dnl # set OpenMPI compiler wrapper
    AC_PROG_CXX(mpicxx)
    AC_CHECK_LIB([music], [_init])
    AC_CHECK_HEADER([music.hh])
    AC_CONFIG_FILES([Makefile])
    AC_OUTPUT

The first three lines set the project name and version, the minimum
version of autotools we require and a list of options for Automake. Line
4 sets the current language, and line 5 that we want a config.h file.

Line 7 tells autoconf to use the ``mpicxx`` MPI wrapper as
the C++ compiler. Lines 8-9 tells it to test for the existence of the
``music`` library, and look for the
``music.hh`` include file.

.. code-block:: cpp

    bin_PROGRAMS = send recv
    send_SOURCES = send.cpp
    recv_SOURCES = recv.cpp

``Makefile.am`` has only three lines:
``bin_PROGRAMS`` lists the binaries we want to build.
``send_SOURCES`` and ``recv_SOURCES`` lists
the source files each one needs.

Your project should already be set up, but if you start from nothing,
you need to generate the rest of the build files. You'll need the
Autotools installed for that. The easiest way to generate all build
files is to use ``autoreconf``:

.. code-block:: sh

      autoreconf --install --force

Then you can build with the usual sequence of commands:

.. code-block:: sh

      ./configure
      make

Try the Code
~~~~~~~~~~~~

We can run these programs just like we did with the NEST example, using
a Music configuration file:

.. code-block:: cpp


    simtime=1.0
    [from]
      binary=./send
      np=2
    [to]
      binary=./recv
      np=2

      from.p_out -> to.p_in [2]

The structure is just the same as before. We have added a
``simtime`` parameter for the two applications to read, and
the binaries are our two new programs. We run this the same way:

.. code-block:: sh

    mpirun -np 4 music simple.music

You can change the simulation time by changing the
``simtime`` parameter at the top of the file. Also, these
apps are made to deal with any number of channels, so you can change
``[2]`` to anything you like. If you have more channels
than MPI processes for the ``recv`` app you will get more
than one channel recorded per output file, just as the channel
allocation code specified. If you have more MPI processes than input
channels, some output files will be empty.

You can connect these with the NEST models that we wrote earlier. Copy
them into the same directory. Then, in the ``cpp.music``
config file, change the ``binary`` parameter in
``[from]`` from ``binary=./send`` to
``binary=./send.py``. You get two sets of output files.
Concatenate them as before, and compare:

.. code-block:: cpp


    send.py            recv

    2   26.100         1    0.0261
    1   27.800         0    0.0278
    2   54.200         1    0.0542
    1   57.600         0    0.0576
    2   82.300         1    0.0823
    1   87.400         0    0.0874
    2   110.40         1    0.1104

Indeed, we get the expected result. The IDs from the ``python`` process on
the left are the originating neurons; the IDs on the right is the MUSIC
channel on the receiving side. And of course NEST deals in milliseconds
while MUSIC uses seconds.

This section has covered most things you need in order to use it for
straightforward user-level input and output applications. But there is a
lot more to the MUSIC API, especially if you intend to implement it as a
simulator interface, so you should consult the documentation for more
details.
.. _music_tutorial_4:

The pymusic interface
---------------------

MUSIC has recently acquired a `plain Python interface <https://github.com/INCF/MUSIC/tree/master/pymusic>`_
to go along with the C++ API. If you just want to connect with a simulation
rather than adding MUSIC capability to a simulator, this Python interface can
be a lot more convenient than C++. You have Numpy, Scipy and other high-level
libraries available, and you don't need to compile anything.

The interface is closely modelled on the C++ API; indeed, the steps to
use it is almost exactly the same. You can mostly refer to the :ref:`C++
description <music_tutorial_3>` for explanation. Below we will only highlight the
differences to the C++ API. The full example code is in the
``pymusic`` directory in the MUSIC repository.

.. code-block:: python

    #!/usr/bin/env python3
    import music

    [ ... ]

    outdata.map(music.Index.GLOBAL,
        base=firstId,
            size=nlocal)

    [ ...]

    runtime = setup.runtime(TICK)
    tickt = runtime.time()
    while tickt < simtime:

        for i in range(width):
        send_poisson(outdata, RATE, tickt, i)

        runtime.tick()
        tickt = runtime.time()

The sending code is almost completely identical to its C++ counterpart.
Make sure ``python3`` is used as interpreter for the code (and make sure this
file is executable). Import ``music`` in the expected way.

Unlike the C++ API, the index is not an object, but simply a label
indicating global or local indexing. The ``map()`` call
thus need to get the first ID and the number of elements mapped to this
rank directly. Also note that the ``map()`` functions have
a somewhat unexpected parameter order, so it's best to use named
parameters just in case.

The runtime looks the same as the C++ counterpart as well. We get the
current simulation time, and repeatedly send new sets of events as long
as the current time is smaller than the simulation time.

.. code-block:: python

    import Queue

    in_q = Queue.Queue()

    # Our input handler function
    def inhandler(t, indextype, channel_id):
        in_q.put([t, channel_id])

        [ ... ]

    indata.map(inhandler,
            music.Index.GLOBAL,
            base=firstId,
            size=nlocal,
            accLatency=IN_LATENCY)

    tickt = runtime.time()
    while tickt < simtime:

        runtime.tick()
        tickt = runtime.time()

        while not in_q.empty():
            ev = in_q.get()
            f.write("{0}\t{1:8.4f}\n".format(ev[1], ev[0]))

Here is the structure for the receiving process, modelled on the C++
code. We use a Python ``Queue``  class to implement
our event queue.

The input handler function has signature
``(float time, int indextype, int channel_id)``. The
:hxt_ref:`time` and ``channel_id`` are the event
times and IDs as before. The ``indextype`` is the type of
the map index for this input and is ``music.Index.LOCAL``
or ``music.Index.GLOBAL``.

The ``map()`` function keyword for acceptable latency is
``accLatency``, and the ``maxBuffered``
keyword we mentioned in the previous section is, unsurprisingly,
``maxBuffered``. The runtime is, again, the same as for
C++.

As the ``pymusic`` bindings are still quite new, the
documentation is still lagging behind. This quick introduction should nevertheless be enough for you
to get going with the bindings. Feel free to ask our `Mailing List <https://www.nest-initiative.org/mailinglist/>`_
if you need further help.
:orphan:

.. _music_sli:

MUSIC with SLI
==============

SLI is the built-in scripting language in NEST. It is a stack-based
language loosely modelled on PostScript. It is quite cumbersome to work
with, is not extensively documented, and has quite a few corner cases,
design issues and unintended behaviours. But it is small, already built
into NEST and is much more memory efficient than using Python. If your
models are very large and memory is tight, or you are using a system
where Python isn't available, then SLI is the way to go.

We won't discuss the code extensively as learning SLI is really outside
the scope of this tutorial. The code follows the same structure as the
other examples, and should be straightforward to follow. But we will
give a few pointers for how to connect things with MUSIC.

The SLI version of the sending process file from
:ref:`Part 2 of the MUSIC tutorial <music_tutorial_2>`, *sender.sli*, is outlined
below. Comments are prefixed with a "%".

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
`p_out`. Dictionaries are bracketed with "<<" and ">>",
and strings are bracketed with parenthesis.

On lines 9-13 we iterate over the range of all neurons and store the index
in `index`. Then we connect each neuron in the NodeCollection to the output
proxy with its own music channel. To get the individual node we use ``Take``.
Note that we use ``Set`` to assign the index on the stack
to a variable. We'd have to rotate the top stack elements if we wanted to
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
.. _tips_music:

Practical tips
==============

Start MUSIC using mpirun
------------------------

    There is an alternative way to start a MUSIC simulation without the ``music``
    binary. The logic for parsing the configuration file is built into
    the library itself. So we can start each binary explicitly using
    mpirun. We give the config file name and the corresponding app label
    as command line options:

    .. code-block:: sh

          mpirun -np 2 <binary> --music_config <config file> --app-label <label> : ...

    So to start a simulation with the sendsimple.py and recv programs,
    we can do:

    .. code-block:: sh

          mpirun -np 2 ./sendsimple.py --music-config simplepy.music --app-label from :/
             -np 2 ./recv --music-config simplepy.music --app-label to

    This looks long and cumbersome, of course, but it can be useful.
    Since it's parsed by the shell you are not limited to what the
    ``music`` launcher can parse, but the binary can be
    anything the shell can handle, including an explicit interpreter
    invocation or a shell script.

    As a note, the config file no longer needs to contain the right
    binary names. But it *does* need to have a non-empty
    ``binary=<something>`` line for each process. The
    parser expects it and will complain (or crash) otherwise. Also, if
    you try to process comand line options in your Pynest script, it is
    very likely you will confuse MUSIC.


Disable messages
----------------

    NEST can be quite chatty as it connects things, especially with large
    networks. If we don't want all that output, we can tell it to display only
    error messages:

    .. code:: python

        nest.set_verbosity("M_ERROR")

    There is unfortunately no straightforward way to suppress the
    initial welcome message. That is somewhat unfortunate, as they add
    up quickly in the output of a simulation when you use more than a
    few hundred cores.

Comma as decimal point
----------------------

    Sorting output spikes may fail if you, like the authors, come from a
    country that uses a comma as decimal separator and runs your computer in
    your native language. The problem is that sort respects the language
    settings and expects the decimal separator to be a comma. When it sees the
    decimal point in the input it assumes the numeral has ended and sorts only
    on the integer part.

    The way to fix this is to set ``LC\_ALL=C`` before
    running the sort command. In a script or in the terminal you can do:

    .. code-block:: sh

          export LC_ALL=C
          cat output-*|sort -k 2 -n >output.spikes

    Or, if you want to do this temporarily for only one command:

    .. code-block:: sh

          cat output-*|LC_ALL=C sort -k 2 -n >output.spikes

Build Autotool-enable project
-----------------------------

    To build an Autotool-enabled C/C++ project, you don't actually need to
    be in the main directory. You can create a subdirectory and build
    everything from there. For instance, with the simple C++ MUSIC project
    in section :ref:`C++ build <music_tutorial_3>`, we can do this:

    .. code-block:: sh

          mkdir build
          cd build
          ../configure
          make

    Why do that? Because all files you generate when building the
    project ends up under the ``build`` subdirectory,
    keeping the source directories completely clean and untouched. You
    can have multiple builds ``debug``,
    ``noMPI`` and so on with different build options
    enabled, and you can completely clean out a build simply by deleting
    the directory.

    This is surely completely obvious to many of you, but this author is
    almost ashamed to admit just how many years it took before I
    realized you could do this. I sometimes actually kept two copies of
    projects checked out just so I could build a separate debug version.
