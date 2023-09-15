/*
 *  eprop_archiving_node.cpp
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

// nestkernel
#include "eprop_archiving_node.h"
#include "kernel_manager.h"

// sli
#include "dictutils.h"

namespace nest
{

nest::EpropArchivingNode::EpropArchivingNode()
  : ArchivingNode()
  , gamma_( 0.3 )
{
}

nest::EpropArchivingNode::EpropArchivingNode( const EpropArchivingNode& n )
  : ArchivingNode( n )
  , gamma_( n.gamma_ )
{
}

void
nest::EpropArchivingNode::get_status( DictionaryDatum& d ) const
{
  ArchivingNode::get_status( d );

  def< double >( d, names::gamma, gamma_ );
}

void
nest::EpropArchivingNode::set_status( const DictionaryDatum& d )
{
  ArchivingNode::set_status( d );

  double new_gamma = gamma_; // preserve values for the case invalid values are set
  updateValue< double >( d, names::gamma, new_gamma );
  gamma_ = new_gamma;
}

void
nest::EpropArchivingNode::init_eprop_buffers( double delay )
{
  // register first entry for every synapse, increase access counter if entry already in list

  std::vector< histentry_extended >::iterator it_reg =
    std::lower_bound( t_last_update_per_synapse_.begin(), t_last_update_per_synapse_.end(), delay - eps_ );

  if ( it_reg == t_last_update_per_synapse_.end() || fabs( delay - it_reg->t_ ) > eps_ )
  {
    t_last_update_per_synapse_.insert( it_reg, histentry_extended( delay, 0.0, 1 ) );
  }
  else
  {
    ++it_reg->access_counter_;
  }
}

void
nest::EpropArchivingNode::get_eprop_history( double time_point, std::deque< histentry_eprop >::iterator* it )
{
  
  *it = std::lower_bound( eprop_history_.begin(), eprop_history_.end(), time_point - 1 + eps_ );
}

void
nest::EpropArchivingNode::register_update( double t_last_update, double t_current_update )
{
  // register spike time if it is not in the list, otherwise increase access counter
  std::vector< histentry_extended >::iterator it_reg =
    std::lower_bound( t_last_update_per_synapse_.begin(), t_last_update_per_synapse_.end(), t_current_update - eps_ );

  if ( it_reg == t_last_update_per_synapse_.end() || fabs( t_current_update - it_reg->t_ ) > eps_ )
  {
    t_last_update_per_synapse_.insert( it_reg, histentry_extended( t_current_update, 0.0, 1 ) );
  }
  else
  {
    ++it_reg->access_counter_;
  }
  // search for old entry, decrease access counter, and delete entry if the access counter equals zero
  it_reg =
    std::lower_bound( t_last_update_per_synapse_.begin(), t_last_update_per_synapse_.end(), t_last_update - eps_ );

  if ( it_reg == t_last_update_per_synapse_.end() || fabs( t_last_update - it_reg->t_ ) > eps_ )
  {
    std::cout << "found no entry to register in scanned " + std::to_string( t_last_update ) + " ms interval \n";
  }
  else
  {
    it_reg->access_counter_--;

    if ( it_reg->access_counter_ == 0 )
    {
      it_reg = t_last_update_per_synapse_.erase( it_reg ); // erase old entry
    }
  }
}


void
nest::EpropArchivingNode::get_spike_history( double t1,
  double t2,
  std::deque< double >::iterator* start,
  std::deque< double >::iterator* finish )
{
  // set pointer to spike history entries corresponding to times t1 and t2
  *finish = spike_history_.end();

  if ( spike_history_.empty() )
  {
    *start = *finish;
    return;
  }
  else
  {
    std::deque< double >::iterator runner1 =
      std::lower_bound( spike_history_.begin(), spike_history_.end(), t1 + eps_ );

    *start = runner1;

    std::deque< double >::iterator runner2 = std::lower_bound( runner1, spike_history_.end(), t2 + eps_ );

    *finish = runner2;
  }
}

void
nest::EpropArchivingNode::erase_unneeded_eprop_history()
{
  if ( eprop_history_.empty() )
    return;

  double update_interval = kernel().simulation_manager.get_eprop_update_interval();

  std::deque< histentry_eprop >::iterator start;
  std::deque< histentry_eprop >::iterator finish;

  auto it = t_last_update_per_synapse_.begin();

  for ( auto t = t_last_update_per_synapse_.begin()->t_; t <= ( t_last_update_per_synapse_.end() - 1 )->t_;
        t += update_interval )
  {
    if ( it->t_ == t )
    {
      it++;
    }
    else
    {
      get_eprop_history( t, &start );
      get_eprop_history( t + update_interval, &finish );
      eprop_history_.erase( start, finish ); // erase found entries since no longer used
    }
  }
  get_eprop_history( 0.0, &start );
  get_eprop_history( t_last_update_per_synapse_.begin()->t_, &finish );
  eprop_history_.erase( start, finish ); // erase found entries since no longer used
}

void
nest::EpropArchivingNode::erase_unneeded_spike_history()
{
  double earliest_time_to_keep = ( t_last_update_per_synapse_.begin() )->t_;
  while ( ( !spike_history_.empty() ) && ( spike_history_.front() + 1.0e-6 < earliest_time_to_keep ) )
  {
    spike_history_.pop_front();
  }
}

void
nest::EpropArchivingNode::write_v_m_pseudo_deriv_to_eprop_history( long time_step, double v_m_pseudo_deriv )
{
  if ( n_incoming_ )
  {
    const Time time_step_ = Time::step( time_step );
    const double time_ms = time_step_.get_ms();
    eprop_history_.push_back( histentry_eprop( time_ms, v_m_pseudo_deriv, 0.0 ) );
  }
}

void
nest::EpropArchivingNode::write_error_signal_to_eprop_history( long time_step, double error_signal )
{
  const Time time_step_ = Time::step( time_step );
  const double time_ms = time_step_.get_ms();
  eprop_history_.push_back( histentry_eprop( time_ms, 0.0, error_signal ) );
}

void
nest::EpropArchivingNode::write_spike_history( long time_step )
{
  const Time time_step_ = Time::step( time_step );
  const double time_ms = time_step_.get_ms();
  spike_history_.push_back( time_ms );
}

double
nest::EpropArchivingNode::get_learning_signal_from_eprop_history( long unsigned int shift )
{
  if ( eprop_history_.size() > shift )
  {
    return ( ( eprop_history_.rbegin() ) + shift )->learning_signal_;
  }
  else
  {
    return 0.0;
  }
}

void
nest::EpropArchivingNode::write_learning_signal_to_eprop_history( LearningSignalConnectionEvent& e )
{
  const double weight = e.get_weight();
  const long delay = e.get_delay_steps();
  const Time stamp = e.get_stamp();

  double t_ms = stamp.get_ms() - 2.0 * Time::get_resolution().get_ms();

  std::deque< histentry_eprop >::iterator start;
  std::deque< histentry_eprop >::iterator finish;

  // get part of eprop history to which the learning signal is added
  get_eprop_history( t_ms, &start );
  get_eprop_history( t_ms + Time::delay_steps_to_ms( delay ), &finish );

  std::vector< unsigned int >::iterator it = e.begin();

  if ( start != finish && it != e.end() )
  {
    double normalized_learning_signal = e.get_coeffvalue( it ); // implicitely decrease access counter
    start->learning_signal_ += weight * normalized_learning_signal;
    ++start;
  }
}

double
nest::EpropArchivingNode::calculate_v_m_pseudo_deriv( double V_m, double V_th, double V_th_const ) const
{
  // V_th: spiking threshold including adaptive part
  // V_th_const: constant part of threshold
  // in LIF neurons without adaptation: V_th = V_th_const
  double v_scaled = ( V_m - V_th ) / V_th_const;
  double norm_diff_threshold = 1.0 - std::fabs( v_scaled );
  double vm_pseudo_deriv = gamma_ * ( norm_diff_threshold > 0.0 ? norm_diff_threshold : 0.0 ) / V_th_const;
  return vm_pseudo_deriv;
}

} // namespace nest
