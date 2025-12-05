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
#include <format>

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"


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
  set_threads_( kernel().vp_manager.get_num_threads() );
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
Model::set_status( const Dictionary& d )
{
  try
  {
    set_status_( d );
  }
  catch ( BadProperty& e )
  {
    throw BadProperty( std::format( "Setting status of model '{}': {}", get_name(), e.what() ) );
  }
}

Dictionary
Model::get_status()
{
  Dictionary d = get_status_();

  std::vector< long > tmp( memory_.size() );
  for ( size_t t = 0; t < tmp.size(); ++t )
  {
    tmp[ t ] = memory_[ t ].size();
  }

  d[ names::instantiations ] = tmp;
  d[ names::type_id ] = kernel().model_manager.get_node_model( type_id_ )->get_name();

  for ( size_t t = 0; t < tmp.size(); ++t )
  {
    tmp[ t ] = memory_[ t ].capacity();
  }

  d[ names::capacity ] = tmp;

  for ( size_t t = 0; t < tmp.size(); ++t )
  {
    tmp[ t ] = memory_[ t ].capacity() - memory_[ t ].size();
  }

  d[ names::available ] = tmp;

  d[ names::model ] = get_name();
  return d;
}

} // namespace
