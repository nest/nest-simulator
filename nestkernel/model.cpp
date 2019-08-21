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
  set_threads_( kernel().vp_manager.get_num_threads() );
}

void
Model::set_threads_( thread t )
{
  for ( size_t i = 0; i < memory_.size(); ++i )
  {
    if ( memory_[ i ].get_instantiations() > 0 )
    {
      throw KernelException();
    }
  }

  std::vector< sli::pool > tmp( t );
  memory_.swap( tmp );

  for ( size_t i = 0; i < memory_.size(); ++i )
  {
    init_memory_( memory_[ i ] );
  }
}

void
Model::reserve_additional( thread t, size_t s )
{
  assert( ( size_t ) t < memory_.size() );
  memory_[ t ].reserve_additional( s );
}

void
Model::clear()
{
  std::vector< sli::pool > mem;
  memory_.swap( mem );
  set_threads_( 1 );
}

size_t
Model::mem_available()
{
  size_t result = 0;
  for ( size_t t = 0; t < memory_.size(); ++t )
  {
    result += memory_[ t ].available();
  }

  return result;
}

size_t
Model::mem_capacity()
{
  size_t result = 0;
  for ( size_t t = 0; t < memory_.size(); ++t )
  {
    result += memory_[ t ].get_total();
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
Model::get_status( void )
{
  DictionaryDatum d = get_status_();

  std::vector< long > tmp( memory_.size() );
  for ( size_t t = 0; t < tmp.size(); ++t )
  {
    tmp[ t ] = memory_[ t ].get_instantiations();
  }

  ( *d )[ names::instantiations ] = Token( tmp );
  ( *d )[ names::type_id ] = LiteralDatum( kernel().model_manager.get_model( type_id_ )->get_name() );

  for ( size_t t = 0; t < tmp.size(); ++t )
  {
    tmp[ t ] = memory_[ t ].get_total();
  }

  ( *d )[ names::capacity ] = Token( tmp );

  for ( size_t t = 0; t < tmp.size(); ++t )
  {
    tmp[ t ] = memory_[ t ].available();
  }

  ( *d )[ names::available ] = Token( tmp );

  ( *d )[ names::model ] = LiteralDatum( get_name() );
  return d;
}

} // namespace
