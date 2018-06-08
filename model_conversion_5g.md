---
layout: index
---

[« Back to the index](index)

<hr>

# Updating models from 2.14 or prior to 2.16

With the introduction of the new connection infrastructure of the 
[5g kernel](https://www.frontiersin.org/articles/10.3389/fninf.2018.00002/full), 
rate neuron, synapse and device models need to be slightly adapted from prior 
versions to be compatible with the latest release (2.16). The following 
describes all necessary changes. If you still encounter issues after 
following these instructions, please contact the
[NEST User mailing list](http://mail.nest-initiative.org/cgi-bin/mailman/listinfo/nest_user), 
create an [issue on GitHub](https://github.com/nest/nest-simulator/issues), or 
directly submit a pull request, updating these instructions.

## Converting rate-neuron models

Since events are now delivered in the same timestep as they are communicated,
there is no need to correct for the min-delay duration in the `handle` 
functions of rate neuron models. Remove `- kernel().connection_manager
.get_min_delay()` in the `handle(...)` functions of all rate models. There is
no need to adapt spiking models, since these carry the correct time stamp.

    ```diff
    diff --git a/models/rate_neuron_ipn_impl.h b/models/rate_neuron_ipn_impl.h
    index 8da17690f..3f7feac63 100644
    --- a/models/rate_neuron_ipn_impl.h
    +++ b/models/rate_neuron_ipn_impl.h
    @@ -444,28 +444,24 @@ nest::rate_neuron_ipn< TNonlinearities >::handle(
           if ( e.get_weight() >= 0.0 )
           {
             B_.delayed_rates_ex_.add_value(
    -          e.get_delay() - kernel().connection_manager.get_min_delay() + i,
    -          e.get_weight() * e.get_coeffvalue( it ) );
    +          e.get_delay() + i, e.get_weight() * e.get_coeffvalue( it ) );
           }
           else
           {
             B_.delayed_rates_in_.add_value(
    -          e.get_delay() - kernel().connection_manager.get_min_delay() + i,
    -          e.get_weight() * e.get_coeffvalue( it ) );
    +          e.get_delay() + i, e.get_weight() * e.get_coeffvalue( it ) );
           }
         }
         else
         {
           if ( e.get_weight() >= 0.0 )
           {
    -        B_.delayed_rates_ex_.add_value(
    -          e.get_delay() - kernel().connection_manager.get_min_delay() + i,
    +        B_.delayed_rates_ex_.add_value( e.get_delay() + i,
               e.get_weight() * nonlinearities_.input( e.get_coeffvalue( it ) ) );
           }
           else
           {
    -        B_.delayed_rates_in_.add_value(
    -          e.get_delay() - kernel().connection_manager.get_min_delay() + i,
    +        B_.delayed_rates_in_.add_value( e.get_delay() + i,
               e.get_weight() * nonlinearities_.input( e.get_coeffvalue( it ) ) );
           }
         }
    ```

## Converting synapse models

The time of the last spike going through a particular connection 
(`t_lastspike`) is now stored directly in the corresponding synapse objects. 
Remove the `t_lastspike` argument from the `check_connection` and `send` 
functions of connections and add a new private member `t_lastspike` for 
connections requiring spike-timing information. Make sure to initialize this 
new variable in the constructor and set it to the spike time (typically 
`t_spike`) at the end of `send(...)`.

    ```diff
    diff --git a/models/stdp_connection.h b/models/stdp_connection.h
    index f46ccb6c8..4056563b9 100644
    --- a/models/stdp_connection.h
    +++ b/models/stdp_connection.h
    @@ -134,13 +134,9 @@ public:
       /**
        * Send an event to the receiver of this connection.
        * \param e The event to send
    -   * \param t_lastspike Point in time of last spike sent.
        * \param cp common properties of all synapses (empty).
        */
    -  void send( Event& e,
    -    thread t,
    -    double t_lastspike,
    -    const CommonSynapseProperties& cp );
    +  void send( Event& e, thread t, const CommonSynapseProperties& cp );
     
     
       class ConnTestDummyNode : public ConnTestDummyNodeBase
    @@ -160,14 +156,13 @@ public:
       check_connection( Node& s,
         Node& t,
         rport receptor_type,
    -    double t_lastspike,
         const CommonPropertiesType& )
       {
         ConnTestDummyNode dummy_target;
     
         ConnectionBase::check_connection_( dummy_target, s, t, receptor_type );
     
    -    t.register_stdp_connection( t_lastspike - get_delay() );
    +    t.register_stdp_connection( t_lastspike_ - get_delay() );
       }
     
       void
    @@ -202,6 +197,8 @@ private:
       double mu_minus_;
       double Wmax_;
       double Kplus_;
    +
    +  double t_lastspike_;
     };
     
     
    @@ -209,18 +206,16 @@ private:
      * Send an event to the receiver of this connection.
      * \param e The event to send
      * \param t The thread on which this connection is stored.
    - * \param t_lastspike Time point of last spike emitted
      * \param cp Common properties object, containing the stdp parameters.
      */
     template < typename targetidentifierT >
     inline void
     STDPConnection< targetidentifierT >::send( Event& e,
       thread t,
    -  double t_lastspike,
       const CommonSynapseProperties& )
     {
       // synapse STDP depressing/facilitation dynamics
    -  //   if(t_lastspike >0) {std::cout << "last spike " << t_lastspike <<
    +  //   if(t_lastspike_ >0) {std::cout << "last spike " << t_lastspike <<
       //   std::endl ;}
       double t_spike = e.get_stamp().get_ms();
       // t_lastspike_ = 0 initially
    @@ -234,7 +229,7 @@ STDPConnection< targetidentifierT >::send( Event& e,
       std::deque< histentry >::iterator start;
       std::deque< histentry >::iterator finish;
     
    -  // For a new synapse, t_lastspike contains the point in time of the last
    +  // For a new synapse, t_lastspike_ contains the point in time of the last
       // spike. So we initially read the
       // history(t_last_spike - dendritic_delay, ..., T_spike-dendritic_delay]
       // which increases the access counter for these entries.
    @@ -242,13 +237,15 @@ STDPConnection< targetidentifierT >::send( Event& e,
       // history[0, ..., t_last_spike - dendritic_delay] have been
       // incremented by Archiving_Node::register_stdp_connection(). See bug #218 for
       // details.
    -  target->get_history(
    -    t_lastspike - dendritic_delay, t_spike - dendritic_delay, &start, &finish );
    +  target->get_history( t_lastspike_ - dendritic_delay,
    +    t_spike - dendritic_delay,
    +    &start,
    +    &finish );
       // facilitation due to post-synaptic spikes since last pre-synaptic spike
       double minus_dt;
       while ( start != finish )
       {
    -    minus_dt = t_lastspike - ( start->t_ + dendritic_delay );
    +    minus_dt = t_lastspike_ - ( start->t_ + dendritic_delay );
         ++start;
         // get_history() should make sure that
         // start->t_ > t_lastspike - dendritic_delay, i.e. minus_dt < 0
    @@ -268,7 +265,9 @@ STDPConnection< targetidentifierT >::send( Event& e,
       e.set_rport( get_rport() );
       e();
     
    -  Kplus_ = Kplus_ * std::exp( ( t_lastspike - t_spike ) / tau_plus_ ) + 1.0;
    +  Kplus_ = Kplus_ * std::exp( ( t_lastspike_ - t_spike ) / tau_plus_ ) + 1.0;
    +
    +  t_lastspike_ = t_spike;
     }
     
     
    @@ -283,6 +282,7 @@ STDPConnection< targetidentifierT >::STDPConnection()
       , mu_minus_( 1.0 )
       , Wmax_( 100.0 )
       , Kplus_( 0.0 )
    +  , t_lastspike_( 0.0 )
     {
     }
     
    @@ -298,6 +298,7 @@ STDPConnection< targetidentifierT >::STDPConnection(
       , mu_minus_( rhs.mu_minus_ )
       , Wmax_( rhs.Wmax_ )
       , Kplus_( rhs.Kplus_ )
    +  , t_lastspike_( rhs.t_lastspike_ )
     {
     }
    ```

All connections transmitting secondary events now need to tell the 
ModelManager whether they support waveform relaxation when registering the 
model. Make sure to add the corresponding argument in 
`register_secondary_connection_model` (see modelsmodule.cpp, e.g., 
`gap_junction`).
    
    ```diff
    diff --git a/models/modelsmodule.cpp b/models/modelsmodule.cpp
    index 29ba57575..04eaef056 100644
    --- a/models/modelsmodule.cpp
    +++ b/models/modelsmodule.cpp
    @@ -477,19 +499,31 @@ ModelsModule::init( SLIInterpreter* )
       kernel()
         .model_manager
         .register_secondary_connection_model< GapJunction< TargetIdentifierPtrRport > >(
    -      "gap_junction", /*has_delay=*/false, /*requires_symmetric=*/true );
    +      "gap_junction",
    +      /*has_delay=*/false,
    +      /*requires_symmetric=*/true,
    +      /*supports_wfr=*/true );
       kernel()
         .model_manager
         .register_secondary_connection_model< RateConnectionInstantaneous< TargetIdentifierPtrRport > >(
    -      "rate_connection_instantaneous", /*has_delay=*/false );
    +      "rate_connection_instantaneous",
    +      /*has_delay=*/false,
    +      /*requires_symmetric=*/false,
    +      /*supports_wfr=*/true );
       kernel()
         .model_manager
         .register_secondary_connection_model< RateConnectionDelayed< TargetIdentifierPtrRport > >(
    -      "rate_connection_delayed" );
    +      "rate_connection_delayed",
    +      /*has_delay=*/true,
    +      /*requires_symmetric=*/false,
    +      /*supports_wfr=*/false );
       kernel()
         .model_manager
         .register_secondary_connection_model< DiffusionConnection< TargetIdentifierPtrRport > >(
    -      "diffusion_connection", /*has_delay=*/false );
    +      "diffusion_connection",
    +      /*has_delay=*/false,
    +      /*requires_symmetric=*/false,
    +      /*supports_wfr=*/true );
    ```

## Converting devices

All devices are now subclasses of a new class `DeviceNode` which takes care 
of additional infrastructure information. In all devices models, include 
`device_node.h` instead of `node.h` and replace all `Node` with `DeviceNode`.

    ```diff
    diff --git a/models/multimeter.h b/models/multimeter.h
    index fe2f7c147..b0573d541 100644
    --- a/models/multimeter.h
    +++ b/models/multimeter.h
    @@ -30,7 +30,7 @@
     #include "connection.h"
     #include "exceptions.h"
     #include "kernel_manager.h"
    -#include "node.h"
    +#include "device_node.h"
     #include "recording_device.h"
     #include "sibling_container.h"
     
    @@ -155,7 +155,7 @@ namespace nest
      * @ingroup Devices
      * @see UniversalDataLogger
      */
    -class Multimeter : public Node
    +class Multimeter : public DeviceNode
     {
     
     public:
    ```

    ```diff
    diff --git a/models/multimeter.cpp b/models/multimeter.cpp
    index 00f39ac57..4822e3d20 100644
    --- a/models/multimeter.cpp
    +++ b/models/multimeter.cpp
    @@ -28,7 +28,7 @@
     namespace nest
     {
     Multimeter::Multimeter()
    -  : Node()
    +  : DeviceNode()
       , device_( *this, RecordingDevice::MULTIMETER, "dat", true, true )
       , P_()
       , S_()
    @@ -38,7 +38,7 @@ Multimeter::Multimeter()
     }
     
     Multimeter::Multimeter( const Multimeter& n )
    -  : Node( n )
    +  : DeviceNode( n )
       , device_( *this, n.device_ )
       , P_( n.P_ )
       , S_()
    ```
