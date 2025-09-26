/*
 *  eprop_archiving_node_recurrent_impl.h
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

#ifndef EPROP_ARCHIVING_NODE_RECURRENT_IMPL_H
#define EPROP_ARCHIVING_NODE_RECURRENT_IMPL_H

#include "eprop_archiving_node_recurrent.h"

namespace nest
{


template < bool hist_shift_required >
inline void
EpropArchivingNodeRecurrent< hist_shift_required >::count_spike()
{
  ++n_spikes_;
}

template < bool hist_shift_required >
inline void
EpropArchivingNodeRecurrent< hist_shift_required >::reset_spike_count()
{
  n_spikes_ = 0;
}

template < bool hist_shift_required >
long
EpropArchivingNodeRecurrent< hist_shift_required >::model_dependent_history_shift_() const
{
  if constexpr ( hist_shift_required )
  {
    return get_shift();
  }
  else
  {
    return -delay_rec_out_;
  }
}

template < bool hist_shift_required >
bool
EpropArchivingNodeRecurrent< hist_shift_required >::history_shift_required_() const
{
  return hist_shift_required;
}

template < bool hist_shift_required >
std::map< std::string, typename EpropArchivingNodeRecurrent< hist_shift_required >::surrogate_gradient_function >
  EpropArchivingNodeRecurrent< hist_shift_required >::surrogate_gradient_funcs_ = {
    { "piecewise_linear",
      &EpropArchivingNodeRecurrent< hist_shift_required >::compute_piecewise_linear_surrogate_gradient },
    { "exponential", &EpropArchivingNodeRecurrent< hist_shift_required >::compute_exponential_surrogate_gradient },
    { "fast_sigmoid_derivative",
      &EpropArchivingNodeRecurrent< hist_shift_required >::compute_fast_sigmoid_derivative_surrogate_gradient },
    { "arctan", &EpropArchivingNodeRecurrent< hist_shift_required >::compute_arctan_surrogate_gradient }
  };

template < bool hist_shift_required >
EpropArchivingNodeRecurrent< hist_shift_required >::EpropArchivingNodeRecurrent()
  : EpropArchivingNode()
  , firing_rate_reg_( 0.0 )
  , f_av_( 0.0 )
  , n_spikes_( 0 )
{
}

template < bool hist_shift_required >
EpropArchivingNodeRecurrent< hist_shift_required >::EpropArchivingNodeRecurrent( const EpropArchivingNodeRecurrent& n )
  : EpropArchivingNode( n )
  , firing_rate_reg_( n.firing_rate_reg_ )
  , f_av_( n.f_av_ )
  , n_spikes_( n.n_spikes_ )
{
}

template < bool hist_shift_required >
typename EpropArchivingNodeRecurrent< hist_shift_required >::surrogate_gradient_function
EpropArchivingNodeRecurrent< hist_shift_required >::find_surrogate_gradient(
  const std::string& surrogate_gradient_function_name )
{
  const auto found_entry_it = surrogate_gradient_funcs_.find( surrogate_gradient_function_name );

  if ( found_entry_it != surrogate_gradient_funcs_.end() )
  {
    return found_entry_it->second;
  }

  std::string error_message = "Surrogate gradient / pseudo-derivate function surrogate_gradient_function from [";
  for ( const auto& surrogate_gradient_func : surrogate_gradient_funcs_ )
  {
    error_message += " \"" + surrogate_gradient_func.first + "\",";
  }
  error_message.pop_back();
  error_message += " ] required.";

  throw BadProperty( error_message );
}

template < bool hist_shift_required >
double
EpropArchivingNodeRecurrent< hist_shift_required >::compute_piecewise_linear_surrogate_gradient( const double r,
  const double v_m,
  const double v_th,
  const double beta,
  const double gamma )
{
  if ( r > 0 )
  {
    return 0.0;
  }

  return gamma * std::max( 0.0, 1.0 - beta * std::abs( v_m - v_th ) );
}

template < bool hist_shift_required >
double
EpropArchivingNodeRecurrent< hist_shift_required >::compute_exponential_surrogate_gradient( const double r,
  const double v_m,
  const double v_th,
  const double beta,
  const double gamma )
{
  if ( r > 0 )
  {
    return 0.0;
  }

  return gamma * std::exp( -beta * std::abs( v_m - v_th ) );
}

template < bool hist_shift_required >
double
EpropArchivingNodeRecurrent< hist_shift_required >::compute_fast_sigmoid_derivative_surrogate_gradient( const double r,
  const double v_m,
  const double v_th,
  const double beta,
  const double gamma )
{
  if ( r > 0 )
  {
    return 0.0;
  }

  return gamma * std::pow( 1.0 + beta * std::abs( v_m - v_th ), -2 );
}

template < bool hist_shift_required >
double
EpropArchivingNodeRecurrent< hist_shift_required >::compute_arctan_surrogate_gradient( const double r,
  const double v_m,
  const double v_th,
  const double beta,
  const double gamma )
{
  if ( r > 0 )
  {
    return 0.0;
  }

  return gamma / M_PI * ( 1.0 / ( 1.0 + std::pow( beta * M_PI * ( v_m - v_th ), 2 ) ) );
}

template < bool hist_shift_required >
void
EpropArchivingNodeRecurrent< hist_shift_required >::append_new_eprop_history_entry( const long time_step )
{
  if ( eprop_indegree_ == 0 )
  {
    return;
  }

  eprop_history_.emplace_back( time_step, 0.0, 0.0, 0.0 );
}

template < bool hist_shift_required >
void
EpropArchivingNodeRecurrent< hist_shift_required >::write_surrogate_gradient_to_history( const long time_step,
  const double surrogate_gradient )
{
  if ( eprop_indegree_ == 0 )
  {
    return;
  }

  auto it_hist = get_eprop_history( time_step );
  it_hist->surrogate_gradient_ = surrogate_gradient;
}

template < bool hist_shift_required >
void
EpropArchivingNodeRecurrent< hist_shift_required >::write_learning_signal_to_history( const long time_step,
  const double learning_signal )
{
  if ( eprop_indegree_ == 0 )
  {
    return;
  }

  long shift = delay_rec_out_ + delay_out_rec_;

  if constexpr ( hist_shift_required )
  {
    shift += delay_out_norm_;
  }


  auto it_hist = get_eprop_history( time_step - shift );
  const auto it_hist_end = get_eprop_history( time_step - shift + delay_out_rec_ );

  for ( ; it_hist != it_hist_end; ++it_hist )
  {
    it_hist->learning_signal_ += learning_signal;
  }
}

template < bool hist_shift_required >
void
EpropArchivingNodeRecurrent< hist_shift_required >::write_firing_rate_reg_to_history( const long t_current_update,
  const double f_target,
  const double c_reg )
{
  if ( eprop_indegree_ == 0 )
  {
    return;
  }

  const double update_interval = kernel::manager< SimulationManager >.get_eprop_update_interval().get_steps();
  const double dt = Time::get_resolution().get_ms();
  const long shift = Time::get_resolution().get_steps();

  const double f_av = n_spikes_ / update_interval;
  const double f_target_ = f_target * dt; // convert from spikes/ms to spikes/step
  const double firing_rate_reg = c_reg * ( f_av - f_target_ ) / update_interval;

  firing_rate_reg_history_.emplace_back( t_current_update + shift, firing_rate_reg );
}

template < bool hist_shift_required >
void
EpropArchivingNodeRecurrent< hist_shift_required >::write_firing_rate_reg_to_history( const long time_step,
  const double z,
  const double f_target,
  const double kappa_reg,
  const double c_reg )
{
  if ( eprop_indegree_ == 0 )
  {
    return;
  }

  const double dt = Time::get_resolution().get_ms();

  const double f_target_ = f_target * dt; // convert from spikes/ms to spikes/step

  f_av_ = kappa_reg * f_av_ + ( 1.0 - kappa_reg ) * z / dt;

  firing_rate_reg_ = c_reg * ( f_av_ - f_target_ );

  auto it_hist = get_eprop_history( time_step );
  it_hist->firing_rate_reg_ = firing_rate_reg_;
}

template < bool hist_shift_required >
double
EpropArchivingNodeRecurrent< hist_shift_required >::get_firing_rate_reg_history( const long time_step )
{
  const auto it_hist = std::lower_bound( firing_rate_reg_history_.begin(), firing_rate_reg_history_.end(), time_step );
  assert( it_hist != firing_rate_reg_history_.end() );

  return it_hist->firing_rate_reg_;
}

template < bool hist_shift_required >
double
EpropArchivingNodeRecurrent< hist_shift_required >::get_learning_signal_from_history( const long time_step )
{
  long shift = delay_rec_out_ + delay_out_rec_;

  if ( hist_shift_required )
  {
    shift += delay_out_norm_;
  }

  const auto it = get_eprop_history( time_step - shift );
  if ( it == eprop_history_.end() )
  {
    return 0;
  }

  return it->learning_signal_;
}

template < bool hist_shift_required >
void
EpropArchivingNodeRecurrent< hist_shift_required >::erase_used_firing_rate_reg_history()
{
  auto it_update_hist = update_history_.begin();
  auto it_reg_hist = firing_rate_reg_history_.begin();

  while ( it_update_hist != update_history_.end() and it_reg_hist != firing_rate_reg_history_.end() )
  {
    if ( it_update_hist->access_counter_ == 0 )
    {
      it_reg_hist = firing_rate_reg_history_.erase( it_reg_hist );
    }
    else
    {
      ++it_reg_hist;
    }
    ++it_update_hist;
  }
}


}


#endif
