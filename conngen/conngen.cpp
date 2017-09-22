/*
 *  conngen.cpp
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

#include "conngen.h"

// Includes from conngen:
#include "cg_connect.h"

// Includes from libnestutil:
#include "logging.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "modelrange.h"
#include "subnet.h"

// Includes from sli:
#include "token.h"

void
nest::cg_connect( nest::ConnectionGeneratorDatum& cg,
  const index source_id,
  const index target_id,
  const DictionaryDatum& params_map,
  const Name& synmodel_name )
{
  Subnet* sources =
    dynamic_cast< Subnet* >( kernel().node_manager.get_node( source_id ) );
  if ( sources == NULL )
  {
    LOG( M_ERROR, "CGConnect_cg_i_i_D_l", "sources must be a subnet." );
    throw SubnetExpected();
  }
  if ( not sources->is_homogeneous() )
  {
    LOG( M_ERROR,
      "CGConnect_cg_i_i_D_l",
      "sources must be a homogeneous subnet." );
    throw BadProperty();
  }
  if ( dynamic_cast< Subnet* >( *sources->local_begin() ) )
  {
    LOG( M_ERROR,
      "CGConnect_cg_i_i_D_l",
      "Only 1-dim subnets are supported as sources." );
    throw BadProperty();
  }

  Subnet* targets =
    dynamic_cast< Subnet* >( kernel().node_manager.get_node( target_id ) );
  if ( targets == NULL )
  {
    LOG( M_ERROR, "CGConnect_cg_i_i_D_l", "targets must be a subnet." );
    throw SubnetExpected();
  }
  if ( not targets->is_homogeneous() )
  {
    LOG( M_ERROR,
      "CGConnect_cg_i_i_D_l",
      "targets must be a homogeneous subnet." );
    throw BadProperty();
  }
  if ( dynamic_cast< Subnet* >( *targets->local_begin() ) )
  {
    LOG( M_ERROR,
      "CGConnect_cg_i_i_D_l",
      "Only 1-dim subnets are supported as targets." );
    throw BadProperty();
  }

  const Token synmodel =
    kernel().model_manager.get_synapsedict()->lookup( synmodel_name );
  if ( synmodel.empty() )
  {
    throw UnknownSynapseType( synmodel_name.toString() );
  }
  const index synmodel_id = static_cast< index >( synmodel );

  const modelrange source_range =
    kernel().modelrange_manager.get_contiguous_gid_range(
      ( *sources->local_begin() )->get_gid() );
  index source_offset = source_range.get_first_gid();
  RangeSet source_ranges;
  source_ranges.push_back(
    Range( source_range.get_first_gid(), source_range.get_last_gid() ) );

  const modelrange target_range =
    kernel().modelrange_manager.get_contiguous_gid_range(
      ( *targets->local_begin() )->get_gid() );
  index target_offset = target_range.get_first_gid();
  RangeSet target_ranges;
  target_ranges.push_back(
    Range( target_range.get_first_gid(), target_range.get_last_gid() ) );

  cg_connect( cg,
    source_ranges,
    source_offset,
    target_ranges,
    target_offset,
    params_map,
    synmodel_id );
}

void
nest::cg_connect( nest::ConnectionGeneratorDatum& cg,
  IntVectorDatum& sources,
  IntVectorDatum& targets,
  const DictionaryDatum& params_map,
  const Name& synmodel_name )
{
  const Token synmodel =
    kernel().model_manager.get_synapsedict()->lookup( synmodel_name );
  if ( synmodel.empty() )
  {
    throw UnknownSynapseType( synmodel_name.toString() );
  }
  const index synmodel_id = static_cast< index >( synmodel );

  RangeSet source_ranges;
  cg_get_ranges( source_ranges, ( *sources ) );

  RangeSet target_ranges;
  cg_get_ranges( target_ranges, ( *targets ) );

  cg_connect( cg,
    source_ranges,
    ( *sources ),
    target_ranges,
    ( *targets ),
    params_map,
    synmodel_id );
}

nest::ConnectionGeneratorDatum
nest::cg_parse( const StringDatum& xml )
{
  return ConnectionGenerator::fromXML( xml );
}

nest::ConnectionGeneratorDatum
nest::cg_parse_file( const StringDatum& xml )
{
  return ConnectionGenerator::fromXMLFile( xml );
}

void
nest::cg_select_implementation( const StringDatum& library,
  const StringDatum& tag )
{
  ConnectionGenerator::selectCGImplementation( tag, library );
}

void
nest::cg_set_masks( nest::ConnectionGeneratorDatum& cg,
  IntVectorDatum& sources,
  IntVectorDatum& targets )
{
  RangeSet source_ranges;
  cg_get_ranges( source_ranges, ( *sources ) );

  RangeSet target_ranges;
  cg_get_ranges( target_ranges, ( *targets ) );

  cg_set_masks( cg, source_ranges, target_ranges );
}

void
nest::cg_start( nest::ConnectionGeneratorDatum& cgd )
{
  cgd->start();
}

bool
nest::cg_next( nest::ConnectionGeneratorDatum& cgd,
  int& src,
  int& tgt,
  std::vector< double >& values )
{
  ConnectionGenerator* generator = cgd.get();

  int arity = generator->arity();
  double* tmp_values = new double[ arity ];

  values.resize( arity );

  if ( generator->next( src, tgt, tmp_values ) )
  {
    for ( int m = 0; m < arity; ++m )
    {
      values[ m ] = tmp_values[ m ];
    }
    delete[] tmp_values;
    cgd.unlock();
    return true;
  }
  else
  {
    cgd.unlock();
    return false;
  }
}
