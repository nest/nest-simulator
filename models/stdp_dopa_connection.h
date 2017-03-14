/*
 *  stdp_dopa_connection.h
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef STDP_DOPA_CONNECTION_H
#define STDP_DOPA_CONNECTION_H

/* BeginDocumentation

   Name: stdp_dopamine_synapse - Synapse type for dopamine-modulated
                                 spike-timing dependent plasticity.

   Description:
   stdp_dopamine_synapse is a connection to create synapses with
   dopamine-modulated spike-timing dependent plasticity (used as a
   benchmark model in [1], based on [2]). The dopaminergic signal is a
   low-pass filtered version of the spike rate of a user-specific pool
   of neurons. The spikes emitted by the pool of dopamine neurons are
   delivered to the synapse via the assigned volume transmitter. The
   dopaminergic dynamics is calculated in the synapse itself.

   Examples:
   /volume_transmitter Create /vol Set
   /iaf_psc_alpha Create /pre_neuron Set
   /iaf_psc_alpha Create /post_neuron Set
   /iaf_psc_alpha Create /neuromod_neuron Set
   /stdp_dopamine_synapse  << /vt vol >>  SetDefaults
   neuromod_neuron vol Connect
   pre_neuron post_neuron /stdp_dopamine_synapse Connect

   Parameters:
     Common properties:
           vt        long   - ID of volume_transmitter collecting the spikes
                              from the pool of dopamine releasing neurons and
                              transmitting the spikes to the synapse. A value of
                              -1 indicates that no volume transmitter has been
                              assigned.
           A_plus    double - Amplitude of weight change for facilitation
           A_minus   double - Amplitude of weight change for depression
           tau_plus  double - STDP time constant for facilitation in ms
           tau_c     double - Time constant of eligibility trace in ms
           tau_n     double - Time constant of dopaminergic trace in ms
           b         double - Dopaminergic baseline concentration
           Wmin      double - Minimal synaptic weight
           Wmax      double - Maximal synaptic weight

     Individual properties:
           c         double - eligibility trace
           n         double - neuromodulator concentration

   Remarks:
     The common properties can only be set by SetDefaults and apply to all
     synapses of the model.

   References:
   [1] Potjans W, Morrison A and Diesmann M (2010). Enabling
       functional neural circuit simulations with distributed
       computing of neuromodulated plasticity.
       Front. Comput. Neurosci. 4:141. doi:10.3389/fncom.2010.00141
   [2] Izhikevich, E.M. (2007). Solving the distal reward problem
       through linkage of STDP and dopamine signaling. Cereb. Cortex,
       17(10), 2443-2452.

   Transmits: SpikeEvent

   Author: Susanne Kunkel
   Remarks:
   - based on an earlier version by Wiebke Potjans
   - major changes to code after code revision in Apr 2013

   SeeAlso: volume_transmitter
*/

// Includes from libnestutil:
#include "numerics.h"

// Includes from models:
#include "volume_transmitter.h"

// Includes from nestkernel:
#include "connection.h"
#include "spikecounter.h"

namespace nest
{

/**
 * Class containing the common properties for all synapses of type dopamine
 * connection.
 */
class STDPDopaCommonProperties : public CommonSynapseProperties
{
public:
  /**
   * Default constructor.
   * Sets all property values to defaults.
   */
  STDPDopaCommonProperties();

  /**
   * Get all properties and put them into a dictionary.
   */
  void get_status( DictionaryDatum& d ) const;

  /**
   * Set properties from the values given in dictionary.
   */
  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  Node* get_node();

  long get_vt_gid() const;

  volume_transmitter* vt_;
  double A_plus_;
  double A_minus_;
  double tau_plus_;
  double tau_c_;
  double tau_n_;
  double b_;
  double Wmin_;
  double Wmax_;
};

inline long
STDPDopaCommonProperties::get_vt_gid() const
{
  if ( vt_ != 0 )
  {
    return vt_->get_gid();
  }
  else
  {
    return -1;
  }
}

/**
 * Class representing an STDPDopaConnection with homogeneous parameters,
 * i.e. parameters are the same for all synapses.
 */
template < typename targetidentifierT >
class STDPDopaConnection : public Connection< targetidentifierT >
{

public:
  typedef STDPDopaCommonProperties CommonPropertiesType;
  typedef Connection< targetidentifierT > ConnectionBase;

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  STDPDopaConnection();

  /**
   * Copy constructor from a property object.
   * Needs to be defined properly in order for GenericConnector to work.
   */
  STDPDopaConnection( const STDPDopaConnection& );

