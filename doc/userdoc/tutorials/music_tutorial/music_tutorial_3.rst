MUSIC Connections in C++ and Python
===================================

The C++ interface
-----------------

The C++ interface is the lowest-level interface and what you would use
to implement a MUSIC interface in simulators. But it is not a
complicated API, so you can easily use it for your own applications that
connect to a MUSIC-enabled simulation.

Let’s take a look at a pair of programs that send and receive spikes.
These can be used as inputs or outputs to the NEST models we created
above with no change to the code. C++ code tends to be somewhat
longwinded so we only show the relevant parts here. The C++ interface is
divided into a setup phase and a runtime phase. You can see the setup below. 

.. note::

   Please note that MUSIC and the recording backend for Arbor are mutually exclusive
   and cannot be enabled at the same time.

.. code-block:: cpp
    :linenos:

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
``COMM_WORLD`` for your MPI processing.

Lines 7 and 8 illustrate something we haven’t discussed so far. We can
set and read free parameters in the MUSIC configuration file. We can for
instance use that to set the simulation time like we do here; although
this is of limited use with a NEST simulation as you can’t read these
configuration parameters from within NEST.

We set up an event output port and name it on line 11 and 12, then get
the number of MPI processes and our process rank for later use. In lines
17-19 we read the number of channels specified for this port in the
configuration file. We don’t need to set the channels explicitly
beforehand like we do in the NEST interface.

We need to tell MUSIC which channels should be processed by what MPI
processes. Lines 22-29 are the standard way to create a linear index map
from channels to MPI processes. It divides the set of channels into
equal-sized chunks, one per MPI process. If channels don’t divide evenly
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
    :linenos:

    outdata->map(&outindex, MUSIC::Index::GLOBAL, maxBuffered)

With a ``maxBuffered`` value of 1, for instance, MUSIC will
send emitted spike events every cycle. With a value of 2 it would send
data every other cycle. This parameter can be necessary if the receiving
side is time-sensitive (perhaps the input controls some kind of physical
hardware), and the data needs to arrive as soon as possible.

.. code-block:: cpp
    :linenos:


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
poisson spike train with the rate we request.

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
    :linenos:

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
separate ``SetAcceptableLatency`` function in the NEST
example earlier, and it works the same way. Just remember that the MUSIC
unit of time is seconds, not milliseconds.

.. code-block:: cpp
    :linenos:


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
queue is empty we’re done and go back around the main loop again.

Lastly we call ``runtime.finalize()`` as before.

Building the Code
~~~~~~~~~~~~~~~~~

We have to build our ``C++`` code. The example code is
already set up for the GNU Autotools, just to show how to do this for a
MUSIC project. There’s only two build-related files we need to care
about (all the rest are autogenerated), ``configure.ac``
and ``Makefile.am``.

.. code-block:: cpp
    :linenos:

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
    :linenos:

    bin_PROGRAMS = send recv
    send_SOURCES = send.cpp
    recv_SOURCES = recv.cpp

``Makefile.am`` has only three lines:
``bin_PROGRAMS`` lists the binaries we want to build.
``send_SOURCES`` and ``recv_SOURCES`` lists
the source files each one needs.

Your project should already be set up, but if you start from nothing,
you need to generate the rest of the build files. You’ll need the
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
    :linenos:


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
    :linenos:


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

