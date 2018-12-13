/*
 *  archiving_node.cpp
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

/**
 * \file archiving_node.cpp
 * Implementation of archiving_node to record and manage spike history
 * \author Moritz Helias, Abigail Morrison
 * \date april 2006
 */

#include "archiving_node.h"

// Includes from nestkernel:
#include "kernel_manager.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{

// member functions for Archiving_Node

nest::Archiving_Node::Archiving_Node()
  : n_incoming_( 0 )
  , Kminus_( 0.0 )
  , triplet_Kminus_( 0.0 )
  , tau_minus_( 20.0 )
  , tau_minus_inv_( 1. / tau_minus_ )
  , tau_minus_triplet_( 110.0 )
  , tau_minus_triplet_inv_( 1. / tau_minus_triplet_ )
  , last_spike_( -1.0 )
  , Ca_t_( 0.0 )
  , Ca_minus_( 0.0 )
  , tau_Ca_( 10000.0 )
  , beta_Ca_( 0.001 )
  , synaptic_elements_map_()
{
}

nest::Archiving_Node::Archiving_Node( const Archiving_Node& n )
  : Node( n )
  , n_incoming_( n.n_incoming_ )
  , Kminus_( n.Kminus_ )
  , triplet_Kminus_( n.triplet_Kminus_ )
  , tau_minus_( n.tau_minus_ )
  , tau_minus_inv_( n.tau_minus_inv_ )
  , tau_minus_triplet_( n.tau_minus_triplet_ )
  , tau_minus_triplet_inv_( n.tau_minus_triplet_inv_ )
  , last_spike_( n.last_spike_ )
  , Ca_t_( n.Ca_t_ )
  , Ca_minus_( n.Ca_minus_ )
  , tau_Ca_( n.tau_Ca_ )
  , beta_Ca_( n.beta_Ca_ )
  , synaptic_elements_map_( n.synaptic_elements_map_ )
{
}

void
Archiving_Node::register_stdp_connection( double t_first_read )
{
  // Mark all entries in the deque, which we will not read in future as read by
  // this input input, so that we savely increment the incoming number of
  // connections afterwards without leaving spikes in the history.
  // For details see bug #218. MH 08-04-22

  for ( std::deque< histentry >::iterator runner = history_.begin();
        runner != history_.end()
          and ( t_first_read - runner->t_ > -1.0
                  * kernel().connection_manager.get_stdp_eps() );
        ++runner )
  {
    ( runner->access_counter_ )++;
  }

  n_incoming_++;
}

double
nest::Archiving_Node::get_K_value( double t )
{
  if ( history_.empty() )
  {
    return Kminus_;
  }
  int i = history_.size() - 1;
  while ( i >= 0 )
  {
    if ( t - history_[ i ].t_ > kernel().connection_manager.get_stdp_eps() )
    {
      return ( history_[ i ].Kminus_
        * std::exp( ( history_[ i ].t_ - t ) * tau_minus_inv_ ) );
    }
    i--;
  }
  return 0;
}

void
nest::Archiving_Node::get_K_values( double t,
  double& K_value,
  double& triplet_K_value )
{
  // case when the neuron has not yet spiked
  if ( history_.empty() )
  {
    triplet_K_value = triplet_Kminus_;
    K_value = Kminus_;
    return;
  }
  // case
  int i = history_.size() - 1;
  while ( i >= 0 )
  {
    if ( t - history_[ i ].t_ > kernel().connection_manager.get_stdp_eps() )
    {
      triplet_K_value = ( history_[ i ].triplet_Kminus_
        * std::exp( ( history_[ i ].t_ - t ) * tau_minus_triplet_inv_ ) );
      K_value = ( history_[ i ].Kminus_
        * std::exp( ( history_[ i ].t_ - t ) * tau_minus_inv_ ) );
      return;
    }
    i--;
  }

  // we only get here if t< time of all spikes in history)

  // return 0.0 for both K values
  triplet_K_value = 0.0;
  K_value = 0.0;
}

void
nest::Archiving_Node::get_history( double t1,
  double t2,
  std::deque< histentry >::iterator* start,
  std::deque< histentry >::iterator* finish )
{
  *finish = history_.end();
  if ( history_.empty() )
  {
    *start = *finish;
    return;
  }
  std::deque< histentry >::reverse_iterator runner = history_.rbegin();
  const double t2_lim = t2 + kernel().connection_manager.get_stdp_eps();
  const double t1_lim = t1 + kernel().connection_manager.get_stdp_eps();
  while ( runner != history_.rend() and runner->t_ >= t2_lim )
  {
    ++runner;
  }
  *finish = runner.base();
  while ( runner != history_.rend() and runner->t_ >= t1_lim )
  {
    runner->access_counter_++;
    ++runner;
  }
  *start = runner.base();
}

