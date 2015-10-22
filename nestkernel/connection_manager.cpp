/*
 *  connection_manager.cpp
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

#include "connection_manager.h"
#include "connector_base.h"
#include "network.h"
#include "spikecounter.h"
#include "nest_time.h"
#include "nest_datums.h"
#include "kernel_manager.h"

#include <algorithm>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace nest
{

ConnectionManager::ConnectionManager()
{
}

ConnectionManager::~ConnectionManager()
{

  clear_prototypes_();

  for ( std::vector< ConnectorModel* >::iterator i = pristine_prototypes_.begin();
        i != pristine_prototypes_.end();
        ++i )
    if ( *i != 0 )
      delete *i;
}

void
ConnectionManager::init( Dictionary* synapsedict )
{
  synapsedict_ = synapsedict;
  init_();
}

void
ConnectionManager::init_()
{
  synapsedict_->clear();

  // one list of prototypes per thread
  std::vector< std::vector< ConnectorModel* > > tmp_proto( kernel().vp_manager.get_num_threads() );
  prototypes_.swap( tmp_proto );

  // (re-)append all synapse prototypes
  for ( std::vector< ConnectorModel* >::iterator i = pristine_prototypes_.begin();
        i != pristine_prototypes_.end();
        ++i )
    if ( *i != 0 )
    {
      std::string name = ( *i )->get_name();
      for ( index t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
        prototypes_[ t ].push_back( ( *i )->clone( name ) );
      synapsedict_->insert( name, prototypes_[ 0 ].size() - 1 );
    }
}


void
ConnectionManager::clear_prototypes_()
{
  for ( std::vector< std::vector< ConnectorModel* > >::iterator it = prototypes_.begin();
        it != prototypes_.end();
        ++it )
  {
    for ( std::vector< ConnectorModel* >::iterator pt = it->begin(); pt != it->end(); ++pt )
      if ( *pt != 0 )
        delete *pt;
    it->clear();
  }
  prototypes_.clear();
}

void
ConnectionManager::reset()
{
  clear_prototypes_();
  init_();
}


synindex
ConnectionManager::register_synapse_prototype( ConnectorModel* cf )
{
  std::string name = cf->get_name();

  if ( synapsedict_->known( name ) )
  {
    delete cf;
    throw NamingConflict("A synapse type called '" + name + "' already exists.\n"
                         "Please choose a different name!");
  }

  pristine_prototypes_.push_back( cf );

  const synindex id = prototypes_[ 0 ].size();
  pristine_prototypes_[ id ]->set_syn_id( id );

  for ( index t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
  {
    prototypes_[ t ].push_back( cf->clone( name ) );
    prototypes_[ t ][ id ]->set_syn_id( id );
  }

  synapsedict_->insert( name, id );

  return id;
}


synindex
ConnectionManager::copy_synapse_prototype( synindex old_id, std::string new_name )
{
  // we can assert here, as nestmodule checks this for us
  assert( !synapsedict_->known( new_name ) );

  int new_id = prototypes_[ 0 ].size();

  if ( new_id == invalid_synindex ) // we wrapped around (=255), maximal id of synapse_model = 254
  {
    LOG( M_ERROR,
      "ConnectionManager::copy_synapse_prototype",
      "CopyModel cannot generate another synapse. Maximal synapse model count of 255 exceeded." );
    throw KernelException( "Synapse model count exceeded" );
  }
  assert( new_id != invalid_synindex );

  for ( index t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
  {
    prototypes_[ t ].push_back( get_synapse_prototype( old_id ).clone( new_name ) );
    prototypes_[ t ][ new_id ]->set_syn_id( new_id );
  }

  synapsedict_->insert( new_name, new_id );
  return new_id;
}


void
ConnectionManager::get_status( DictionaryDatum& ) const
{
}

void
ConnectionManager::set_prototype_status( synindex syn_id, const DictionaryDatum& d )
{
  assert_valid_syn_id( syn_id );
  for ( index t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
  {
    try
    {
      prototypes_[ t ][ syn_id ]->set_status( d );
    }
    catch ( BadProperty& e )
    {
      throw BadProperty( String::compose( "Setting status of prototype '%1': %2",
        prototypes_[ t ][ syn_id ]->get_name(),
        e.message() ) );
    }
  }
}

DictionaryDatum
ConnectionManager::get_prototype_status( synindex syn_id ) const
{
  assert_valid_syn_id( syn_id );

  DictionaryDatum dict( new Dictionary );

  for ( index t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
    prototypes_[ t ][ syn_id ]->get_status( dict );

  ( *dict )[ "num_connections" ] =
    kernel().connection_builder_manager.get_num_connections( syn_id );

  return dict;
}


} // namespace
