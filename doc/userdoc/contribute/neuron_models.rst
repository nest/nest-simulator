Writing neuron models
=====================

.. note::

   We are currently in the process of transitioning from the old way of
   writing models for NEST (i.e manually by implementing a C++ class) to
   our new modeling language NESTML, which allows to write neuron models
   using domain concepts from computational neuroscience as first level
   objects and generate the source code automatically.

   If you intend to write your own neuron model, please have a look
   at `the NESTML GitHub page <https://github.com/nest/nestml>`_. In case
   of questions, feel free to subscribe to
   the `NEST mailing list <http://www.nest-simulator.org/community/>`_. If
   you find any problems with NESTML, we kindly ask you to open an issue
   on the `NESTML issue tracker <https://github.com/nest/nestml>`_.

All models in NEST are derived from the base class Node. Neuron models
are derived directly from the base class, whereas devices use the base
classes Device and Recorder. The base classes define some pure virtual
functions that have to be implemented in a derived model class. The
most important functions are:

* ``void calibrate()``: Set scheduler dependent internal
  parameters. This function is called by the scheduler right before
  the simulation is performed. Here you can reset the size of ring
  buffers, and set variables that depend on the state of the
  scheduler.
* ``void update(Time const & origin, const long_t from, const long_t
  to)``: Update the dynamic state of the model. Here, the model
  propagates its internal state from one time step to the next. The
  parameter origin represents the beginning of the current network
  time slice (in steps of \Delta_{min}, the minimal connection delay
  in the network). to and from refer to the actual time steps that
  have to be considered inside of the slice. They are expressed in
  units of h, the simulation resolution.
* ``void set_properties(const DictionaryDatum & d)``: Set user defined
  parameters of the model. This function is called when SetStatus is
  executed in SLI.
* ``void get_properties(DictionaryDatum & d) const``: Retrieve a
  dictionary with parameters from the model. This is the C++ end of
  the SLI function GetStatus.

The following sections will explain each of these functions in detail
with the help of an example model, called my_model.

Model description
-----------------

The model that's used as an example is a simple and unrealistic neuron
model. We want it to do the following:

* It should have an internal variable counter that counts the number
  of spikes it received from other neurons.
* If counter reaches a specific number threshold it should send out a
  spike to each of its target nodes.
* For more complexity, the number of incoming spikes is multiplied
  with a constant factor that can be set by the user.It should
  default to 1.

Considerations for transferring the ideas to code
-------------------------------------------------

The possibility to receive a special type of event is controlled by
the presence of an overloaded version of Node::handle(EventT) for this
specific type of event. The default implementation in the base class
will throw UnexpectedEvent for each incoming event.

Updating the internal state
---------------------------

A typical update method of a neuron model will look like this:

.. code-block:: C++

   void nest::iaf_neuron::update(Time const & origin, const long_t from, const long_t to)
   {
     assert(to >= 0 && (delay) from < Scheduler::get_min_delay());
     assert(from < to);
     for ( long_t lag = from ; lag < to ; ++lag )
     {
       if ( r_ == 0 )
       {
         // neuron not refractory
         y3_ = P30_*(y0_ + I_e_) + P31_*y1_ + P32_*y2_ + P33_*y3_;
       }
       else // neuron is absolute refractory
        --r_;
       // alpha shape PSCs
       y2_ = P21_*y1_ + P22_ * y2_;
       y1_ *= P11_;
                                                          // apply spikes delivered in this step
       y1_ += PSCInitialValue_* spikes_.get_value(lag);   // the spikes arriving at T+1 have an
                                                          // immediate effect on the state of the
                                                          // neuron
       // threshold crossing
       if (y3_ >= Theta_)
       {
         r_ = RefractoryCounts_;
         y3_=V_reset_;
         // A supra-threshold membrane potential should never be observable.
         // The reset at the time of threshold crossing enables accurate integration
         // independent of the computation step size, see [2,3] for details.
         set_spiketime(Time::step(origin.get_steps()+lag+1));
         SpikeEvent se;
         network()->send(*this, se, lag);
       }
       // set new input current
       y0_ = currents_.get_value(lag);
       // voltage logging for entire time slice
       potentials_[network()->get_slice() % 2][lag] = y3_ + U0_;
     }
   }