void
nest::Archiving_Node::set_spiketime( Time const& t_sp, double offset )
{
  const double t_sp_ms = t_sp.get_ms() - offset;
  update_synaptic_elements( t_sp_ms );
  Ca_minus_ += beta_Ca_;

  if ( n_incoming_ )
  {
    // prune all spikes from history which are no longer needed
    // except the penultimate one. we might still need it.
    while ( history_.size() > 1 )
    {
      if ( history_.front().access_counter_ >= n_incoming_ )
      {
        history_.pop_front();
      }
      else
      {
        break;
      }
    }
    // update spiking history
    Kminus_ =
      Kminus_ * std::exp( ( last_spike_ - t_sp_ms ) * tau_minus_inv_ ) + 1.0;
    triplet_Kminus_ = triplet_Kminus_
        * std::exp( ( last_spike_ - t_sp_ms ) * tau_minus_triplet_inv_ )
      + 1.0;
    last_spike_ = t_sp_ms;
    history_.push_back( histentry( last_spike_, Kminus_, triplet_Kminus_, 0 ) );
  }
  else
  {
    last_spike_ = t_sp_ms;
  }
}

void
nest::Archiving_Node::get_status( DictionaryDatum& d ) const
{
  DictionaryDatum synaptic_elements_d;
  DictionaryDatum synaptic_element_d;

  def< double >( d, names::t_spike, get_spiketime_ms() );
  def< double >( d, names::tau_minus, tau_minus_ );
  def< double >( d, names::Ca, Ca_minus_ );
  def< double >( d, names::tau_Ca, tau_Ca_ );
  def< double >( d, names::beta_Ca, beta_Ca_ );
  def< double >( d, names::tau_minus_triplet, tau_minus_triplet_ );
#ifdef DEBUG_ARCHIVER
  def< int >( d, names::archiver_length, history_.size() );
#endif

  synaptic_elements_d = DictionaryDatum( new Dictionary );
  def< DictionaryDatum >( d, names::synaptic_elements, synaptic_elements_d );
  for ( std::map< Name, SynapticElement >::const_iterator it =
          synaptic_elements_map_.begin();
        it != synaptic_elements_map_.end();
        ++it )
  {
    synaptic_element_d = DictionaryDatum( new Dictionary );
    def< DictionaryDatum >(
      synaptic_elements_d, it->first, synaptic_element_d );
    it->second.get( synaptic_element_d );
  }
}

void
nest::Archiving_Node::set_status( const DictionaryDatum& d )
{
  // We need to preserve values in case invalid values are set
  double new_tau_minus = tau_minus_;
  double new_tau_minus_triplet = tau_minus_triplet_;
  double new_tau_Ca = tau_Ca_;
  double new_beta_Ca = beta_Ca_;
  updateValue< double >( d, names::tau_minus, new_tau_minus );
  updateValue< double >( d, names::tau_minus_triplet, new_tau_minus_triplet );
  updateValue< double >( d, names::tau_Ca, new_tau_Ca );
  updateValue< double >( d, names::beta_Ca, new_beta_Ca );

  if ( new_tau_minus <= 0.0 or new_tau_minus_triplet <= 0.0 )
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }

  tau_minus_ = new_tau_minus;
  tau_minus_triplet_ = new_tau_minus_triplet;
  tau_minus_inv_ = 1. / tau_minus_;
  tau_minus_triplet_inv_ = 1. / tau_minus_triplet_;

  if ( new_tau_Ca <= 0.0 )
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }
  tau_Ca_ = new_tau_Ca;

  if ( new_beta_Ca <= 0.0 )
  {
    throw BadProperty(
      "For Ca to function as an integrator of the electrical activity, beta_ca "
      "needs to be greater than 0." );
  }
  beta_Ca_ = new_beta_Ca;

  // check, if to clear spike history and K_minus
  bool clear = false;
  updateValue< bool >( d, names::clear, clear );
  if ( clear )
  {
    clear_history();
  }

  if ( d->known( names::synaptic_elements_param ) )
  {
    const DictionaryDatum synaptic_elements_dict =
      getValue< DictionaryDatum >( d, names::synaptic_elements_param );

    for ( std::map< Name, SynapticElement >::iterator it =
            synaptic_elements_map_.begin();
          it != synaptic_elements_map_.end();
          ++it )
    {
      if ( synaptic_elements_dict->known( it->first ) )
      {
        const DictionaryDatum synaptic_elements_a =
          getValue< DictionaryDatum >( synaptic_elements_dict, it->first );
        it->second.set( synaptic_elements_a );
      }
    }
  }
  if ( not d->known( names::synaptic_elements ) )
  {
    return;
  }
  // we replace the existing synaptic_elements_map_ by the new one
  DictionaryDatum synaptic_elements_d;
  std::pair< std::map< Name, SynapticElement >::iterator, bool > insert_result;

  synaptic_elements_map_ = std::map< Name, SynapticElement >();
  synaptic_elements_d =
    getValue< DictionaryDatum >( d, names::synaptic_elements );

  for ( Dictionary::const_iterator i = synaptic_elements_d->begin();
        i != synaptic_elements_d->end();
        ++i )
  {
    insert_result = synaptic_elements_map_.insert(
      std::pair< Name, SynapticElement >( i->first, SynapticElement() ) );
    ( insert_result.first->second )
      .set( getValue< DictionaryDatum >( synaptic_elements_d, i->first ) );
  }
}

