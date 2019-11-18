Using NEST with MUSIC
=====================

Introduction
------------

NEST supports the `MUSIC interface
<http://software.incf.org/software/music>`__, a standard by
the INCF, which allows the transmission of data between applications at
runtime [1]_. It can be used to couple NEST with other simulators, with
applications for stimulus generation and data analysis and visualization
and with custom applications that also use the MUSIC interface.

Basically, all communication with MUSIC is mediated via *proxies* that
receive/send data from/to remote applications using MUSIC. Different
proxies are used for the different types of data. At the moment, NEST
supports sending and receiving spike events and receiving continuous
data and string messages.

You can find the installation instructions for MUSIC on their Github Page:
`INCF/MUSIC <https://github.com/INCF/MUSIC/>`__

Reference
~~~~~~~~~~~

.. [1] Djurfeldt M, et al. 2010. Run-time interoperability between neuronal
 simulators based on the MUSIC framework. Neuroinformatics, 8.
 `doi:10.1007/s12021-010-9064-z*
 <http://www.springerlink.com/content/r6j425027lmv1251/>`__.

Sending and receiving spike events
----------------------------------

A minimal example for the exchange of spikes between two independent
instances of NEST is given in the example
``examples/nest/music/minimalmusicsetup.music``.

It sends spikes using the ``music_event_out_proxy`` script and receives the
spikes using a ``music_event_in_proxy``.

::

    stoptime=0.01

    [from]
      binary=./minimalmusicsetup_sendnest.py
      np=1

    [to]
      binary=./minimalmusicsetup_receivenest.py
      np=1

    from.spikes_out -> to.spikes_in [1]

This configuration file sets up two applications, ``from`` and ``to``,
which are both instances of NEST. The first runs a script to send
spike events on the MUSIC port ``spikes_out`` to the second, which
receives the events on the port ``spikes_in``. The width of the port is
1.

The content of ``minimalmusicsetup_sendnest.py`` is contained in the
following listing.


First, we import nest and set up a check to ensure MUSIC is installed before
continuing.

::

   import nest

   nest.sli_run("statusdict/have_music ::")
   if not nest.spp():
       import sys

       print("NEST was not compiled with support for MUSIC, not running.")
       sys.exit()

   nest.set_verbosity("M_ERROR")

Next we create a ``spike_generator`` and set the spike times. We then create
our neuron model (``iaf_psc_alpha``) and connect the neuron with the spike
generator.

::

   sg = nest.Create('spike_generator')
   nest.SetStatus(sg, {'spike_times': [1.0, 1.5, 2.0]})

   n = nest.Create('iaf_psc_alpha')

   nest.Connect(sg, n, 'one_to_one', {'weight': 750.0, 'delay': 1.0})

We then create a voltmeter, which will measure the membrane potenial, and
connect it with the neuron.

::

   vm = nest.Create('voltmeter')
   nest.SetStatus(vm, {'record_to': 'screen'})

   nest.Connect(vm, n)

Finally, we  create a ``music_event_out_proxy``, which forwards the spikes it
receives directly to the MUSIC event output port ``spikes_out``. The spike
generator is connected to the ``music_event_out_proxy`` on channel 0 and the
network is simulated for 10 milliseconds.

::

   meop = nest.Create('music_event_out_proxy')
   nest.SetStatus(meop, {'port_name': 'spikes_out'})

   nest.Connect(sg, meop, 'one_to_one', {'music_channel': 0})

   nest.Simulate(10)


The next listing contains the content of
``minimalmusicsetup_receivenest.py``, which is set up similarly to the above
script, but without the spike generator.

