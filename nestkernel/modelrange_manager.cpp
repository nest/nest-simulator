/*
 *  modelrange_manager.cpp
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

#include "modelrange_manager.h"

// C includes:
#include <assert.h>

// Includes from nestkernel:
#include "kernel_manager.h"
#include "model.h"


namespace nest
{

ModelRangeManager::ModelRangeManager()
  : modelranges_()
  , first_node_id_( 0 )
  , last_node_id_( 0 )
{
}

void
ModelRangeManager::initialize()
{
}

void
ModelRangeManager::finalize()
{
  modelranges_.clear();
  first_node_id_ = 0;
  last_node_id_ = 0;
}

void
ModelRangeManager::add_range( index model, index first_node_id, index last_node_id )
{
  if ( not modelranges_.empty() )
  {
    assert( first_node_id == last_node_id_ + 1 );
    if ( model == modelranges_.back().get_model_id() )
    {
      modelranges_.back().extend_range( last_node_id );
    }
    else
    {
      modelranges_.push_back( modelrange( model, first_node_id, last_node_id ) );
    }
  }
  else
  {
    modelranges_.push_back( modelrange( model, first_node_id, last_node_id ) );
    first_node_id_ = first_node_id;
  }

  last_node_id_ = last_node_id;
}

index
ModelRangeManager::get_model_id( index node_id ) const
{
  if ( not is_in_range( node_id ) )
  {
    throw UnknownNode( node_id );
  }

  int left = -1;
  int right = modelranges_.size();
  assert( right >= 1 );

  // to ensure thread-safety, use local range_idx
  size_t range_idx = right / 2; // start in center
  while ( not modelranges_[ range_idx ].is_in_range( node_id ) )
  {
    if ( node_id > modelranges_[ range_idx ].get_last_node_id() )
    {
      left = range_idx;
      range_idx += ( right - range_idx ) / 2;
    }
    else
    {
      right = range_idx;
      range_idx -= ( range_idx - left ) / 2;
    }
    assert( left + 1 < right );
    assert( range_idx < modelranges_.size() );
  }
  return modelranges_[ range_idx ].get_model_id();
}

nest::Model*
nest::ModelRangeManager::get_model_of_node_id( index node_id )
{
  return kernel().model_manager.get_model( get_model_id( node_id ) );
}

bool
ModelRangeManager::model_in_use( index i ) const
{
  bool found = false;

  for ( std::vector< modelrange >::const_iterator it = modelranges_.begin(); it != modelranges_.end(); ++it )
  {
    if ( it->get_model_id() == i )
    {
      found = true;
      break;
    }
  }

  return found;
}

const modelrange&
ModelRangeManager::get_contiguous_node_id_range( index node_id ) const
{
  if ( not is_in_range( node_id ) )
  {
    throw UnknownNode( node_id );
  }

  for ( std::vector< modelrange >::const_iterator it = modelranges_.begin(); it != modelranges_.end(); ++it )
  {
    if ( it->is_in_range( node_id ) )
    {
      return ( *it );
    }
  }

  throw UnknownNode( node_id );
}

} // namespace nest