void
nest::Archiving_Node::clear_history()
{
  last_spike_ = -1.0;
  Kminus_ = 0.0;
  triplet_Kminus_ = 0.0;
  history_.clear();
  Ca_minus_ = 0.0;
  Ca_t_ = 0.0;
}


/* ----------------------------------------------------------------
* Get the number of synaptic_elements
* ---------------------------------------------------------------- */
double
nest::Archiving_Node::get_synaptic_elements( Name n ) const
{
  std::map< Name, SynapticElement >::const_iterator se_it;
  se_it = synaptic_elements_map_.find( n );
  double z_value;

  if ( se_it != synaptic_elements_map_.end() )
  {
    z_value = ( se_it->second ).get_z();
    if ( ( se_it->second ).continuous() )
    {
      return z_value;
    }
    else
    {
      return std::floor( z_value );
    }
  }
  else
  {
    return 0.0;
  }
}

int
nest::Archiving_Node::get_synaptic_elements_vacant( Name n ) const
{
  std::map< Name, SynapticElement >::const_iterator se_it;
  se_it = synaptic_elements_map_.find( n );

  if ( se_it != synaptic_elements_map_.end() )
  {
    return se_it->second.get_z_vacant();
  }
  else
  {
    return 0;
  }
}

int
nest::Archiving_Node::get_synaptic_elements_connected( Name n ) const
{
  std::map< Name, SynapticElement >::const_iterator se_it;
  se_it = synaptic_elements_map_.find( n );

  if ( se_it != synaptic_elements_map_.end() )
  {
    return se_it->second.get_z_connected();
  }
  else
  {
    return 0;
  }
}

std::map< Name, double >
nest::Archiving_Node::get_synaptic_elements() const
{
  std::map< Name, double > n_map;

  for ( std::map< Name, SynapticElement >::const_iterator it =
          synaptic_elements_map_.begin();
        it != synaptic_elements_map_.end();
        ++it )
  {
    n_map.insert( std::pair< Name, double >(
      it->first, get_synaptic_elements( it->first ) ) );
  }
  return n_map;
}

void
nest::Archiving_Node::update_synaptic_elements( double t )
{
  assert( t >= Ca_t_ );

  for ( std::map< Name, SynapticElement >::iterator it =
          synaptic_elements_map_.begin();
        it != synaptic_elements_map_.end();
        ++it )
  {
    it->second.update( t, Ca_t_, Ca_minus_, tau_Ca_ );
  }
  // Update calcium concentration
  Ca_minus_ = Ca_minus_ * std::exp( ( Ca_t_ - t ) / tau_Ca_ );
  Ca_t_ = t;
}

void
nest::Archiving_Node::decay_synaptic_elements_vacant()
{
  for ( std::map< Name, SynapticElement >::iterator it =
          synaptic_elements_map_.begin();
        it != synaptic_elements_map_.end();
        ++it )
  {
    it->second.decay_z_vacant();
  }
}