::

  import nest

  nest.sli_run("statusdict/have_music ::")
  if not nest.spp():
      import sys

      print("NEST was not compiled with support for MUSIC, not running.")
      sys.exit()

  nest.set_verbosity("M_ERROR")

  meip = nest.Create('music_event_in_proxy')
  nest.SetStatus(meip, {'port_name': 'spikes_in', 'music_channel': 0})

  n = nest.Create('iaf_psc_alpha')

  nest.Connect(meip, n, 'one_to_one', {'weight': 750.0})

  vm = nest.Create('voltmeter')
  nest.SetStatus(vm, {'record_to': 'screen'})

  nest.Connect(vm, n)

  nest.Simulate(10)


Running the example using ``mpirun -np 2 music minimalmusicsetup.music``
yields the following output, which shows that the neurons in both
processes receive the same input from the ``spike_generator`` in the
first NEST process and show the same membrane potential trace.

::

    NEST v1.9.svn (C) 1995-2008 The NEST Initiative
    -70
    -70
    -68.1559
    -61.9174
    -70
    -70
    -70
    -65.2054
    -62.1583

    NEST v1.9.svn (C) 1995-2008 The NEST Initiative
    -70
    -70
    -68.1559
    -61.9174
    -70
    -70
    -70
    -65.2054
    -62.1583

Receiving string messages
-------------------------

Currently, NEST is only able to receive messages, and unable to send string
messages. We thus use MUSIC's ``messagesource`` program for the
generation of messages in the following example. The configuration file
(``msgtest.music``) is shown below

::

    stoptime=1.0
    np=1
    [from]
      binary=messagesource
      args=messages
    [to]
      binary=./msgtest.py

    from.out -> to.msgdata [0]

This configuration file connects MUSIC's ``messagesource`` program to
the port ``msgdata`` of a NEST instance. The ``messagesource`` program
needs a data file, which contains the messages and the corresponding
time stamps. For this example, we use the data file, ``messages0.dat``:

::

    0.3     Hello
    0.7     !

.. note::

  In MUSIC, the default unit for time is seconds for the specification
  of times, while NEST uses miliseconds.

The script that sets up the receiving side (``msgtest.py``)
of the example is shown in the following script.

We first import NEST and create an instance of the ``music_message_in_proxy``.
We then set the name of the port it listens on to ``msgdata``. The network is
simulated  in steps of 10 ms.

::

    #!/usr/bin/python

    import nest

    mmip = nest.Create ('music_message_in_proxy')
    nest.SetStatus (mmip, {'port_name' : 'msgdata'})

    # Simulate and get message data with a granularity of 10 ms:
    time = 0
    while time < 1000:
        nest.Simulate (10)
        data = nest.GetStatus(mmip, 'data')
        print data
        time += 10


We then run the example using

::

  mpirun -np 2 music msgtest.music

which yields the following output:

::

               -- N E S T 2 beta --
              Neural Simulation Tool
      Copyright 1995-2009 The NEST Initiative
       Version 1.9-svn Sep 22 2010 16:50:01

    This program is provided AS IS and comes with
    NO WARRANTY. See the file LICENSE for details.

    Problems or suggestions?
      Website     : <a class="external free" href="https://www.nest-initiative.org" rel="nofollow">https://www.nest-initiative.org</a>
      Mailing list: nest_user@nest-initiative.org

    Type 'nest.help()' to find out more about NEST.

    Sep 23 16:09:12 Simulate [Info]:
        Simulating 10 ms.

    Sep 23 16:09:12 Scheduler::prepare_nodes [Info]:
        Please wait. Preparing elements.

    Sep 23 16:09:12 music_message_in_proxy::calibrate() [Info]:
        Mapping MUSIC input port 'msgdata' with width=0 and acceptable latency=0
        ms.

    Sep 23 16:09:12 Scheduler::prepare_nodes [Info]:
        Simulating 1 nodes.

    Sep 23 16:09:12 Scheduler::resume [Info]:
        Entering MUSIC runtime with tick = 0.1 ms

    Sep 23 16:09:12 Scheduler::resume [Info]:
        Simulation finished.
    [{'messages': [], 'message_times': array([], dtype=float64)}]

    :

    Sep 23 16:13:36 Simulate [Info]:
        Simulating 10 ms.

    Sep 23 16:13:36 Scheduler::prepare_nodes [Info]:
        Please wait. Preparing elements.

    Sep 23 16:13:36 Scheduler::prepare_nodes [Info]:
        Simulating 1 nodes.

    Sep 23 16:13:36 Scheduler::resume [Info]:
        Simulation finished.
    [{'messages': ['Hello', '!'], 'message_times': array([ 300.,  700.])}]

