---
layout: index
---

[« Back to the index](index)

<hr>

# Updating models for NEST 2.4 or prior to 2.6 or later

This text describes which changes need to be applied to a neuron model
and to a synapse model in order to convert it from NEST 2.4 (3rd
generation kernel) to NEST 2.6 (4th generation kernel). Please read
Kunkel et al, Front Neuroinform 8:78 (2014),
[doi:10.3389/fninf.2014.00078](http://dx.doi.org/10.3389/fninf.2014.00078
) especially Section 3.3, before beginning to convert your models. It
will provide necessary background information.

##  Converting neuron models

* Change statement `using Node::connect_sender;` to `using Node::handles_test_event;`. 
* Remove function `check_connection(Connection&, port)`.
* Change function `connect_sender(SpikeEvent&, port)` to `handles_test_event(SpikeEvent&, rport)` for each event type that can be received by the neuron; note the change in the datatype of the second argument from `port` to `rport`
* For most neuron models, the implementation `handles_test_event()` will be identical to the previous implementation of `connect_sender()`. It should be similar to the one for `iaf_neuron`:

       inline
       port iaf_neuron::handles_test_event(SpikeEvent&, rport receptor_type)
       {
         if (receptor_type != 0)
           throw UnknownReceptorType(receptor_type, get_name());
         return 0;
       }

* Define function `send_test_event(Node&, rport, synindex, bool)`, so that it sends an event of the type that the neuron sends (typically `SpikeEvent`). The implementation is similar to the `check_connection()` function in NEST 2.4, and will in most cases be the same as for `iaf_neuron`. The last two parameters will usually not be relevant.

       inline
       port iaf_neuron::send_test_event(Node& target, rport receptor_type, synindex, bool)
       {
         SpikeEvent e;
         e.set_sender(*this);
         return target.handles_test_event(e, receptor_type);
       }

## Converting stimulating devices

Stimulating devices, such as spike and current generators, need to
handle a some extra aspects and therefore have a more complex
send_test_event(). function. Please see the function for the
poisson_generator below as an example.

* The function must call `device_.enforce_single_syn_type(syn_id)`, where `device_` is of type `StimulatingDevice`. This call ensures that only a single synapse type is used for 'outgoing' connections from the device. While this is not really relevant for poisson_generator, it is crucial for a range of other stimulators and therefore enforce by NEST throughout.
* Many stimulating devices use callback mechanisms, where the device first sends a `DSSpikeEvent` or `DSCurrentEvent`, which it then handles by its own `event_hook()` method, which dispatches the "real" output to the targets as `SpikeEvent` or `CurrentEvent`. The branch in the implementation below ensures that a `DSSpikeEvent` is used on the first call to `send_test_event()` and a `SpikeEvent` on the second, cf. Fig 5 in Kunkel et al (2014). The mechanism ensures that only static synapses can be used to connect devices to neurons.

       inline
       port poisson_generator::send_test_event(Node& target, rport receptor_type, synindex syn_id, bool dummy_target)
       {
         device_.enforce_single_syn_type(syn_id);
         
         if ( dummy_target )
         {
           DSSpikeEvent e;
           e.set_sender(*this);
           return target.handles_test_event(e, receptor_type);
         }
         else
         {
           SpikeEvent e;
            e.set_sender(*this);
            return target.handles_test_event(e, receptor_type);
         }
       }

## Converting synapse models

Converting a synapse model requires more comprehensive changes. We
will first consider stdp_synapse as an example, and consider more
complex models later.

### Connection base class, target data types, and synapse model registration

* All connections must be derived from the base class template `Connection<T>`.
* The template parameter T represents the target identifier data type (see Kunkel et al, Sec 3.3.2). It can either be `TargetIdentifierPtrRport` for general synapses or `TargetIdentifierIndex` for HPC synapses with extra-low memory footprint (max 65.535 targets per thread, rport fixed to 0).
* The template is instantiated on synapse model registration, e.g. in `modelsmodule.cpp`

       register_connection_model < STDPConnection<TargetIdentifierPtrRport> > (net_, "stdp_synapse");
       register_connection_model < STDPConnection<TargetIdentifierIndex> > (net_, "stdp_synapse_hpc");

### General remarks

* Ususally, only `connection.h` needs to be included in files defining synapses.
* Because the base class `Connection<T>` is a template, all code depending on it must be visible at compile time. This may require moving code from cpp-files to h-files.
* If you are certain that you will never want to use your synapse model as an HPC-synapse, you can derive it from the specialized base class.
* The following aspects of a connection are handled by the Connection<T> base class:
   * the target node
   * the rport
   * the delay 
* They are available from derived classes through accessor methods. 

### Example: stdp_synapse

* This is a synapse model in which all parameters are individual to each connection.
* Change the class declaration to

       template<typename targetidentifierT>
       class STDPConnection : public Connection<targetidentifierT>

* Add the following typedefs
   * the first one is required by GenericConnectorModel<ConnectionType>;
   * the second provides a convenient shorthand for the base class. 

          typedef CommonSynapseProperties CommonPropertiesType;
          typedef Connection<targetidentifierT> ConnectionBase;

* Add the following using declarations for accessor methods in the base class template

       using ConnectionBase::get_delay_steps;
       using ConnectionBase::get_delay;
       using ConnectionBase::get_rport;
       using ConnectionBase::get_target;

* Add a data member for the synaptic weight (in NEST 2.4 handled by base class ConnectionHetWD)

       double_t weight_;
       ...

