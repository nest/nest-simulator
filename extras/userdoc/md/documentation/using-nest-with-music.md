# Using NEST with MUSIC

## Introduction

NEST supports the [MUSIC interface](http://software.incf.org/software/music), a
standard by the INCF, which allows the transmission of data between applications
at runtime. It can be used to couple NEST with other simulators, with
applications for stimulus generation and data analysis and visualization and
with custom applications that also use the MUSIC interface.

Basically, all communication with MUSIC is mediated via *proxies* that
receive/send data from/to remote applications using MUSIC. Different proxies are
used for the different types of data. At the moment, NEST supports sending and
receiving spike events and receiving continuous data and string messages.

### Publication

The implementation of the MUSIC interface for NEST is published as
*Mikael Djurfeldt, Johannes Hjorth, Jochen Martin Eppler, Niraj Dudani,
Moritz Helias, Tobias C Potjans, Upinder S Bhalla, Markus Diesmann,
Jeanette Hellgren Kotaleski, and Örjan Ekeberg. Run-time interoperability
between neuronal simulators based on the MUSIC framework.
Neuroinformatics, 8, 2010. doi:10.1007/s12021-010-9064-z* and available from [here](http://www.springerlink.com/content/r6j425027lmv1251/).

## Sending and receiving spike events

A minimal example for the exchange of spikes between two independent instances
of NEST is given in the example `examples/nest/music/minimalmusicsetup.music`.
It uses one [SLI script](an-introduction-to-sli.md), which sends spikes using a
`music_event_out_proxy` and one SLI script, which receives the spikes using a
`music_event_in_proxy`. The configuration file is shown in the following
listing:

    [from]
      binary=nest
      np=1
      args=minimalmusicsetup_sendnest.sli

    [to]
      binary=nest
      np=1
      args=minimalmusicsetup_receivenest.sli
      from.spikes_out -> to.spikes_in [1]

This configuration file sets up two applications, `from` and `to`, which are
both instances of `NEST`. The first runs a script to send spike events on the
MUSIC port `spikes_out` to the second, which receives the events on the port
`spikes_in`. The width of the port is 1. The content of
`minimalmusicsetup_sendnest.sli` is contained in the following listing:

    /spike_generator Create /sg Set
    sg << /spike_times [1.0 1.5 2.0 ]>> SetStatus
    /iaf_psc_alpha Create /n Set
    sg n << /weight 750.0 >> Connect
    /voltmeter Create /vm Set
    vm << /record_to [ /screen ] >> SetStatus
    vm n Connect
    /music_event_out_proxy Create /meop Set
    meop << /port_name (spikes_out) >> SetStatus
    sg meop << /music_channel 0 >> Connect
    10 Simulate

Line 1 creates a `spike_generator`, which sends three spikes. The spike times
are specified in line 2. The script then creates an `iaf_psc_alpha` in line 3 and
connects the `spike_generator` to the `iaf_psc_alpha` in line 4. The membrane
potential of the `iaf_psc_alpha` is measured by a `voltmeter`, which is created in
line 5 and set to print the measured values in line 6. The connection between
the `voltmeter` and the `iaf_psc_alpha` is established in line 7. Line 8 creates a
`music_event_out_proxy`, which forwards the spikes it receives directly to the
MUSIC event output port `spikes_out` (set in line 9). The `spike_generator` is
connected to the MUSIC channel 0 on the `music_event_out_proxy` in line 10.
Finally, the network is simulated for 10 miliseconds.

The next listing contains the content of `minimalmusicsetup_receivenest.sli`:

/music_event_in_proxy Create /meip Set
meip << /port_name (spikes_in) /music_channel 0 >> SetStatus
/iaf_psc_alpha Create /n Set
meip n << /weight 750.0 >> Connect
/voltmeter Create /vm Set
vm << /record_to [ /screen ] >> SetStatus
vm n Connect
10 Simulate

Running the example using `mpirun -np 2 music minimalmusicsetup.music` yields
the following output, which shows that the neurons in both processes receive the
same input from the `spike_generator` in the first NEST process and show the
same membrane potential trace.

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

## Receiving string messages

Currently, NEST is only able to receive, but not to send string messages. We
thus use MUSIC's `messagesource` program for the generation of messages in the
following example. The configuration file (`msgtest.music`) is shown in the
following listing:

    stoptime=1.0
    np=1
    [from]
      binary=messagesource
      args=messages
    [to]
      binary=./msgtest.py

    from.out -> to.msgdata [0]

This configuration file connects MUSIC's `messagesource` program to the port
`msgdata` of a NEST instance. The `messagesource` program needs a data file,
which contains the messages and the corresponding time stamps. The data file
(`messages0.dat`) is shown in the following listing:

    0.3     Hello
    0.7     !

Please note that MUSIC uses a default unit of seconds for the specification of
times, while NEST uses miliseconds. The example uses the [PyNEST](introduction-to-pynest.md)
syntax instead of SLI for the NEST part. The script that sets up the receiving
side (`msgtest.py`) of the exampe is shown in the following listing:

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

We first import the `nest` in line 2 and create an instance of the
`music_message_in_proxy` in line 3. In line 4, we set the name of the port it
listens to to `msgdata`. Lines 6 through 11 simulate the network in steps of
10 ms. Running the example using `mpirun -np 2 music msgtest.music` yields the
following output:

               -- N E S T 2 beta --
              Neural Simulation Tool
      Copyright 1995-2009 The NEST Initiative
       Version 1.9-svn Sep 22 2010 16:50:01

    This program is provided AS IS and comes with
    NO WARRANTY. See the file LICENSE for details.

    Problems or suggestions?
      Website     : <a class="external free" href="http://www.nest-initiative.org" rel="nofollow">http://www.nest-initiative.org</a>
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

## Receiving continuous data

As in the case of string message, NEST currently only supports receiving
continuous data, but not sending. This means that we have to use another of
MUSIC's test programs to generate the data for us. This time, we use
`constsource`, which generates a sequence of numbers form 0 to w, where w is the
width of the port. The MUSIC configuration file (`conttest.music`) is shown in
the following listing:

    stoptime=1.0
    [from]
    np=1
    binary=constsource
    [to]
    np=1
    binary=./conttest.py

    from.contdata -> to.contdata [10]

The receiving side is again implemented using a [PyNEST](introduction-to-pynest.md)
script (`conttest.py`):

    #!/usr/bin/python

    import nest

    mcip = nest.Create('music_cont_in_proxy')
    nest.SetStatus(mcip, {'port_name' : 'contdata'})

    # Simulate and get vector data with a granularity of 10 ms:
    time = 0
    while time < 1000:
       nest.Simulate (10)
       data = nest.GetStatus (mcip, 'data')
       print data
       time += 10

We first import the nest in line 2 and create an instance of the
music\_cont\_in\_proxy in line 3. In line 4, we set the name of the port it
listens to to msgdata. Lines 6 through 11 simulate the network in steps of
10 ms. Running the example using `mpirun -np 2 music conttest.music` yields the
following output:

               -- N E S T 2 beta --
              Neural Simulation Tool
      Copyright 1995-2009 The NEST Initiative
       Version 1.9-svn Sep 22 2010 16:50:01

    This program is provided AS IS and comes with
    NO WARRANTY. See the file LICENSE for details.

    Problems or suggestions?
      Website     : <a class="external free" href="http://www.nest-initiative.org" rel="nofollow">http://www.nest-initiative.org</a>
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