Receiving continuous data
-------------------------

As in the case of string message, NEST currently only supports receiving
continuous data, but not sending. This means that we have to use another
of MUSIC's test programs to generate the data for us. This time, we use
``constsource``, which generates a sequence of numbers form 0 to w,
where w is the width of the port. The MUSIC configuration file
(``conttest.music``) is shown in the following listing:

::

    stoptime=0.01

    [from]
      binary=./minimalmusicsetup_sendnest.py
      np=1

    [to]
      binary=./minimalmusicsetup_receivenest.py
      np=1

    from.spikes_out -> to.spikes_in [1]

::

    stoptime=1.0
    [from]
    np=1
    binary=./cont_out.py
    [to]
    np=1
    binary=./cont_in.py

    from.cont_out -> to.cont_in [10]

The receiving side is again implemented using a
:doc:`PyNEST <../tutorials/index>` script (``conttest.py``).
We first import the NEST and create an instance of the
``music_cont_in_proxy``. we set the name of the port
it listens on to ``msgdata``. We then simulate the network in
steps of 10 ms.

::

    #!/usr/bin/python

    import nest

    mcip = nest.Create('music_cont_in_proxy')
    nest.SetStatus(mcip, {'port_name' : 'cont_in'})

    # Simulate and get vector data with a granularity of 10 ms:
    time = 0
    while time < 1000:
       nest.Simulate (10)
       data = nest.GetStatus (mcip, 'data')
       print data
       time += 10

The example is run using

::

  mpirun -np 2 music conttest.music

which yields the following output:

::

               -- N E S T 2 beta --
              Neural Simulation Tool
      Copyright 1995-2009 The NEST Initiative
       Version 1.9-svn Sep 22 2010 16:50:01

    This program is provided AS IS and comes with
    NO WARRANTY. See the file LICENSE for details.

    Problems or suggestions?
      Website     : <a class="external free" href="https://www.nest-initiative.org" rel="nofollow">https://www.nest-initiative.org</a>
      Mailing list: nest_user@nest-initiative.org

    Type 'nest.help()' to find out more about NEST.

    Sep 23 16:49:09 Simulate [Info]:
        Simulating 10 ms.

    Sep 23 16:49:09 Scheduler::prepare_nodes [Info]:
        Please wait. Preparing elements.

    Sep 23 16:49:09 music_cont_in_proxy::calibrate() [Info]:
        Mapping MUSIC input port 'contdata' with width=10.

    Sep 23 16:49:09 Scheduler::prepare_nodes [Info]:
        Simulating 1 nodes.

    Sep 23 16:49:09 Scheduler::resume [Info]:
        Entering MUSIC runtime with tick = 0.1 ms

    Sep 23 16:49:09 Scheduler::resume [Info]:
        Simulation finished.
    [array([ 0.,  1.,  2.,  3.,  4.,  5.,  6.,  7.,  8.,  9.])]

    :

    Sep 23 16:47:24 Simulate [Info]:
        Simulating 10 ms.

    Sep 23 16:47:24 Scheduler::prepare_nodes [Info]:
        Please wait. Preparing elements.

    Sep 23 16:47:24 Scheduler::prepare_nodes [Info]:
        Simulating 1 nodes.

    Sep 23 16:47:24 Scheduler::resume [Info]:
        Simulation finished.
    [array([ 0.,  1.,  2.,  3.,  4.,  5.,  6.,  7.,  8.,  9.])]