  // Explicitly declare all methods inherited from the dependent base
  // ConnectionBase. This avoids explicit name prefixes in all places these
  // functions are used. Since ConnectionBase depends on the template parameter,
  // they are not automatically found in the base class.
  using ConnectionBase::get_delay;
  using ConnectionBase::get_delay_steps;
  using ConnectionBase::get_rport;
  using ConnectionBase::get_target;

  /**
   * Get all properties of this connection and put them into a dictionary.
   */
  void get_status( DictionaryDatum& d ) const;

  /**
   * Set properties of this connection from the values given in dictionary.
   */
  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  /**
   * Send an event to the receiver of this connection.
   * \param e The event to send
   */
  void send( Event& e, thread t, double, const STDPDopaCommonProperties& cp );

  void trigger_update_weight( thread t,
    const std::vector< spikecounter >& dopa_spikes,
    double t_trig,
    const STDPDopaCommonProperties& cp );

  class ConnTestDummyNode : public ConnTestDummyNodeBase
  {
  public:
    // Ensure proper overriding of overloaded virtual functions.
    // Return values from functions are ignored.
    using ConnTestDummyNodeBase::handles_test_event;
    port
    handles_test_event( SpikeEvent&, rport )
    {
      return invalid_port_;
    }
  };

  /*
   * This function calls check_connection on the sender and checks if the
   * receiver accepts the event type and receptor type requested by the sender.
   * Node::check_connection() will either confirm the receiver port by returning
   * true or false if the connection should be ignored.
   * We have to override the base class' implementation, since for STDP
   * connections we have to call register_stdp_pl_connection on the target
   * neuron to inform the Archiver to collect spikes for this connection.
   * Further, the STDP dopamine synapse requires a volume transmitter to be set
   * before any simulation is performed. Checking this satisfies ticket #926.
   *
   * \param s The source node
   * \param r The target node
   * \param receptor_type The ID of the requested receptor type
   * \param t_lastspike last spike produced by presynaptic neuron (in ms)
   */
  void
  check_connection( Node& s,
    Node& t,
    rport receptor_type,
    double t_lastspike,
    const CommonPropertiesType& cp )
  {
    if ( cp.vt_ == 0 )
      throw BadProperty(
        "No volume transmitter has been assigned to the dopamine synapse." );

    ConnTestDummyNode dummy_target;
    ConnectionBase::check_connection_( dummy_target, s, t, receptor_type );

    t.register_stdp_connection( t_lastspike - get_delay() );
  }

  void
  set_weight( double w )
  {
    weight_ = w;
  }

private:
  // update dopamine trace from last to current dopamine spike and increment
  // index
  void update_dopamine_( const std::vector< spikecounter >& dopa_spikes,
    const STDPDopaCommonProperties& cp );

  void update_weight_( double c0,
    double n0,
    double minus_dt,
    const STDPDopaCommonProperties& cp );

  void process_dopa_spikes_( const std::vector< spikecounter >& dopa_spikes,
    double t0,
    double t1,
    const STDPDopaCommonProperties& cp );
  void facilitate_( double kplus, const STDPDopaCommonProperties& cp );
  void depress_( double kminus, const STDPDopaCommonProperties& cp );

  // data members of each connection
  double weight_;
  double Kplus_;
  double c_;
  double n_;

  // dopa_spikes_idx_ refers to the dopamine spike that has just been processes
  // after trigger_update_weight a pseudo dopamine spike at t_trig is stored at
  // index 0 and dopa_spike_idx_ = 0
  index dopa_spikes_idx_;

