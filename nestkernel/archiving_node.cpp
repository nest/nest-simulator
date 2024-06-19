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

#include "archiving_node.h"

// Includes from nestkernel:
#include "kernel_manager.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{

// member functions for ArchivingNode

nest::ArchivingNode::ArchivingNode()
  : n_incoming_( 0 )
  , Kminus_( 0.0 )
  , Kminus_triplet_( 0.0 )
  , tau_minus_( 1.0 ) // TODO: signals that tau_minus_ has not been set
  , tau_minus_inv_( 1. / tau_minus_ )
  , tau_minus_triplet_( 110.0 )
  , tau_minus_triplet_inv_( 1. / tau_minus_triplet_ )
  , max_delay_( 0 )
  , trace_( 0.0 )
  , last_spike_( -1 )
{
}

nest::ArchivingNode::ArchivingNode( const ArchivingNode& n )
  : StructuralPlasticityNode( n )
  , n_incoming_( n.n_incoming_ )
  , Kminus_( n.Kminus_ )
  , Kminus_triplet_( n.Kminus_triplet_ )
  , tau_minus_( 1.0 ) // TODO: signals that tau_minus_ has not been set
  , tau_minus_inv_( 1. / tau_minus_ )
  , tau_minus_triplet_( n.tau_minus_triplet_ )
  , tau_minus_triplet_inv_( n.tau_minus_triplet_inv_ )
  , max_delay_( n.max_delay_ )
  , trace_( n.trace_ )
  , last_spike_( n.last_spike_ )
{
}

void
ArchivingNode::register_stdp_connection( size_t t_first_read, size_t delay, const double tau_minus )
{
  if ( tau_minus_ > 1.0 && tau_minus_ != tau_minus )
  {
    throw IllegalConnection();
  }
  else
  {
    tau_minus_ = tau_minus;
    tau_minus_inv_ = 1. / tau_minus_;
  }

  // Mark all entries in the deque, which we will not read in future as read by
  // this input, so that we safely increment the incoming number of
  // connections afterwards without leaving spikes in the history.
  // For details see bug #218. MH 08-04-22
  for ( std::deque< histentry >::iterator runner = history_.begin();
        runner != history_.end() and ( t_first_read - runner->t_ >= 0 );
        ++runner )
  {
    ( runner->access_counter_ )++;
  }

  n_incoming_++;

  max_delay_ = std::max( delay, max_delay_ );
}

double
nest::ArchivingNode::get_K_value( long t, size_t& dt_steps )
{
  dt_steps = 0;
  // case when the neuron has not yet spiked
  if ( history_.empty() )
  {
    trace_ = 0.;
    return trace_;
  }

  int i = history_.size() - 1;

  while ( i >= 0 )
  {
    const auto hist = history_[ i ];
    if ( t > static_cast< long >( hist.t_ ) )
    {
      trace_ = hist.Kminus_;
      dt_steps = t - hist.t_;

      return trace_;
    }
    --i;
  }

  // this case occurs when the trace was requested at a time precisely at or
  // before the first spike in the history
  trace_ = 0.;
  return trace_;
}

