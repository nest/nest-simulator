/*
 *  model.cpp
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

#include "model.h"

// C++ includes:
#include <algorithm>

// Includes from libnestutil:
#include "compose.hpp"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "model_manager.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{

Model::Model( const std::string& name )
  : name_( name )
  , type_id_( 0 )
  , memory_()
{
}

void
Model::set_threads()
{
  set_threads_( kernel::manager< VPManager >.get_num_threads() );
}

void
Model::set_threads_( size_t t )
{
  for ( size_t i = 0; i < memory_.size(); ++i )
  {
    if ( memory_[ i ].size() > 0 )
    {
      throw KernelException();
    }
  }

  memory_.resize( t );
  memory_.shrink_to_fit();
}

void
Model::reserve_additional( size_t t, size_t n )
{
  assert( t < memory_.size() );
  memory_[ t ].reserve( n );
}

void
Model::clear()
{
  memory_.clear();
  set_threads_( 1 );
}

size_t
Model::mem_available()
{
  size_t result = 0;
  for ( size_t t = 0; t < memory_.size(); ++t )
  {
    result += memory_[ t ].capacity() - memory_[ t ].size();
  }

  return result;
}

size_t
Model::mem_capacity()
{
  size_t result = 0;
  for ( size_t t = 0; t < memory_.size(); ++t )
  {
    result += memory_[ t ].capacity();
  }

  return result;
}

void
Model::set_status( DictionaryDatum d )
{
  try
  {
    set_status_( d );
  }
  catch ( BadProperty& e )
  {
    throw BadProperty( String::compose( "Setting status of model '%1': %2", get_name(), e.message() ) );
  }
}

DictionaryDatum
Model::get_status()
{
  DictionaryDatum d = get_status_();

  std::vector< long > tmp( memory_.size() );
  for ( size_t t = 0; t < tmp.size(); ++t )
  {
    tmp[ t ] = memory_[ t ].size();
  }

  ( *d )[ names::instantiations ] = Token( tmp );
  ( *d )[ names::type_id ] = LiteralDatum( kernel::manager< ModelManager >.get_node_model( type_id_ )->get_name() );

  for ( size_t t = 0; t < tmp.size(); ++t )
  {
    tmp[ t ] = memory_[ t ].capacity();
  }

  ( *d )[ names::capacity ] = Token( tmp );

  for ( size_t t = 0; t < tmp.size(); ++t )
  {
    tmp[ t ] = memory_[ t ].capacity() - memory_[ t ].size();
  }

  ( *d )[ names::available ] = Token( tmp );

  ( *d )[ names::model ] = LiteralDatum( get_name() );
  return d;
}

std::string
Model::get_name() const
{

  return name_;
}

Node*
Model::create( size_t t )
{

  assert( t < memory_.size() );
  Node* n = create_();
  memory_[ t ].emplace_back( n );
  return n;
}

size_t
Model::get_type_id() const
{

  return type_id_;
}

void
Model::set_type_id( size_t id )
{

  type_id_ = id;
}

} // namespace
