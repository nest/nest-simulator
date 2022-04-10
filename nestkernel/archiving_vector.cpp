/*
 *  archiving_vector.cpp
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
#include "archiving_vector.h"

// Includes from nestkernel:
#include "kernel_manager.h"

// Includes from sli:
#include "dictutils.h"
#include "structural_plasticity_node.h"

namespace nest
{

nest::ArchivingVector::ArchivingVector()
  : n_incoming_( 0 )
  , Kminus_( 0 )
  , Kminus_triplet_( 0 )
  , tau_minus_( 0 )
  , tau_minus_inv_( 0 )
  , tau_minus_triplet_( 0 )
  , tau_minus_triplet_inv_( 0 )
  , max_delay_( 0 )
  , trace_( 0 )
  , last_spike_( 0 )
  , history_( 0 )
{
}
nest::ArchivingVector::ArchivingVector( const ArchivingVector& n )
{
  throw std::runtime_error( "ArchivingVector shouldn't be cloned" );
}

void
ArchivingVector::resize( index extended_space )
{
  index total_space = size() + extended_space;

  n_incoming_.resize( total_space, 0 );
  Kminus_triplet_.resize( total_space, 0.0 );
  Kminus_.resize( total_space, 0.0 );
  tau_minus_.resize( total_space, 20.0 );
  tau_minus_inv_.resize( total_space, 1 / 20.0 );
  tau_minus_triplet_.resize( total_space, 110.0 );
  tau_minus_triplet_inv_.resize( total_space, 1 / 110.0 );
  max_delay_.resize( total_space, 0.0 );
  trace_.resize( total_space, 0.0 );
  last_spike_.resize( total_space, -1.0 );
  history_.resize( total_space, std::deque< histentry >() );


  StructuralPlasticityVector::resize( extended_space );
}

void
nest::ArchivingVector::register_stdp_connection( double t_first_read, double delay, index local_id )
{
  for ( std::deque< histentry >::iterator runner = history_.at( local_id ).begin();
        runner != history_.at( local_id ).end()
        and ( t_first_read - runner->t_ > -1.0 * kernel().connection_manager.get_stdp_eps() );
        ++runner )
  {
    ( runner->access_counter_ )++;
  }

  n_incoming_.at( local_id )++;

  max_delay_.at( local_id ) = std::max( delay, max_delay_.at( local_id ) );
}
double
nest::ArchivingVector::get_K_value( double t, index local_id )
{
  // case when the neuron has not yet spiked
  if ( history_.at( local_id ).empty() )
  {
    trace_.at( local_id ) = 0.;
    return trace_.at( local_id );
  }

  // search for the latest post spike in the history buffer that came strictly
  // before `t`
  int i = history_.at( local_id ).size() - 1;
  while ( i >= 0 )
  {
    if ( t - history_.at( local_id )[ i ].t_ > kernel().connection_manager.get_stdp_eps() )
    {
      trace_.at( local_id ) = ( history_.at( local_id )[ i ].Kminus_
        * std::exp( ( history_.at( local_id )[ i ].t_ - t ) * tau_minus_inv_.at( local_id ) ) );
      return trace_.at( local_id );
    }
    --i;
  }

  // this case occurs when the trace was requested at a time precisely at or
  // before the first spike in the history
  trace_.at( local_id ) = 0.;
  return trace_.at( local_id );
}
void
nest::ArchivingVector::get_K_values( double t,
  double& K_value,
  double& nearest_neighbor_K_value,
  double& K_triplet_value,
  index local_id )
{
  // case when the neuron has not yet spiked
  if ( history_.at( local_id ).empty() )
  {
    K_triplet_value = Kminus_triplet_.at( local_id );
    nearest_neighbor_K_value = Kminus_.at( local_id );
    K_value = Kminus_.at( local_id );
    return;
  }

  // search for the latest post spike in the history buffer that came strictly
  // before `t`
  int i = history_.at( local_id ).size() - 1;
  while ( i >= 0 )
  {
    if ( t - history_.at( local_id )[ i ].t_ > kernel().connection_manager.get_stdp_eps() )
    {
      K_triplet_value = ( history_.at( local_id )[ i ].Kminus_triplet_
        * std::exp( ( history_.at( local_id )[ i ].t_ - t ) * tau_minus_triplet_inv_.at( local_id ) ) );
      K_value = ( history_.at( local_id )[ i ].Kminus_
        * std::exp( ( history_.at( local_id )[ i ].t_ - t ) * tau_minus_inv_.at( local_id ) ) );
      nearest_neighbor_K_value = std::exp( ( history_.at( local_id )[ i ].t_ - t ) * tau_minus_inv_.at( local_id ) );
      return;
    }
    --i;
  }

  // this case occurs when the trace was requested at a time precisely at or
  // before the first spike in the history
  K_triplet_value = 0.0;
  nearest_neighbor_K_value = 0.0;
  K_value = 0.0;
}
void
nest::ArchivingVector::get_history( double t1,
  double t2,
  std::deque< histentry >::iterator* start,
  std::deque< histentry >::iterator* finish,
  index local_id )
{
  *finish = history_.at( local_id ).end();
  if ( history_.at( local_id ).empty() )
  {
    *start = *finish;
    return;
  }
  std::deque< histentry >::reverse_iterator runner = history_.at( local_id ).rbegin();
  const double t2_lim = t2 + kernel().connection_manager.get_stdp_eps();
  const double t1_lim = t1 + kernel().connection_manager.get_stdp_eps();
  while ( runner != history_.at( local_id ).rend() and runner->t_ >= t2_lim )
  {
    ++runner;
  }
  *finish = runner.base();
  while ( runner != history_.at( local_id ).rend() and runner->t_ >= t1_lim )
  {
    runner->access_counter_++;
    ++runner;
  }
  *start = runner.base();
}


void
nest::ArchivingVector::set_spiketime( const Time& t_sp, index local_id, double offset )
{
  StructuralPlasticityVector::set_spiketime( t_sp, local_id, offset );

  const double t_sp_ms = t_sp.get_ms() - offset;

  if ( n_incoming_.at( local_id ) )
  {
    // prune all spikes from history which are no longer needed
    // only remove a spike if:
    // - its access counter indicates it has been read out by all connected
    //   STDP synapses, and
    // - there is another, later spike, that is strictly more than
    //   (max_delay_ + eps) away from the new spike (at t_sp_ms)
    while ( history_.at( local_id ).size() > 1 )
    {
      const double next_t_sp = history_.at( local_id )[ 1 ].t_;
      if ( history_.at( local_id ).front().access_counter_ >= n_incoming_.at( local_id )
        and t_sp_ms - next_t_sp > max_delay_.at( local_id ) + kernel().connection_manager.get_stdp_eps() )
      {
        history_.at( local_id ).pop_front();
      }
      else
      {
        break;
      }
    }
    // update spiking history
    Kminus_.at( local_id ) =
      Kminus_.at( local_id ) * std::exp( ( last_spike_.at( local_id ) - t_sp_ms ) * tau_minus_inv_.at( local_id ) )
      + 1.0;
    Kminus_triplet_.at( local_id ) = Kminus_triplet_.at( local_id )
        * std::exp( ( last_spike_.at( local_id ) - t_sp_ms ) * tau_minus_triplet_inv_.at( local_id ) )
      + 1.0;
    last_spike_.at( local_id ) = t_sp_ms;
    history_.at( local_id )
      .push_back( histentry( last_spike_.at( local_id ), Kminus_.at( local_id ), Kminus_triplet_.at( local_id ), 0 ) );
  }
  else
  {
    last_spike_.at( local_id ) = t_sp_ms;
  }
}
void
nest::ArchivingVector::get_status( DictionaryDatum& d, index local_id ) const
{
  def< double >( d, names::t_spike, get_spiketime_ms( local_id ) );
  def< double >( d, names::tau_minus, tau_minus_.at( local_id ) );
  def< double >( d, names::tau_minus_triplet, tau_minus_triplet_.at( local_id ) );
  def< double >( d, names::post_trace, trace_.at( local_id ) );
#ifdef DEBUG_ARCHIVER
  def< int >( d, names::archiver_length, history_.at( local_id ).size() );
#endif

  // add status dict items from the parent class
  StructuralPlasticityVector::get_status( d, local_id );
}
void
nest::ArchivingVector::set_status( const DictionaryDatum& d, index local_id )
{
  // We need to preserve values in case invalid values are set
  double new_tau_minus = tau_minus_.at( local_id );
  double new_tau_minus_triplet = tau_minus_triplet_.at( local_id );
  updateValue< double >( d, names::tau_minus, new_tau_minus );
  updateValue< double >( d, names::tau_minus_triplet, new_tau_minus_triplet );

  if ( new_tau_minus <= 0.0 or new_tau_minus_triplet <= 0.0 )
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }

  StructuralPlasticityVector::set_status( d, local_id );

  // do the actual update
  tau_minus_.at( local_id ) = new_tau_minus;
  tau_minus_triplet_.at( local_id ) = new_tau_minus_triplet;
  tau_minus_inv_.at( local_id ) = 1. / new_tau_minus;
  tau_minus_triplet_inv_.at( local_id ) = 1. / new_tau_minus_triplet;

  // check, if to clear spike history and K_minus
  bool clear = false;
  updateValue< bool >( d, names::clear, clear );
  if ( clear )
  {
    clear_history( local_id );
  }
}
void
nest::ArchivingVector::clear_history( index local_id )
{
  last_spike_.at( local_id ) = -1.0;
  Kminus_.at( local_id ) = 0.0;
  Kminus_triplet_.at( local_id ) = 0.0;
  history_.at( local_id ).clear();
}
} // of namespace nest