Sending Events
--------------

During simulation, interactions are realized by events that travel
from a sending node to the receiving node. Nodes can send and receive
events. Typically, a node sends only one type of event, while it may
handle several event types. Events are the only way by which a node
can exchange information with its environment. There are different
types of events, depending on the information which is to be
transmitted between the nodes. The following event types are
available:

* SpikeEvent
* RateEvent
* CurrentEvent
* PotentialEvent

Each event carries a time-stamp according to the time when it was created.

Each model can only send a single type of event. So it is not possible
for a neuron to send SpikeEvents and CurrentEvents. The type of event
is defined in my_model::check_connection(). If we assume that the
model should send SpikeEvents, this function would look like this:

.. code-block:: C++

   port my_model::check_connection(Node& r, port rp)
   {
     SpikeEvent e;
     e.set_sender(*this);
     e.set_receiver(r);
     e.set_rport(rp);
     return r.connect_sender(e);
   }

Handling incoming events
------------------------

See the ``handle()`` functions.

Proxies and local receivers
---------------------------

You need to include the following two lines in the declaration of your
generator class (these lines are correct for a generator providing
current input to nodes and which shall be recordable by multimeter):

.. code-block:: C++

   bool has_proxies()    const { return false; }
   bool local_receiver() const { return true;  }

The standard location is right behind the ``using Node::handle;`` line.

What do these lines mean? In parallel simulations, each node in a
network is updated by one parallel process, and a proper instance of
that node exists in that process. All other processes only represent
that node through an instance of the proxynode class. Most devices are
exceptions: they have a proper instance on each parallel process: In a
parallel simulation with 10 processes, there will be 10 multimeter
instances in all, and each instance records only from nodes on the
same process.

Furthermore, NEST can only send spikes between parallel
processes. Therefore, any current generator must have proper instances
on each parallel process, so that CurrentEvents can be sent locally on
each process. By defining has_proxies() to return false, you tell the
NEST kernel to create individual instances of your generator on each
process. If you don't do this, the kernel will try to send any event
created by your generator via the global event queue (which ships
events between processes), and that triggers the assertion, since the
global queue does not accept anything else than SpikeEvents.

Now nodes in general can receive (spike) input from any other node,
whether on the same or different processes. But if the sending node
was a device with instances on each parallel process, while the
receiver was a "normal" node, then that receiver might end up
receiving input from each instance of the sending node. But those
multiple instances are only a technicality and should not provide
input multiple times. Therefore, NEST prohibits connections from nodes
with proxies to "normal" nodes. The only exception is if a node class
is a so-called "local receiver", i.e., it will accept input only from
nodes (more precisely: node instances) on the same process. We inform
the NEST kernel about this by defining local_receiver() to return
true.

You need to define your generator as local_receiver so that it can
accept DataLoggingRequests from the Multimeter, which is a device
without proxies.

Now why was this no problem with the smp_generator, which you used as
starting point? Well, the smp_generator creates a sinusoidally
modulated random spike train and sends that one train to all its
targets, no matter on which parallel process the target "lives". This
is only possible by having a single smp_generator instance in the
entire network, so smp_generator is one of the few devices *with*
proxies. This is possible because it sends SpikeEvents, which can be
communicated globally. Since it has proxies, it cannot be a "local
receiver", as it does not have an instance on all processes.

So why does having proxies not automatically imply being a local
receiver? Because some devices are global receivers. Volume
transmitters receive neuromodulatory spikes from neurons which can be
anywhere on the physical network, but only send modulatory information
to synapses that are local to them. Thus they have no proxies but are
not local receivers.