  // time of last update, which is either time of last presyn. spike or
  // time-driven update
  double t_last_update_;
};

//
// Implementation of class STDPDopaConnection.
//

template < typename targetidentifierT >
STDPDopaConnection< targetidentifierT >::STDPDopaConnection()
  : ConnectionBase()
  , weight_( 1.0 )
  , Kplus_( 0.0 )
  , c_( 0.0 )
  , n_( 0.0 )
  , dopa_spikes_idx_( 0 )
  , t_last_update_( 0.0 )
{
}

template < typename targetidentifierT >
STDPDopaConnection< targetidentifierT >::STDPDopaConnection(
  const STDPDopaConnection& rhs )
  : ConnectionBase( rhs )
  , weight_( rhs.weight_ )
  , Kplus_( rhs.Kplus_ )
  , c_( rhs.c_ )
  , n_( rhs.n_ )
  , dopa_spikes_idx_( rhs.dopa_spikes_idx_ )
  , t_last_update_( rhs.t_last_update_ )
{
}

template < typename targetidentifierT >
void
STDPDopaConnection< targetidentifierT >::get_status( DictionaryDatum& d ) const
{

  // base class properties, different for individual synapse
  ConnectionBase::get_status( d );
  def< double >( d, names::weight, weight_ );

  // own properties, different for individual synapse
  def< double >( d, "c", c_ );
  def< double >( d, "n", n_ );
}

template < typename targetidentifierT >
void
STDPDopaConnection< targetidentifierT >::set_status( const DictionaryDatum& d,
  ConnectorModel& cm )
{
  // base class properties
  ConnectionBase::set_status( d, cm );
  updateValue< double >( d, names::weight, weight_ );

  updateValue< double >( d, "c", c_ );
  updateValue< double >( d, "n", n_ );
}

template < typename targetidentifierT >
inline void
STDPDopaConnection< targetidentifierT >::update_dopamine_(
  const std::vector< spikecounter >& dopa_spikes,
  const STDPDopaCommonProperties& cp )
{
  double minus_dt = dopa_spikes[ dopa_spikes_idx_ ].spike_time_
    - dopa_spikes[ dopa_spikes_idx_ + 1 ].spike_time_;
  ++dopa_spikes_idx_;
  n_ = n_ * std::exp( minus_dt / cp.tau_n_ )
    + dopa_spikes[ dopa_spikes_idx_ ].multiplicity_ / cp.tau_n_;
}

template < typename targetidentifierT >
inline void
STDPDopaConnection< targetidentifierT >::update_weight_( double c0,
  double n0,
  double minus_dt,
  const STDPDopaCommonProperties& cp )
{
  const double taus_ = ( cp.tau_c_ + cp.tau_n_ ) / ( cp.tau_c_ * cp.tau_n_ );
  weight_ = weight_
    - c0 * ( n0 / taus_ * numerics::expm1( taus_ * minus_dt )
             - cp.b_ * cp.tau_c_ * numerics::expm1( minus_dt / cp.tau_c_ ) );

  if ( weight_ < cp.Wmin_ )
    weight_ = cp.Wmin_;
  if ( weight_ > cp.Wmax_ )
    weight_ = cp.Wmax_;
}

template < typename targetidentifierT >
inline void
STDPDopaConnection< targetidentifierT >::process_dopa_spikes_(
  const std::vector< spikecounter >& dopa_spikes,
  double t0,
  double t1,
  const STDPDopaCommonProperties& cp )
{
  // process dopa spikes in (t0, t1]
  // propagate weight from t0 to t1
  if ( ( dopa_spikes.size() > dopa_spikes_idx_ + 1 )
    && ( dopa_spikes[ dopa_spikes_idx_ + 1 ].spike_time_ <= t1 ) )
  {
    // there is at least 1 dopa spike in (t0, t1]
    // propagate weight up to first dopa spike and update dopamine trace
    // weight and eligibility c are at time t0 but dopamine trace n is at time
    // of last dopa spike
    double n0 =
      n_ * std::exp( ( dopa_spikes[ dopa_spikes_idx_ ].spike_time_ - t0 )
             / cp.tau_n_ ); // dopamine trace n at time t0
    update_weight_(
      c_, n0, t0 - dopa_spikes[ dopa_spikes_idx_ + 1 ].spike_time_, cp );
    update_dopamine_( dopa_spikes, cp );

    // process remaining dopa spikes in (t0, t1]
    double cd;
    while ( ( dopa_spikes.size() > dopa_spikes_idx_ + 1 )
      && ( dopa_spikes[ dopa_spikes_idx_ + 1 ].spike_time_ <= t1 ) )
    {
      // propagate weight up to next dopa spike and update dopamine trace
      // weight and dopamine trace n are at time of last dopa spike td but
      // eligibility c is at time
      // t0
      cd = c_ * std::exp( ( t0 - dopa_spikes[ dopa_spikes_idx_ ].spike_time_ )
                  / cp.tau_c_ ); // eligibility c at time of td
      update_weight_( cd,
        n_,
        dopa_spikes[ dopa_spikes_idx_ ].spike_time_
          - dopa_spikes[ dopa_spikes_idx_ + 1 ].spike_time_,
        cp );
      update_dopamine_( dopa_spikes, cp );
    }

    // propagate weight up to t1
    // weight and dopamine trace n are at time of last dopa spike td but
    // eligibility c is at time t0
    cd = c_ * std::exp( ( t0 - dopa_spikes[ dopa_spikes_idx_ ].spike_time_ )
                / cp.tau_c_ ); // eligibility c at time td
    update_weight_(
      cd, n_, dopa_spikes[ dopa_spikes_idx_ ].spike_time_ - t1, cp );
  }
  else
  {
    // no dopamine spikes in (t0, t1]
    // weight and eligibility c are at time t0 but dopamine trace n is at time
    // of last dopa spike
    double n0 =
      n_ * std::exp( ( dopa_spikes[ dopa_spikes_idx_ ].spike_time_ - t0 )
             / cp.tau_n_ ); // dopamine trace n at time t0
    update_weight_( c_, n0, t0 - t1, cp );
  }

  // update eligibility trace c for interval (t0, t1]
  c_ = c_ * std::exp( ( t0 - t1 ) / cp.tau_c_ );
}

template < typename targetidentifierT >
inline void
STDPDopaConnection< targetidentifierT >::facilitate_( double kplus,
  const STDPDopaCommonProperties& cp )
{
  c_ += cp.A_plus_ * kplus;
}

template < typename targetidentifierT >
inline void
STDPDopaConnection< targetidentifierT >::depress_( double kminus,
  const STDPDopaCommonProperties& cp )
{
  c_ -= cp.A_minus_ * kminus;
}

/**
 * Send an event to the receiver of this connection.
 * \param e The event to send
 * \param p The port under which this connection is stored in the Connector.
 * \param t_lastspike Time point of last spike emitted
 */
template < typename targetidentifierT >
inline void
STDPDopaConnection< targetidentifierT >::send( Event& e,
  thread t,
  double,
  const STDPDopaCommonProperties& cp )
{
  // t_lastspike_ = 0 initially

  Node* target = get_target( t );

  // purely dendritic delay
  double dendritic_delay = get_delay();

  double t_spike = e.get_stamp().get_ms();

  // get history of dopamine spikes
  const std::vector< spikecounter >& dopa_spikes = cp.vt_->deliver_spikes();

  // get spike history in relevant range (t_last_update, t_spike] from
  // post-synaptic neuron
  std::deque< histentry >::iterator start;
  std::deque< histentry >::iterator finish;
  target->get_history( t_last_update_ - dendritic_delay,
    t_spike - dendritic_delay,
    &start,
    &finish );

  // facilitation due to post-synaptic spikes since last update
  double t0 = t_last_update_;
  double minus_dt;
  while ( start != finish )
  {
    process_dopa_spikes_( dopa_spikes, t0, start->t_ + dendritic_delay, cp );
    t0 = start->t_ + dendritic_delay;
    minus_dt = t_last_update_ - t0;
    if ( start->t_ < t_spike ) // only depression if pre- and postsyn. spike
                               // occur at the same time
      facilitate_( Kplus_ * std::exp( minus_dt / cp.tau_plus_ ), cp );
    ++start;
  }

  // depression due to new pre-synaptic spike
  process_dopa_spikes_( dopa_spikes, t0, t_spike, cp );
  depress_( target->get_K_value( t_spike - dendritic_delay ), cp );

  e.set_receiver( *target );
  e.set_weight( weight_ );
  e.set_delay( get_delay_steps() );
  e.set_rport( get_rport() );
  e();

  Kplus_ =
    Kplus_ * std::exp( ( t_last_update_ - t_spike ) / cp.tau_plus_ ) + 1.0;
  t_last_update_ = t_spike;
}

template < typename targetidentifierT >
inline void
STDPDopaConnection< targetidentifierT >::trigger_update_weight( thread t,
  const std::vector< spikecounter >& dopa_spikes,
  const double t_trig,
  const STDPDopaCommonProperties& cp )
{
  // propagate all state variables to time t_trig
  // this does not include the depression trace K_minus, which is updated in the
  // postsyn. neuron

  // purely dendritic delay
  double dendritic_delay = get_delay();

  // get spike history in relevant range (t_last_update, t_trig] from postsyn.
  // neuron
  std::deque< histentry >::iterator start;
  std::deque< histentry >::iterator finish;
  get_target( t )->get_history( t_last_update_ - dendritic_delay,
    t_trig - dendritic_delay,
    &start,
    &finish );

  // facilitation due to postsyn. spikes since last update
  double t0 = t_last_update_;
  double minus_dt;
  while ( start != finish )
  {
    process_dopa_spikes_( dopa_spikes, t0, start->t_ + dendritic_delay, cp );
    t0 = start->t_ + dendritic_delay;
    minus_dt = t_last_update_ - t0;
    facilitate_( Kplus_ * std::exp( minus_dt / cp.tau_plus_ ), cp );
    ++start;
  }

  // propagate weight, eligibility trace c, dopamine trace n and facilitation
  // trace K_plus to time t_trig but do not increment/decrement as there are no
  // spikes to be handled at t_trig
  process_dopa_spikes_( dopa_spikes, t0, t_trig, cp );
  n_ = n_ * std::exp( ( dopa_spikes[ dopa_spikes_idx_ ].spike_time_ - t_trig )
              / cp.tau_n_ );
  Kplus_ = Kplus_ * std::exp( ( t_last_update_ - t_trig ) / cp.tau_plus_ );

  t_last_update_ = t_trig;
  dopa_spikes_idx_ = 0;
}

} // of namespace nest

#endif // of #ifndef STDP_DOPA_CONNECTION_H