* In default and copy constructor, add forward to base class and initializer for weight. Default weight in NEST is 1.0

       STDPConnection() :
          ConnectionBase(),
          weight_(1.0),
          ...

       STDPConnection<targetidentifierT> &rhs) :
         ConnectionBase(rhs),
         weight_(rhs.weight_),
         ...

* Define a class implementing a dummy node used for the first step in connection testing (see Fig 5 in Kunkel et al, 2014), derived from ConnTestDummyNodeBase.

   * The class should be defined inside your connection class.
   * This class must override handles_test_event() for each event type that the synapse handles.
   * The using declaration is required because we override an overloaded virtual function.
   * The function should always return invalid_port_, this return value is ignored by the caller.
   * The class and its base are called check_helper and check_helper_base in Kunkel et al, 2014. 

          class ConnTestDummyNode: public ConnTestDummyNodeBase
          {
          public:
            using ConnTestDummyNodeBase::handles_test_event;
            port handles_test_event(SpikeEvent&, rport) { return invalid_port_; }
          };

* Implement check_connection(). The fourth parameter, const CommenPropertiesType&, is new in the method signature relative to NEST 2.4, and the implementation slightly different. It forwards the actual connection checking to the base class.

       void check_connection(Node & s, Node & t, rport receptor_type, double_t t_lastspike, 
                             const CommonPropertiesType& cp)
       {
         ConnTestDummyNode dummy_target;
         ConnectionBase::check_connection_(dummy_target, s, t, receptor_type);
         t.register_stdp_connection(t_lastspike - get_delay());
       }

* Add a set_weight() method. It is used to set the weight efficiently during synapse creation. 

       void set_weight(double_t w) { weight_ = w; }

* Modify the send() method. The key changes are
   
   * add thread t as second argument to the send() method, so that its signature becomes 

          void send(Event& e, thread t, double_t t_lastspike, const CommonSynapseProperties &cp)

   * the method receives the target thread as second argument
   * target, rport and delay information must be obtained from the base class through accessor methods
   * the code below shows only the modified lines 

          void send(Event& e, thread t, double_t t_lastspike, const CommonSynapseProperties &)
          {
            [snip]
          
            Node *target = get_target(t);
            double_t dendritic_delay = get_delay();
          
            [snip]  
          
            target->get_history(t_lastspike - dendritic_delay, t_spike - dendritic_delay,
                         		  &start, &finish);
          
           [snip]
          
           e.set_receiver(*target);
           e.set_weight(weight_);
           e.set_delay(get_delay_steps());
           e.set_rport(get_rport());
           e();
            
           [snip]
          }

*  Add forwards to base class and add weight in set_status() and get_status() 

        void get_status(DictionaryDatum & d) const
        {
          ConnectionBase::get_status(d);
          def<double_t>(d, names::weight, weight_);
          ...
        }
          
        void set_status(const DictionaryDatum & d, ConnectorModel &cm)
        {
          ConnectionBase::set_status(d, cm);
          updateValue<double_t>(d, names::weight, weight_);
          ... 
        }

### Example: stdp_synapse_hom

This section describes the implementation stdp_synapse_hom relative to
stdp_synapse, both for NEST 2.6, it does not compare this synapse to
its NEST 2.4 version. The specialty of the _hom variant is that all
parameters concerning plasticity are homogeneous, i.e., identical for
all synapses of the type.

* Define a class representing the common properties, derived from CommonSynapseProperties. Note that the constructor, and status setters/getters must forward to the base class.

       class STDPHomCommonProperties : public CommonSynapseProperties
       {
       public:
         STDPHomCommonProperties():
           CommonSynapseProperties(),
           tau_plus_(20.0),
           ...
           {}
      
         void get_status(DictionaryDatum & d) const
         {
           CommonSynapseProperties::get_status(d);
           def<double_t>(d, "tau_plus", tau_plus_);
           ...
         }
         
         void set_status(const DictionaryDatum & d, ConnectorModel& cm)
         {
           CommonSynapseProperties::set_status(d, cm);
           updateValue<double_t>(d, "tau_plus", tau_plus_);
           ...
         }
       };

* Typedef this class as `CommonPropertiesType` 

       typedef STDPHomCommonProperties CommonPropertiesType;

* `send()` also needs to take this type as type of its fourth argument 

       void send(Event& e, thread t, double_t t_lastspike, const STDPHomCommonProperties &)

* All data members that are in `STDPHomCommonProperties` are removed 

* `send()` and its helper functions must access those members through the common properties reference (`depress_()`, not show):

       double_t facilitate_(double_t w, double_t kplus, const STDPHomCommonProperties &cp)
       {
         double_t norm_w = (w / cp.Wmax_) + (cp.lambda_ * std::pow(1.0 - (w/cp.Wmax_), cp.mu_plus_) * kplus);
         return norm_w < 1.0 ? norm_w * cp.Wmax_ : cp.Wmax_;
       }
       
       void send(Event& e, thread t, double_t t_lastspike, const STDPHomCommonProperties &cp)
       {
         [snip] 
       
         while (start != finish)
         {
           [snip]	
           weight_ = facilitate_(weight_, Kplus_ * std::exp(minus_dt / cp.tau_plus_), cp);
         }
       
         weight_ = depress_(weight_, target->get_K_value(t_spike - dendritic_delay), cp);
       
         [snip]
        
         Kplus_ = Kplus_ * std::exp((t_lastspike - t_spike) /  cp.tau_plus_) + 1.0;
       }

* In `get_status()`, drop all data members that have been moved to common properties.