void
nest::Archiving_Node::connect_synaptic_element( Name name, int n )
{
  std::map< Name, SynapticElement >::iterator se_it;
  se_it = synaptic_elements_map_.find( name );

  if ( se_it != synaptic_elements_map_.end() )
  {
    se_it->second.connect( n );
  }
}

// member functions for Clopath_Archiving_Node

nest::Clopath_Archiving_Node::Clopath_Archiving_Node()
  : Archiving_Node()
  , A_LTD_( 14.0e-5 )
  , A_LTP_( 8.0e-5 )
  , u_ref_squared_( 60.0 )
  , theta_plus_( -45.3 )
  , theta_minus_( -70.6 )
  , A_LTD_const_( true )
  , delay_u_bars_( 5.0 )
  , ltd_hist_len_( 0 )
  , ltd_hist_current_( 0 )
{
}

nest::Clopath_Archiving_Node::Clopath_Archiving_Node(
  const Clopath_Archiving_Node& n )
  : Archiving_Node( n )
  , A_LTD_( n.A_LTD_ )
  , A_LTP_( n.A_LTP_ )
  , u_ref_squared_( n.u_ref_squared_ )
  , theta_plus_( n.theta_plus_ )
  , theta_minus_( n.theta_minus_ )
  , A_LTD_const_( n.A_LTD_const_ )
  , delay_u_bars_( n.delay_u_bars_ )
  , ltd_hist_len_( n.ltd_hist_len_ )
  , ltd_hist_current_( n.ltd_hist_current_ )
{
}

void
nest::Clopath_Archiving_Node::init_clopath_buffers()
{
  // Implementation of the delay of the convolved membrane potentials.
  // This delay is not described in Clopath et al. 2010 but is present in
  // the code which was presumably used to create the figures in the paper.
  delayed_u_bars_idx_ = 0;
  delay_u_bars_steps_ = Time::delay_ms_to_steps( delay_u_bars_ ) + 1;
  delayed_u_bar_plus_.resize( delay_u_bars_steps_ );
  delayed_u_bar_minus_.resize( delay_u_bars_steps_ );

  // initialize the ltp-history
  ltd_hist_current_ = 0;
  ltd_hist_len_ = kernel().connection_manager.get_max_delay();
  ltd_history_.resize( ltd_hist_len_, histentry_cl( 0.0, 0.0, 0 ) );
}

void
nest::Clopath_Archiving_Node::get_status( DictionaryDatum& d ) const
{
  Archiving_Node::get_status( d );

  def< double >( d, names::A_LTD, A_LTD_ );
  def< double >( d, names::A_LTP, A_LTP_ );
  def< double >( d, names::u_ref_squared, u_ref_squared_ );
  def< double >( d, names::theta_plus, theta_plus_ );
  def< double >( d, names::theta_minus, theta_minus_ );
  def< bool >( d, names::A_LTD_const, A_LTD_const_ );
  def< double >( d, names::delay_u_bars, delay_u_bars_ );
}

void
nest::Clopath_Archiving_Node::set_status( const DictionaryDatum& d )
{
  Archiving_Node::set_status( d );

  // We need to preserve values in case invalid values are set
  double new_A_LTD = A_LTD_;
  double new_A_LTP = A_LTP_;
  double new_theta_plus = theta_plus_;
  double new_theta_minus = theta_minus_;
  double new_u_ref_squared = u_ref_squared_;
  double new_A_LTD_const = A_LTD_const_;
  double new_delay_u_bars = delay_u_bars_;
  updateValue< double >( d, names::A_LTD, new_A_LTD );
  updateValue< double >( d, names::A_LTP, new_A_LTP );
  updateValue< double >( d, names::u_ref_squared, new_u_ref_squared );
  updateValue< double >( d, names::theta_plus, new_theta_plus );
  updateValue< double >( d, names::theta_minus, new_theta_minus );
  updateValue< bool >( d, names::A_LTD_const, new_A_LTD_const );
  updateValue< double >( d, names::delay_u_bars, new_delay_u_bars );
  A_LTD_ = new_A_LTD;
  A_LTP_ = new_A_LTP;
  u_ref_squared_ = new_u_ref_squared;

  if ( u_ref_squared_ <= 0 )
  {
    throw BadProperty( "Ensure that u_ref_squared > 0" );
  }

  theta_plus_ = new_theta_plus;
  theta_minus_ = new_theta_minus;
  A_LTD_const_ = new_A_LTD_const;
  delay_u_bars_ = new_delay_u_bars;
}

