/*
 *  urbanczik_archiving_node_impl.h
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

#ifndef URBANCZIK_ARCHIVING_NODE_IMPL_H
#define URBANCZIK_ARCHIVING_NODE_IMPL_H

#include "urbanczik_archiving_node.h"

namespace nest
{

template < class urbanczik_parameters >
inline double
UrbanczikArchivingNode< urbanczik_parameters >::get_C_m( int comp )
{
  return urbanczik_params->C_m[ comp ];
}

template < class urbanczik_parameters >
inline double
UrbanczikArchivingNode< urbanczik_parameters >::get_g_L( int comp )
{
  return urbanczik_params->g_L[ comp ];
}

template < class urbanczik_parameters >
inline double
UrbanczikArchivingNode< urbanczik_parameters >::get_tau_L( int comp )
{
  return urbanczik_params->C_m[ comp ] / urbanczik_params->g_L[ comp ];
}

template < class urbanczik_parameters >
inline double
UrbanczikArchivingNode< urbanczik_parameters >::get_tau_syn_ex( int comp )
{
  return urbanczik_params->tau_syn_ex[ comp ];
}

template < class urbanczik_parameters >
inline double
UrbanczikArchivingNode< urbanczik_parameters >::get_tau_syn_in( int comp )
{
  return urbanczik_params->tau_syn_in[ comp ];
}

template < class urbanczik_parameters >
UrbanczikArchivingNode< urbanczik_parameters >::UrbanczikArchivingNode()
  : ArchivingNode()
{
}

template < class urbanczik_parameters >
UrbanczikArchivingNode< urbanczik_parameters >::UrbanczikArchivingNode( const UrbanczikArchivingNode& n )
  : ArchivingNode( n )
{
}

template < class urbanczik_parameters >
void
UrbanczikArchivingNode< urbanczik_parameters >::get_status( DictionaryDatum& d ) const
{
  ArchivingNode::get_status( d );
}

template < class urbanczik_parameters >
void
UrbanczikArchivingNode< urbanczik_parameters >::set_status( const DictionaryDatum& d )
{
  ArchivingNode::set_status( d );
}

template < class urbanczik_parameters >
void
UrbanczikArchivingNode< urbanczik_parameters >::get_urbanczik_history( double t1,
  double t2,
  std::deque< histentry_extended >::iterator* start,
  std::deque< histentry_extended >::iterator* finish,
  int comp )
{
  *finish = urbanczik_history_[ comp - 1 ].end();
  if ( urbanczik_history_[ comp - 1 ].empty() )
  {
    *start = *finish;
    return;
  }
  else
  {
    std::deque< histentry_extended >::iterator runner = urbanczik_history_[ comp - 1 ].begin();
    // To have a well defined discretization of the integral, we make sure
    // that we exclude the entry at t1 but include the one at t2 by subtracting
    // a small number so that runner->t_ is never equal to t1 or t2.
    while ( ( runner != urbanczik_history_[ comp - 1 ].end() ) and runner->t_ - 1.0e-6 < t1 )
    {
      ++runner;
    }
    *start = runner;
    while ( ( runner != urbanczik_history_[ comp - 1 ].end() ) and runner->t_ - 1.0e-6 < t2 )
    {
      ( runner->access_counter_ )++;
      ++runner;
    }
    *finish = runner;
  }
}

template < class urbanczik_parameters >
void
UrbanczikArchivingNode< urbanczik_parameters >::write_urbanczik_history( Time const& t_sp,
  double V_W,
  int n_spikes,
  int comp )
{
  const double t_ms = t_sp.get_ms();

  const double g_D = urbanczik_params->g_conn[ urbanczik_parameters::SOMA ];
  const double g_L = urbanczik_params->g_L[ urbanczik_parameters::SOMA ];
  const double E_L = urbanczik_params->E_L[ urbanczik_parameters::SOMA ];
  const double V_W_star = ( ( E_L * g_L + V_W * g_D ) / ( g_D + g_L ) );

  if ( n_incoming_ )
  {
    // prune all entries from history which are no longer needed
    // except the penultimate one. we might still need it.
    while ( urbanczik_history_[ comp - 1 ].size() > 1 )
    {
      if ( urbanczik_history_[ comp - 1 ].front().access_counter_ >= n_incoming_ )
      {
        urbanczik_history_[ comp - 1 ].pop_front();
      }
      else
      {
        break;
      }
    }

    double dPI = ( n_spikes - urbanczik_params->phi( V_W_star ) * Time::get_resolution().get_ms() )
      * urbanczik_params->h( V_W_star );
    urbanczik_history_[ comp - 1 ].push_back( histentry_extended( t_ms, dPI, 0 ) );
  }
}

}


#endif