void
nest::ArchivingNode::get_K_values( double t,
  double& K_value,
  double& nearest_neighbor_K_value,
  double& K_triplet_value )
{
  // case when the neuron has not yet spiked
  if ( history_.empty() )
  {
    K_triplet_value = Kminus_triplet_;
    nearest_neighbor_K_value = Kminus_;
    K_value = Kminus_;
    return;
  }

  // search for the latest post spike in the history buffer that came strictly
  // before `t`
  int i = history_.size() - 1;
  while ( i >= 0 )
  {
    if ( t - history_[ i ].t_ > kernel().connection_manager.get_stdp_eps() )
    {
      K_triplet_value =
        ( history_[ i ].Kminus_triplet_ * std::exp( ( history_[ i ].t_ - t ) * tau_minus_triplet_inv_ ) );
      K_value = ( history_[ i ].Kminus_ * std::exp( ( history_[ i ].t_ - t ) * tau_minus_inv_ ) );
      nearest_neighbor_K_value = std::exp( ( history_[ i ].t_ - t ) * tau_minus_inv_ );
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
nest::ArchivingNode::get_history( long t1,
  long t2,
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
  while ( runner != history_.rend() and static_cast< long >( runner->t_ ) > t2 )
  {
    ++runner;
  }

  *finish = runner.base();
  while ( runner != history_.rend() and static_cast< long >( runner->t_ ) > t1 )
  {
    runner->access_counter_++;
    ++runner;
  }
  *start = runner.base();
}

void
nest::ArchivingNode::set_spiketime( Time const& t_sp, double offset )
{
  StructuralPlasticityNode::set_spiketime( t_sp, offset );

  const size_t t_sp_ms = t_sp.get_steps();

  if ( n_incoming_ )
  {
    // prune all spikes from history which are no longer needed
    // only remove a spike if:
    // - its access counter indicates it has been read out by all connected
    //   STDP synapses, and
    // - there is another, later spike, that is strictly more than
    //   (min_global_delay + max_local_delay + eps) away from the new spike (at t_sp_ms)
    while ( history_.size() > 1 )
    {
      const size_t next_t_sp = history_[ 1 ].t_;
      if ( history_.front().access_counter_ >= n_incoming_
        and t_sp_ms - next_t_sp > max_delay_ + kernel().connection_manager.get_min_delay() )
      {
        history_.pop_front();
      }
      else
      {
        break;
      }
    }
    // update spiking history
    Kminus_ = Kminus_ * std::exp( ( Time( Time::step( last_spike_ - t_sp_ms ) ).get_ms() ) * tau_minus_inv_ ) + 1.0;
    // TODO: save the triplet
    Kminus_triplet_ = Kminus_triplet_ * std::exp( ( last_spike_ - t_sp_ms ) * tau_minus_triplet_inv_ ) + 1.0;
    last_spike_ = t_sp_ms;
    history_.push_back( histentry( last_spike_, Kminus_, Kminus_triplet_, 0 ) );
  }
  else
  {
    last_spike_ = t_sp_ms;
  }
}

void
nest::ArchivingNode::get_status( DictionaryDatum& d ) const
{
  def< double >( d, names::t_spike, get_spiketime_ms() );
  def< double >( d, names::tau_minus, tau_minus_ );
  def< double >( d, names::tau_minus_triplet, tau_minus_triplet_ );
  def< double >( d, names::post_trace, trace_ );
#ifdef DEBUG_ARCHIVER
  def< int >( d, names::archiver_length, history_.size() );
#endif

  // add status dict items from the parent class
  StructuralPlasticityNode::get_status( d );
}

void
nest::ArchivingNode::set_status( const DictionaryDatum& d )
{
  // We need to preserve values in case invalid values are set
  double new_tau_minus = tau_minus_;
  double new_tau_minus_triplet = tau_minus_triplet_;
  updateValue< double >( d, names::tau_minus_triplet, new_tau_minus_triplet );

  if ( new_tau_minus <= 0.0 or new_tau_minus_triplet <= 0.0 )
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }

  StructuralPlasticityNode::set_status( d );

  // do the actual update
  tau_minus_ = new_tau_minus;
  tau_minus_triplet_ = new_tau_minus_triplet;
  tau_minus_inv_ = 1. / tau_minus_;
  tau_minus_triplet_inv_ = 1. / tau_minus_triplet_;

  // check, if to clear spike history and K_minus
  bool clear = false;
  updateValue< bool >( d, names::clear, clear );
  if ( clear )
  {
    clear_history();
  }
}

void
nest::ArchivingNode::clear_history()
{
  last_spike_ = -1.0;
  Kminus_ = 0.0;
  Kminus_triplet_ = 0.0;
  history_.clear();
}


} // of namespace nest
