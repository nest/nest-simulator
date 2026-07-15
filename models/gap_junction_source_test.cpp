/*
 *  gap_junction_source_test.cpp
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

#include "gap_junction_source_test.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "nest_impl.h"

namespace nest
{

void
register_gap_junction_source_test( const std::string& name )
{
  register_node_model< gap_junction_source_test >( name );
}

gap_junction_source_test::gap_junction_source_test()
  : ArchivingNode()
  , source_values_( 1, 0.0 )
  , received_()
{
  Node::set_node_uses_wfr( kernel().simulation_manager.use_wfr() );
}

gap_junction_source_test::gap_junction_source_test( const gap_junction_source_test& n )
  : ArchivingNode( n )
  , source_values_( n.source_values_ )
  , received_()
{
  Node::set_node_uses_wfr( kernel().simulation_manager.use_wfr() );
}

void
gap_junction_source_test::init_buffers_()
{
  received_.assign( n_ports_(), 0.0 );
  ArchivingNode::clear_history();
}

void
gap_junction_source_test::pre_run_hook()
{
}

void
gap_junction_source_test::get_status( Dictionary& d ) const
{
  ArchivingNode::get_status( d );
  d[ "source_values" ] = source_values_;
  d[ "received" ] = received_;
}

void
gap_junction_source_test::set_status( const Dictionary& d )
{
  ArchivingNode::set_status( d );

  std::vector< double > source_values = source_values_;
  if ( d.update_value( "source_values", source_values ) )
  {
    if ( source_values.empty() )
    {
      throw BadProperty( "source_values must contain at least one entry." );
    }
    source_values_ = source_values;
  }
}

void
gap_junction_source_test::sends_secondary_event( GapJunctionEvent&, const size_t source_port )
{
  if ( source_port >= n_ports_() )
  {
    throw UnknownSourcePort( source_port );
  }
}

bool
gap_junction_source_test::update_( Time const&, const long from, const long to, const bool )
{
  const size_t interpolation_order = kernel().simulation_manager.get_wfr_interpolation_order();
  const size_t buffer_size = kernel().connection_manager.get_min_delay() * ( interpolation_order + 1 );

  // Emit one constant waveform per source port. Distinct ports carry distinct
  // values so routing can be verified on the receiving side.
  for ( size_t port = 0; port < n_ports_(); ++port )
  {
    std::vector< double > new_coefficients( buffer_size, 0.0 );
    for ( long lag = from; lag < to; ++lag )
    {
      new_coefficients[ lag * ( interpolation_order + 1 ) + 0 ] = source_values_[ port ];
    }

    GapJunctionEvent ge;
    ge.set_coeffarray( new_coefficients );
    kernel().event_delivery_manager.send_secondary( *this, ge, port );
  }

  return false;  // constant waveform: waveform relaxation is trivially converged
}

void
gap_junction_source_test::handle( GapJunctionEvent& e )
{
  const double weight = e.get_weight();
  const size_t rport = e.get_rport();
  assert( rport < received_.size() );

  // The constant coefficient (first entry of the coeffarray) carries the
  // scalar sample; record the delivered weight * value for this receptor.
  std::vector< unsigned int >::iterator it = e.begin();
  double constant = 0.0;
  if ( it != e.end() )
  {
    constant = e.get_coeffvalue( it );
  }
  received_[ rport ] = weight * constant;
}

}  // namespace nest
