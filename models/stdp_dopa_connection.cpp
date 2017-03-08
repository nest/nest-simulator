/*
 *  stdp_dopa_connection.cpp
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

#include "stdp_dopa_connection.h"

// Includes from nestkernel:
#include "common_synapse_properties.h"
#include "connector_model.h"
#include "event.h"
#include "kernel_manager.h"

// Includes from sli:
#include "dictdatum.h"

namespace nest
{
//
// Implementation of class STDPDopaCommonProperties.
//

STDPDopaCommonProperties::STDPDopaCommonProperties()
  : CommonSynapseProperties()
  , vt_( 0 )
  , A_plus_( 1.0 )
  , A_minus_( 1.5 )
  , tau_plus_( 20.0 )
  , tau_c_( 1000.0 )
  , tau_n_( 200.0 )
  , b_( 0.0 )
  , Wmin_( 0.0 )
  , Wmax_( 200.0 )
{
}

void
STDPDopaCommonProperties::get_status( DictionaryDatum& d ) const
{
  CommonSynapseProperties::get_status( d );
  if ( vt_ != 0 )
  {
    def< long >( d, "vt", vt_->get_gid() );
  }
  else
  {
    def< long >( d, "vt", -1 );
  }

  def< double >( d, "A_plus", A_plus_ );
  def< double >( d, "A_minus", A_minus_ );
  def< double >( d, "tau_plus", tau_plus_ );
  def< double >( d, "tau_c", tau_c_ );
  def< double >( d, "tau_n", tau_n_ );
  def< double >( d, "b", b_ );
  def< double >( d, "Wmin", Wmin_ );
  def< double >( d, "Wmax", Wmax_ );
}

void
STDPDopaCommonProperties::set_status( const DictionaryDatum& d,
  ConnectorModel& cm )
{
  CommonSynapseProperties::set_status( d, cm );

  long vtgid;
  if ( updateValue< long >( d, "vt", vtgid ) )
  {
    vt_ = dynamic_cast< volume_transmitter* >( kernel().node_manager.get_node(
      vtgid, kernel().vp_manager.get_thread_id() ) );
    if ( vt_ == 0 )
    {
      throw BadProperty( "Dopamine source must be volume transmitter" );
    }
  }

  updateValue< double >( d, "A_plus", A_plus_ );
  updateValue< double >( d, "A_minus", A_minus_ );
  updateValue< double >( d, "tau_plus", tau_plus_ );
  updateValue< double >( d, "tau_c", tau_c_ );
  updateValue< double >( d, "tau_n", tau_n_ );
  updateValue< double >( d, "b", b_ );
  updateValue< double >( d, "Wmin", Wmin_ );
  updateValue< double >( d, "Wmax", Wmax_ );
}

Node*
STDPDopaCommonProperties::get_node()
{
  if ( vt_ == 0 )
  {
    throw BadProperty(
      "No volume transmitter has been assigned to the dopamine synapse." );
  }
  else
  {
    return vt_;
  }
}

} // of namespace nest
