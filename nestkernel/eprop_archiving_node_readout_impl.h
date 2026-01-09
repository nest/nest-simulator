/*
 *  eprop_archiving_node_readout_impl.h
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

#ifndef EPROP_ARCHIVING_NODE_READOUT_IMPL_H
#define EPROP_ARCHIVING_NODE_READOUT_IMPL_H

// models
#include "eprop_archiving_node_readout.h"

namespace nest
{

template < bool hist_shift_required >
void
EpropArchivingNodeReadout< hist_shift_required >::append_new_eprop_history_entry( long time_step )
{
  if ( eprop_indegree_ == 0 )
  {
    return;
  }

  if constexpr ( hist_shift_required )
  {
    time_step -= delay_out_norm_;
  }

  eprop_history_.emplace_back( time_step, 0.0 );
}

template < bool hist_shift_required >
void
EpropArchivingNodeReadout< hist_shift_required >::write_error_signal_to_history( long time_step,
  const double error_signal )
{
  if ( eprop_indegree_ == 0 )
  {
    return;
  }

  if constexpr ( hist_shift_required )
  {
    time_step -= delay_out_norm_;
  }

  auto it_hist = get_eprop_history( time_step );
  it_hist->error_signal_ = error_signal;
}

template < bool hist_shift_required >
long
EpropArchivingNodeReadout< hist_shift_required >::model_dependent_history_shift_() const
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
EpropArchivingNodeReadout< hist_shift_required >::history_shift_required_() const
{
  return hist_shift_required;
}


}

#endif
