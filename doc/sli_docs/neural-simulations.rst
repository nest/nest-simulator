Neural simulations
==================

Overview
--------

A simulation of a network is like an experiment with the difference that
it takes place inside the computer’s memory rather than in the physical
world.

Like in a real experiment, you need a system which you want to
investigate. Moreover, you need a clear idea of *what* you want to learn
from the experiment. In the context of a network simulation this means
that you have to know *which input* you want to give to your network and
*which output* you expect.

The next section will illustrate the main concepts of NEST simulations,
using a simple simulation. The following sections will then give a
step-by-step introduction to the main concepts of NEST simulations.
Finally, we will discuss a complex example.

A simple simulation
-------------------

The simplest simulation in NEST is that of a network which contains just
one neuron:

::

   SLI ] /iaf_psc_alpha Create /neuron Set

We are going to simulate a standard integrate and fire model with
resting potential at -70 mV and spike threshold at -55 mV. In this line,
we use the model ``iaf_psc_alpha`` to create a neuron. The command
``Create`` returns a handle to the created neuron, which we store in the
variable ``neuron``.

Next, we would like to add a stimulation and a recording device to the
neuron, so that we will see something during the simulation.

In our little example, we want to inject a current into the neuron and
record its membrane potential. Thus, we need to create the corresponding
devices and connect them to the neuron:

::

   SLI ] /voltmeter Create  /vm Set

This command creates a voltmeter node. The handle to the new voltmeter
is stored in the variable ``vm``.

By default, the voltmeter will only record membrane potential values, so
we configure it to show us the time stamp of each value as well. We also
set it to print the time and potential to the screen and we set the
recording interval to 0.1 ms. The default is 1.0 ms.

::

   SLI ] vm << /record_to /screen /interval 0.1 >> SetStatus

The double angled brackets ``<<`` and ``>>``\ delimit a dictionary
definition which consists of successive ``/key value`` pairs.

After setting the parameters of the voltmeter, we connect it to the
neuron, using the handles we have created above:

::

   SLI ] vm neuron Connect

Now we create a DC generator which will supply a constant current to our
neuron:

::

   SLI ] /dc_generator Create /stim Set

Next, we want to set the amplitude of the DC generator such that it
delivers enough current to elicit a spike in the neuron:

::

   SLI ] stim << /amplitude 600.0 >> SetStatus

Here, we only adjust the strength of the input, we set its new value to
``600.0``, which is the amplitude in pA.

We have to connect the DC generator to the neuron:

::

   SLI ] stim neuron Connect

We can now run the simulation and expect to see some results:

::

   SLI ] 15.0 Simulate

The command ``Simulate`` runs the simulation for the specified number of
milliseconds. Below, you see a transcript of the simulation:

::

   Nov 10 08:57:51 Simulate [Info]:
       Simulating 15 ms.
   Nov 10 08:57:51 Scheduler::prepare_nodes [Info]:
       Please wait. Preparing elements.
   Nov 10 08:57:51 Scheduler::prepare_nodes [Info]:
       Simulating 3 local nodes.
   Nov 10 08:57:51 Scheduler::update [Info]:
       Simulating using OpenMP.

   1   0.1 -70
   1   0.2 -70
   1   0.3 -70
   1   0.4 -70
   1   0.5 -70
   1   0.6 -70
   1   0.7 -70
   1   0.8 -70
   1   0.9 -70
   1   1   -70
   1   1.1 -70
   1   1.2 -69.7612    
   1   1.3 -69.5248    
   1   1.4 -69.2907    
   1   1.5 -69.0589    

   :

   1   10.5    -55.3751    
   1   10.6    -55.2818    
   1   10.7    -55.1894    
   1   10.8    -55.098
   1   10.9    -55.0075    
   1   11  -70
   1   11.1    -70
   1   11.2    -70
   1   11.3    -70

   :

   1   12.7    -70
   1   12.8    -70
   1   12.9    -70
   1   13  -70
   1   13.1    -69.7612    
   1   13.2    -69.5248    
   1   13.3    -69.2907    
   1   13.4    -69.0589    
   1   13.5    -68.8295    
   1   13.6    -68.6023    
   1   13.7    -68.3775    
   1   13.8    -68.1548    
   1   13.9    -67.9343    
   1   14  -67.7161    

   Nov 10 08:57:51 Scheduler::resume [Info]:
       Simulation finished.
   Nov 10 08:57:51 Scheduler::finalize_nodes() [Info]:
        using OpenMP.

