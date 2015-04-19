---
layout: index
---

[« Back to the index](index)

<hr>

# Multimeter support for models

How does logging work from the perspective of a model developer?

1. The model must have a `universal_data_logger<>` member, usually
   called `logger_`. Once every timestep the `update()` function of
   the model calls `B_.logger_.record_data(origin.get_steps()+lag);`
   to notify that new data is available for pick-up.
2. The logger collects data from the model neuron through access
   functions, i.e., member functions of the model class returning
   values of state variables.
3. The `RecordablesMap` of the model provides the mapping between
   names of recordable quantities and the pertaining access
   functions. The keys of the RecordablesMap (i.e. the names of the
   recordables) are made available to the user through the
   `recordables` list in the model status dictionary.
4. By setting the `record_from` property of a multimeter to a list of
   recordables of a model, the user selects which quantities to
   record. When the multimeter is connected to model neurons, it
   configures the loggers in the models such that they collect the
   correct data.
5. During simulation, multimeter regularly sends `DataLoggingRequest`
   events to all connected neurons to collect data from the
   `universal_data_loggers` in the neurons.

## Boilerplate code

Here comes boilerplate code for universal data logger support, taken
from `iaf_neuron`. The code below includes only lines directly
pertaining to the logger.

NOTE: For nest-2.0.0-rc3 and later, connect_sender() has to return the
result of B_.logger_.connect_logging_device() as shown below.

### Header file

    #include "universal_data_logger.h"
    
    namespace nest
    {
      class iaf_neuron: public Archiving_Node
      {    
      public:
        using Node::connect_sender;
        using Node::handle;
    
        void handle(DataLoggingRequest &);
        port connect_sender(DataLoggingRequest&, port);
    
      private:
        friend class RecordablesMap<iaf_neuron>;
        friend class UniversalDataLogger<iaf_neuron>;
    
        struct Buffers_ {
          Buffers_(iaf_neuron &);
          Buffers_(const Buffers_ &, iaf_neuron &);
          UniversalDataLogger<iaf_neuron> logger_;
        };
        
        //! Access function: Read out the real membrane potential
        double_t get_V_m_() const { return S_.y3_ + P_.U0_; }
    
        //! Mapping of recordables names to access functions
        static RecordablesMap<iaf_neuron> recordablesMap_;
    };  // end class iaf_neuron
    
    inline
    port iaf_neuron::connect_sender(DataLoggingRequest &dlr, 
    		      port receptor_type)
    {
      if (receptor_type != 0)
        throw UnknownReceptorType(receptor_type, get_name());
      return B_.logger_.connect_logging_device(dlr, recordablesMap_);
    }
    
    inline
    void iaf_neuron::get_status(DictionaryDatum &d) const
    {
      (*d)[names::recordables] = recordablesMap_.get_list();
    }
    
    } // namespace

### Implementation file

    #include "universal_data_logger_impl.h"
    
    nest::RecordablesMap<nest::iaf_neuron> nest::iaf_neuron::recordablesMap_;
    namespace nest
    {
      template <>
      void RecordablesMap<iaf_neuron>::create()
      {
        insert_(names::V_m, &iaf_neuron::get_V_m_);
      }
    }
    
    nest::iaf_neuron::Buffers_::Buffers_(iaf_neuron &n)
      : logger_(n)
    {}
    
    nest::iaf_neuron::Buffers_::Buffers_(const Buffers_ &, iaf_neuron &n)
      : logger_(n)
    {}
    
    nest::iaf_neuron::iaf_neuron()
      : Archiving_Node(), 
        P_(), 
        S_(),
        B_(*this)
    {
      recordablesMap_.create();
    }
    
    void nest::iaf_neuron::init_buffers_()
    {
      B_.logger_.reset(); // includes resize
    }
    
    void nest::iaf_neuron::calibrate()
    {
      B_.logger_.init();
    }
    
    void nest::iaf_neuron::update(Time const & origin, const long_t from, const long_t to)
    {
      for ( long_t lag = from ; lag < to ; ++lag )
      {
         // update neuron
    
          B_.logger_.record_data(origin.get_steps()+lag);
        }  
    }                           
                         
    void nest::iaf_neuron::handle(DataLoggingRequest& e)
    {
      B_.logger_.handle(e);
    }

## An example of a more complex recordables map

`iaf_cond_alpha` has a five-element state vector, and three of its
elements are recordables quantities: the membrane potential and the
excitatory and inhibitory conductances. In addition, we make the
remaining refractory time recordable (mostly useful for debugging). We
only need to write a single, templatized access function for all state
vector members:

    template <State_::StateVecElems elem>
    double_t get_y_elem_() const { return S_.y[elem]; }
    
    double_t get_r_() const { return Time::get_resolution().get_ms() * S_.r; }

When mapping names to functions in the recordables map, we instantiate
the templated access function with the (symbolic) index of the state
vector element we want to record.

    template <>
    void RecordablesMap<iaf_cond_alpha>::create()
    {
      // use standard names whereever you can for consistency!
      insert_(names::V_m, 
  	    &iaf_cond_alpha::get_y_elem_<iaf_cond_alpha::State_::V_M>);
      insert_(names::g_ex, 
  	    &iaf_cond_alpha::get_y_elem_<iaf_cond_alpha::State_::G_EXC>);
      insert_(names::g_in, 
  	    &iaf_cond_alpha::get_y_elem_<iaf_cond_alpha::State_::G_INH>);
  
      insert_(names::t_ref_remaining, 
  	    &iaf_cond_alpha::get_r_);
    }