double
nest::Clopath_Archiving_Node::get_LTD_value( double t )
{
  std::vector< histentry_cl >::iterator runner;
  if ( ltd_history_.empty() || t < 0.0 )
  {
    // Return 0.0 if history is empty
    return 0.0;
  }
  else
  {
    runner = ltd_history_.begin();
    while ( runner != ltd_history_.end() )
    {
      if ( abs( t - runner->t_ ) < 1e-4 )
      {
        // Return the corresponding value if an entry at time t exists
        return runner->dw_;
      }
      ( runner->access_counter_ )++;
      ++runner;
    }
  }
  // Return zero if there is no entry at time t
  return 0.0;
}

void
nest::Clopath_Archiving_Node::get_LTP_history( double t1,
  double t2,
  std::deque< histentry_cl >::iterator* start,
  std::deque< histentry_cl >::iterator* finish )
{
  *finish = ltp_history_.end();
  if ( ltp_history_.empty() )
  {
    *start = *finish;
    return;
  }
  else
  {
    std::deque< histentry_cl >::iterator runner = ltp_history_.begin();
    // To have a well defined discretization of the integral, we make sure
    // that we exclude the entry at t1 but include the one at t2 by subtracting
    // a small number so that runner->t_ is never equal to t1 or t2.
    while ( ( runner != ltp_history_.end() ) && ( runner->t_ - 1.0e-6 < t1 ) )
    {
      ++runner;
    }
    *start = runner;
    while ( ( runner != ltp_history_.end() ) && ( runner->t_ - 1.0e-6 < t2 ) )
    {
      ( runner->access_counter_ )++;
      ++runner;
    }
    *finish = runner;
  }
}

void
nest::Clopath_Archiving_Node::write_LTP_LTD_history( Time const& t_sp,
  double u,
  double u_bar_plus,
  double u_bar_minus,
  double u_bar_bar )
{
  const double t_ms = t_sp.get_ms();

  // write u_bar_p/m in ring buffer
  delayed_u_bar_plus_[ delayed_u_bars_idx_ ] = u_bar_plus;

  delayed_u_bar_minus_[ delayed_u_bars_idx_ ] = u_bar_minus;

  // increment pointer
  delayed_u_bars_idx_ = ( delayed_u_bars_idx_ + 1 ) % delay_u_bars_steps_;

  // read oldest values from buffers
  double del_u_bar_plus = delayed_u_bar_plus_[ delayed_u_bars_idx_ ];
  double del_u_bar_minus = delayed_u_bar_minus_[ delayed_u_bars_idx_ ];

  // save data for Clopath STDP if necessary
  if ( ( u > theta_plus_ ) && ( del_u_bar_plus > theta_minus_ ) )
  {
    write_LTP_history( t_ms, u, del_u_bar_plus );
  }

  if ( del_u_bar_minus > theta_minus_ )
  {
    write_LTD_history( t_ms, del_u_bar_minus, u_bar_bar );
  }
}

void
nest::Clopath_Archiving_Node::write_LTD_history( const double t_ltd_ms,
  double u_bar_minus,
  double u_bar_bar )
{
  if ( n_incoming_ )
  {
    const double dw = A_LTD_const_ ? A_LTD_ * ( u_bar_minus - theta_minus_ )
                                   : A_LTD_ * u_bar_bar * u_bar_bar
        * ( u_bar_minus - theta_minus_ ) / u_ref_squared_;
    ltd_history_[ ltd_hist_current_ ] = histentry_cl( t_ltd_ms, dw, 0 );
    ltd_hist_current_ = ( ltd_hist_current_ + 1 ) % ltd_hist_len_;
  }
}

void
nest::Clopath_Archiving_Node::write_LTP_history( const double t_ltp_ms,
  double u,
  double u_bar_plus )
{
  if ( n_incoming_ )
  {
    // prune all entries from history which are no longer needed
    // except the penultimate one. we might still need it.
    while ( ltp_history_.size() > 1 )
    {
      if ( ltp_history_.front().access_counter_ >= n_incoming_ )
      {
        ltp_history_.pop_front();
      }
      else
      {
        break;
      }
    }
    // dw is not the change of the synaptic weight since the factor
    // x_bar is not included (but later in the synapse)
    const double dw = A_LTP_ * ( u - theta_plus_ )
      * ( u_bar_plus - theta_minus_ ) * Time::get_resolution().get_ms();
    ltp_history_.push_back( histentry_cl( t_ltp_ms, dw, 0 ) );
  }
}

} // of namespace nest