After some initial messages from the simulation scheduler, we see the
output from the voltmeter. The number in the left column represents the
global ID (GID) of the model neuron and the center column the network
time in milliseconds. The right column contains the values of the
membrane potential at that time. The potential is given in mV.

By default, NEST uses a simulation stepsize of 0.1 ms. With a simulation
time of 15.0 ms, we have 150 simulation steps.

The neuron that we have simulated was a standard *integrate-and-fire*
neuron [Tuckwell91] with a resting potential of -70 mV and a threshold
at -55.0 mV. We see the first effect of the DC input current at 1.2 ms.
This time delay is due to several reasons: The ``dc_generator`` emits
the first current output at the end of the first time step, i.e., at 0.1
ms. Since the connection between generator and neuron was created with
the default delay of 1 ms, the current signal arrives at the neuron at
1.1 ms and thus affects the membrane potential for the first time during
the time step from 1.1 ms to 1.2 ms.

During the time step from 10.9 ms to 11.0 ms, the membrane potential
crosses the threshold value -55.0 mV. Thus, the neuron emits an output
spike at 11.0 ms and the membrane potential is then reset to -70.0 mV
and clamped to the resting value for 2 ms, the refractory period of the
neuron. After the refractory period, the membrane continues to
depolarize due to the continuing input current.

Nodes and Models
----------------

In NEST, the neural system is a collection of nodes and their
interactions. Nodes correspond to things like neurons, synapses, and
devices, and are implemented in C++. The network and its configuration
are defined at the level of the simulation language interpreter.

Nodes are created from a set of prescribed models which are stored in
the dictionary ``modeldict``. The most important neuron models are:

Model name Description ``iaf_psc_alpha`` Simple integrate-and-fire
neuron with alpha-function PSCs. ``iaf_psc_delta`` Integrate-and-fire
neuron with delta-function PSCs. ``iaf_cond_alpha`` Conductance-based
integrate-and-fire neuron with alpha-function synapses. ``iaf_cond_exp``
Conductance-based integrate-and-fire neuron with exp-function synapses.
``hh_psc_alpha`` ``hh_cond_exp_traub`` In order to make the models
visible to the interpreter, the model dictionary has to be opened.

Creating nodes
~~~~~~~~~~~~~~

Before continuing with the example, we reset NEST, to clear all nodes
that we have created before.

::

   SLI ] ResetKernel
   Sep 21 10:13:39 Network::clear_models [Info]:
    Models will be cleared and parameters reset.

Nodes are created from a model, using the command ``Create``.

::

   SLI ] /iaf_psc_alpha Create ==
   1

In the fist line, we create one integrate and fire neuron from the model
``iaf_psc_alpha``.

The return value of ``Create`` is an integer that identifies the last
node that was created in the network (note that this can be different
from 1 if you have not called ``ResetKernel before)``. This integer is
called the node’s *global id* (the network as a whole owns the global id
``0``, therefore the ids of user-created nodes start with ``1``). Often,
it is neccessary to have a large number of nodes of the same type. The
command Create can also be used for this purpose. The following line of
code create 10 integrate and fire neurons:

::

   SLI ] /iaf_psc_alpha 10 Create ==
   11

Status information
------------------

Nodes have a state which can be extracted and modified. In the follwing
example, we display the status information of one the neurons in the
layer we have created above:

::

   SLI ] 1 ShowStatus
   --------------------------------------------------                                                                                
   Name                     Type                Value                                                                                
   --------------------------------------------------                                                                                
   archiver_length          integertype         0                                                                                    
   C_m                      doubletype          250                                                                                  
   E_L                      doubletype          -70                                                                                  
   frozen                   booltype            false
   global_id                integertype         1
   I_e                      doubletype          0
   local                    booltype            true
   local_id                 integertype         1
   model                    literaltype         iaf_psc_alpha
   node_type                literaltype         neuron
   parent                   integertype         0
   recordables              arraytype           <arraytype>
   state                    integertype         0
   tau_m                    doubletype          10
   tau_minus                doubletype          20
   tau_minus_triplet        doubletype          110
   tau_syn_ex               doubletype          2
   tau_syn_in               doubletype          2
   thread                   integertype         0
   t_ref                    doubletype          2
   t_spike                  doubletype          -1
   vp                       integertype         0
   V_m                      doubletype          -70
   V_reset                  doubletype          -70
   V_th                     doubletype          -55
   --------------------------------------------------
   Total number of entries: 24

Using the command ``SetStatus``, it is possible to change the entries of
this so called *status dictionary*. The following lines of code change
the threshold value ``V_th`` to -60 mV:

::

   SLI ] 1 << /V_th -60.0 >> SetStatus
   SLI ] 1 GetStatus /V_th get =
   -60

Please note, that ``SetStatus`` checks if a property really exists in a
node and will issue an error if it doesn’t. This behavior can be changed
by the following command:

::

   << /dict_miss_is_error false >> SetKernelStatus

Then, NEST is very tolerant with respect to the property that you are
trying to change: If it does not know the property, or if the property
cannot be changed, there will be no error, but only a warning. In any
case, ``SetStatus`` does complain if the new value does not match in the
expected type:

::

   SLI ] 1 << /V_th (60) >> SetStatus


   Dec 01 15:33:54 SetStatus_ad [Error]: TypeMismatch
       Expected datatype: doubletype
       Provided datatype: stringtype

In order to find out, which properties of a given model can be changed
an which not, you have to refer to the model’s documentation.

Connections
-----------

Connections between nodes define possible channels for interactions
between them. A connection between two nodes is established, using the
command ``Connect``.

Each connection has two basic parameters, *weight* and *delay*. The
weight determines the strength of the connection, the delay determines
how long an event needs to travel from the sending to the receiving
node. The delay must be a positive number greater or equal to the
simulation stepsize and is given in ms.

Example 1
~~~~~~~~~

::

   SLI ] /iaf_psc_alpha Create /n1 Set
   SLI ] /iaf_psc_alpha Create /n2 Set
   SLI ] /iaf_psc_alpha Create /n3 Set
   SLI ]
   SLI ] n1 n2 Connect
   SLI ] n1 n3 Connect

To inspect the parameters of a connection, one first needs to obtain a
handle to the connection. This is done using the command
``GetConnections``. It takes a dictionary that at least contains the id
of the source node and will return a list of handles for all outgoing
connections. The search can be restricted by using the optional
parameters *target* and *synapse_type*.

Example 2
~~~~~~~~~

::

   SLI ] << /source n1 >> GetConnections /c1 Set
   SLI ] c1 length ==
   2
   SLI ] << /source n1 /target n2 >> GetConnections /c2 Set
   SLI ] c2 length ==
   1

To actually see the parameters of the connection, ``GetStatus`` is used,
just like it is for nodes.

Example 3
~~~~~~~~~

::

   SLI ] c1 0 get GetStatus info
   --------------------------------------------------
   Name                     Type                Value
   --------------------------------------------------
   delay                    doubletype          1
   receptor                 integertype         0
   sizeof                   integertype         32
   source                   integertype         1
   synapse_model            literaltype         static_synapse
   target                   integertype         2
   weight                   doubletype          1
   --------------------------------------------------
   Total number of entries: 7

To change the paramters of a connection, ``SetStatus`` is used, just
like it is for nodes.

