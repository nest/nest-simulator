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
  : Node()
{
}

nest::EpropArchivingNode::EpropArchivingNode( const EpropArchivingNode& n )
  : Node( n )
{
}

const long
nest::EpropArchivingNode::get_shift() const
{
  long shift_rec = offset_gen + delay_in_rec;
  long shift_out = offset_gen + delay_in_rec + delay_rec_out;

  return get_name() == "eprop_readout" ? shift_out : shift_rec;
}

void
nest::EpropArchivingNode::init_update_history()
{
  // register first entry for every synapse, increase access counter if entry already in list

  long shift = get_shift();

  std::vector< HistEntryEpropUpdate >::iterator it =
    std::lower_bound( update_history_.begin(), update_history_.end(), shift );

  if ( it == update_history_.end() or it->t_ != shift )
    update_history_.insert( it, HistEntryEpropUpdate( shift, 1 ) );
  else
    ++it->access_counter_;
}

void
nest::EpropArchivingNode::write_update_to_history( long t_last_update, long t_current_update )
{
  long shift = get_shift();

  std::vector< HistEntryEpropUpdate >::iterator it;

  it = std::lower_bound( update_history_.begin(), update_history_.end(), t_current_update + shift );

  if ( it != update_history_.end() or it->t_ == ( t_current_update + shift ) )
    ++it->access_counter_;
  else
    update_history_.insert( it, HistEntryEpropUpdate( t_current_update + shift, 1 ) );

  it = std::lower_bound( update_history_.begin(), update_history_.end(), t_last_update + shift );

  if ( it != update_history_.end() or it->t_ == ( t_last_update + shift ) )
    --it->access_counter_;
}

void
nest::EpropArchivingNode::get_eprop_history( long time_step, std::deque< HistEntryEpropArchive >::iterator* it )
{
  if ( eprop_history_.empty() )
  {
    *it = eprop_history_.end();
    return;
  }

  *it = std::lower_bound( eprop_history_.begin(), eprop_history_.end(), time_step );
}

void
nest::EpropArchivingNode::erase_unneeded_eprop_history()
{
  if ( eprop_history_.empty() or update_history_.empty() )
    return;

  long update_interval = kernel().simulation_manager.get_eprop_update_interval().get_steps();

  std::deque< HistEntryEpropArchive >::iterator start;
  std::deque< HistEntryEpropArchive >::iterator finish;

  auto it = update_history_.begin();

  for ( long t = update_history_.begin()->t_; t <= ( update_history_.end() - 1 )->t_; t += update_interval )
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
  get_eprop_history( 0, &start );
  get_eprop_history( update_history_.begin()->t_, &finish );
  eprop_history_.erase( start, finish ); // erase found entries since no longer used
}


void
nest::EpropArchivingNode::write_surrogate_gradient_to_history( long time_step, double surrogate_gradient )
{
  eprop_history_.push_back( HistEntryEpropArchive( time_step, surrogate_gradient, 0.0 ) );
}

void
nest::EpropArchivingNode::write_error_signal_to_history( long time_step, double error_signal )
{
  long shift = delay_out_norm;
  eprop_history_.push_back( HistEntryEpropArchive( time_step - shift, 0.0, error_signal ) );
}


void
nest::EpropArchivingNode::write_learning_signal_to_history( long& time_step,
  long& delay_out_rec,
  double& weight,
  double& error_signal )
{
  long shift = delay_rec_out + delay_out_norm + delay_out_rec;

  std::deque< HistEntryEpropArchive >::iterator it_hist;
  std::deque< HistEntryEpropArchive >::iterator it_hist_end;

  get_eprop_history( time_step - shift, &it_hist );
  get_eprop_history( time_step - shift + delay_out_rec, &it_hist_end );

  if ( it_hist != it_hist_end )
  {
    it_hist->learning_signal_ += weight * error_signal;
    ++it_hist;
  }
}

void
nest::EpropArchivingNode::erase_unneeded_update_history()
{
  if ( update_history_.empty() )
    return;

  for ( auto it = update_history_.begin(); it < update_history_.end(); it++ )
  {
    if ( it->access_counter_ == 0 )
      update_history_.erase( it );
  }
}

void
nest::EpropArchivingNode::erase_unneeded_firing_rate_reg_history()
{
  if ( update_history_.empty() or firing_rate_reg_history_.empty() )
    return;

  auto it_update_hist = update_history_.begin();
  auto it_reg_hist = firing_rate_reg_history_.begin();
  for ( ; it_update_hist != update_history_.end() and it_reg_hist != firing_rate_reg_history_.end();
        ++it_update_hist, ++it_reg_hist )
  {
    if ( it_update_hist->access_counter_ == 0 )
      firing_rate_reg_history_.erase( it_reg_hist );
  }
}

void
nest::EpropArchivingNode::add_spike_to_counter()
{
  n_spikes_++;
}

void
nest::EpropArchivingNode::reset_spike_counter()
{
  n_spikes_ = 0;
}

void
nest::EpropArchivingNode::write_firing_rate_reg_to_history( long t_current_update, double f_target, double c_reg )
{
  long const update_interval = kernel().simulation_manager.get_eprop_update_interval().get_steps();
  long shift = Time::get_resolution().get_steps();

  double f_av = n_spikes_ / static_cast< double >( update_interval );
  double f_target_ = f_target / 1000.0; // convert to kHz
  double firing_rate_reg = c_reg * ( f_av - f_target_ ) / static_cast< double >( update_interval );

  firing_rate_reg_history_.push_back( HistEntryEpropFiringRateReg( t_current_update + shift, firing_rate_reg ) );
}

double
nest::EpropArchivingNode::get_firing_rate_reg( long time_step )
{
  if ( firing_rate_reg_history_.empty() )
    return 0.0;

  long const update_interval = kernel().simulation_manager.get_eprop_update_interval().get_steps();

  std::vector< HistEntryEpropFiringRateReg >::iterator it;

  it =
    std::lower_bound( firing_rate_reg_history_.begin(), firing_rate_reg_history_.end(), time_step + update_interval );

  return it->firing_rate_reg_;
}

void
nest::EpropArchivingNode::write_eprop_parameter_to_map( std::string parameter_name, double parameter_value )
{
  eprop_parameter_map_[ parameter_name ] = parameter_value;
}

std::map< std::string, double >&
nest::EpropArchivingNode::get_eprop_parameter_map()
{
  return eprop_parameter_map_;
}

} // namespace nest