Example 4
~~~~~~~~~

::

   SLI ] c1 0 get << /weight 2.0 >> SetStatus
   SLI ] c1 0 get GetStatus /weight get ==
   2.000000e+00

Devices
-------

Devices are network nodes which provide input to the network or record
its output. They encapsulate the stimulation and measurement process. If
you want to extract certain information from a simulation, you need a
device which is able to deliver this information. Likewise, if you want
to send specific input to the network, you need a device which delivers
this input.

Devices have a built-in timer which controls the period they are active.
Outside this interval, a device will remain siltent. The timer can be
configured using the command ``SetStatus``.

By definition a device is active in the interval \\((t_1,t_2)\) , if we
can observe events \\(E\) with time stamps \\(t_E\) which obey \\(t_1 <=
t_E < t_2\) for all \\(E\) . In other words, the interval during which
the device is active corresponds to the range of time-stamps of the
device’s events.

Note that it is not possible to generate/observe an event with time
stamp 0.

Device parameters
~~~~~~~~~~~~~~~~~

The following entries of the status dictionary are the same for all
stimulation and recording devices:

Property Type Description ``/start`` double First time of activity,
relative to the value of ``origin`` in ms. ``/stop`` double First time
of inactivity, relative to the value of ``origin`` in ms. ``/origin``
double Origin of the device clock, relative to the network time in ms.
In general, the following must hold:

1. *stop* >= *start*
2. If *stop =* start\ *, the device is inactive.*

Stimulating Devices
~~~~~~~~~~~~~~~~~~~

A range of devices is available for the stimulation of neurons. The most
important ones are listed in the following table. For details, refer to
the documentation of the respective decive.

Model name Description ``spike_generator`` Device to generate spikes at
specific times. ``poisson_generator`` Device to generate poisson
shotnoise. ``dc_generator`` Device to generate a constant current.
``ac_generator`` Device to generate an alternating (sine) current.
``step_current_generator`` Device to generate a step current with
different amplitudes at different times.

Example 5
^^^^^^^^^

::

   SLI ] /iaf_psc_alpha Create /n Set
   SLI ] /poisson_generator Create /pg Set
   SLI ] pg << /rate 220.0 Hz >> SetStatus
   SLI ] pg n Connect

Recording devices
~~~~~~~~~~~~~~~~~

All devices which are used to observe the state of other network nodes
are called recording devices. Examples are ``multimeter`` and
``spike_detector``.

Recording devices have properties which control the amount, the
format, and the destination of their output. The latter is done by
setting their property ``record_to`` to the name of the recording
backend to use. To dump recorded data to a file, set ``/ascii``, to
print to the screen, use ``/screen`` and to hold the data in memory,
set ``/memory``, which is also the default for all devices. Data
stored in memory can be retrieved after the simulation using
``GetStatus``. To get a list of all available recording backends, run

::
   
   SLI ] GetKernelStatus /recording_backends get keys ==

Device models are also stored in the dictionary ``modeldict``. The most
important devices are:

* ``voltmeter`` Device to observe membrane potentials.
* ``multimeter`` Device to observe arbitrary analog quantities.
* ``spike_detector`` Device to observe spike times.

Please note that the connection direction for analog recorders (all
except ``spike_detector`` in above list) is inverted with respect to
other recorders, i.e. the recorder has to connected to the neurons in
this case.

Example 6
^^^^^^^^^

::

   SLI ] /iaf_psc_alpha Create /n Set
   SLI ] /voltmeter Create /vm Set
   SLI ] /spike_detector Create /sd Set
   SLI ] vm n Connect
   SLI ] n sd Connect

Simulation
----------

NEST simulations are time driven. The simulation time proceeds in
discrete steps of size ``dt``, set using the property ``resolution`` of
the root node. In each time slice, all nodes in the system are updated
and pending events are delivered.

The simulation is run by calling the command ``t Simulate``, where ``t``
is the simulation time in milliseconds
