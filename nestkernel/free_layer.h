/*
 *  free_layer.h
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

#ifndef FREE_LAYER_H
#define FREE_LAYER_H

// C++ includes:
#include <algorithm>
#include <limits>
#include <sstream>

// Includes from nestkernel:
#include "nest_names.h"

// Includes from sli:
#include "dictutils.h"

// Includes from spatial:
#include "layer.h"
#include "ntree_impl.h"

namespace nest
{

/**
 * Layer with free positioning of neurons, positions specified by user.
 */
template < int D >
class FreeLayer : public Layer< D >
{
public:
  Position< D > get_position( index sind ) const;
  void set_status( const DictionaryDatum& );
  void get_status( DictionaryDatum& ) const;

protected:
  /**
   * Communicate positions across MPI processes
   * @param iter Insert iterator which will receive pairs of Position,node ID
   * @param node_collection NodeCollection of the layer
   */
  template < class Ins >
  void communicate_positions_( Ins iter, NodeCollectionPTR node_collection );

  void insert_global_positions_ntree_( Ntree< D, index >& tree, NodeCollectionPTR node_collection );
  void insert_global_positions_vector_( std::vector< std::pair< Position< D >, index > >& vec,
    NodeCollectionPTR node_collection );

  /// Vector of positions.
  std::vector< Position< D > > positions_;

  /// This class is used when communicating positions across MPI procs.
  class NodePositionData
  {
  public:
    index
    get_node_id() const
    {
      return node_id_;
    }
    Position< D >
    get_position() const
    {
      return Position< D >( pos_ );
    }
    bool operator<( const NodePositionData& other ) const
    {
      return node_id_ < other.node_id_;
    }
    bool operator==( const NodePositionData& other ) const
    {
      return node_id_ == other.node_id_;
    }

  private:
    double node_id_;
    double pos_[ D ];
  };
};

template < int D >
void
FreeLayer< D >::set_status( const DictionaryDatum& d )
{
  Layer< D >::set_status( d );

  Position< D > max_point; // for each dimension, the largest value of the positions, aka upper right
  Position< D > epsilon;   // small value which may be added to layer size ensuring that single-point layers have a size

  for ( int d = 0; d < D; ++d )
  {
    this->lower_left_[ d ] = std::numeric_limits< double >::infinity();
    max_point[ d ] = -std::numeric_limits< double >::infinity();
    epsilon[ d ] = 0.1;
  }

  // Read positions from dictionary
  if ( d->known( names::positions ) )
  {
    const Token& tkn = d->lookup( names::positions );
    if ( tkn.is_a< TokenArray >() )
    {
      size_t step = 1;
      if ( d->known( names::step ) )
      {
        step = getValue< long >( d->lookup( names::step ) );
      }
      assert( step == 1 );
      TokenArray pos = getValue< TokenArray >( tkn );

      // Assuming step==1
      const auto num_local_nodes = this->node_collection_->end() - this->node_collection_->MPI_local_begin();
      const auto num_devices = this->node_collection_->num_devices();
      // A NodeCollection with spatial information cannot be a composite, so it contains either only devices or
      // only regular nodes. Regular nodes are distributed over MPI processes, while devices are present on every
      // process, and therefore requires positions on all processes.
      const auto expected_num_positions = num_devices > 0 ? num_devices : num_local_nodes;

      positions_.clear();
      positions_.reserve( expected_num_positions );

      auto nc_it = this->node_collection_->begin();
      for ( Token* it = pos.begin(); it != pos.end(); ++it, ++nc_it )
      {
        assert( nc_it != this->node_collection_->end() );
        Position< D > point = getValue< std::vector< double > >( *it );
        const auto node = kernel().node_manager.get_mpi_local_node_or_device_head( ( *nc_it ).node_id );
        assert( node );
        if ( not node->is_proxy() )
        {
          positions_.push_back( point );
        }
        // We do the calculation for lower_left_ and max_point even if we don't add the position, to keep the size of
        // the layer consistent over processes.
        for ( int d = 0; d < D; ++d )
        {
          if ( point[ d ] < this->lower_left_[ d ] )
          {
            this->lower_left_[ d ] = point[ d ];
          }
          if ( point[ d ] > max_point[ d ] )
          {
            max_point[ d ] = point[ d ];
          }
        }
      }
      assert( positions_.size() == expected_num_positions );
    }
    else if ( tkn.is_a< ParameterDatum >() )
    {
      auto pd = dynamic_cast< ParameterDatum* >( tkn.datum() );
      auto pos = dynamic_cast< DimensionParameter* >( pd->get() );

      const auto num_local_nodes = this->node_collection_->end() - this->node_collection_->MPI_local_begin();
      const auto num_devices = this->node_collection_->num_devices();
      // A NodeCollection with spatial information cannot be a composite, so it contains either only devices or
      // only regular nodes. Regular nodes are distributed over MPI processes, while devices are present on every
      // process, and therefore requires positions on all processes.
      const auto expected_num_positions = num_devices > 0 ? num_devices : num_local_nodes;

      positions_.clear();
      positions_.reserve( expected_num_positions );

      RngPtr rng = get_rank_synced_rng();

      for ( auto nc_it = this->node_collection_->begin(); nc_it < this->node_collection_->end(); ++nc_it )
      {
        Position< D > point = pos->get_values( rng );

        const auto node = kernel().node_manager.get_mpi_local_node_or_device_head( ( *nc_it ).node_id );
        assert( node );
        if ( not node->is_proxy() )
        {
          positions_.push_back( point );
        }
        // We do the calculation for lower_left_ and max_point even if we don't add the position, to keep the size of
        // the layer consistent over processes.
        for ( int d = 0; d < D; ++d )
        {
          if ( point[ d ] < this->lower_left_[ d ] )
          {
            this->lower_left_[ d ] = point[ d ];
          }
          if ( point[ d ] > max_point[ d ] )
          {
            max_point[ d ] = point[ d ];
          }
        }
      }
      assert( positions_.size() == expected_num_positions );
    }
    else
    {
      throw KernelException( "'positions' must be an array or a DimensionParameter." );
    }
  }
  if ( d->known( names::extent ) )
  {
    this->extent_ = getValue< std::vector< double > >( d, names::extent );

    Position< D > center = ( max_point + this->lower_left_ ) / 2;
    auto lower_left_point = this->lower_left_; // save lower-left-most point
    this->lower_left_ = center - this->extent_ / 2;

    // check if all points are inside the specified layer extent
    auto upper_right_limit = center + this->extent_ / 2;
    for ( int d = 0; d < D; ++d )
    {
      if ( lower_left_point[ d ] < this->lower_left_[ d ] or max_point[ d ] > upper_right_limit[ d ] )
      {
        throw BadProperty( "Node position outside of layer" );
      }
    }
  }
  else
  {
    this->extent_ = max_point - this->lower_left_;
    this->extent_ += epsilon * 2;
    this->lower_left_ -= epsilon;
  }
}

template < int D >
void
FreeLayer< D >::get_status( DictionaryDatum& d ) const
{
  Layer< D >::get_status( d );

  TokenArray points;
  for ( typename std::vector< Position< D > >::const_iterator it = positions_.begin(); it != positions_.end(); ++it )
  {
    points.push_back( it->getToken() );
  }
  def2< TokenArray, ArrayDatum >( d, names::positions, points );
}

template < int D >
Position< D >
FreeLayer< D >::get_position( index lid ) const
{
  const auto num_procs = kernel().mpi_manager.get_num_processes();
  const long diff = static_cast< long >( lid ) - num_procs + 1;
  const size_t position_id =
    std::floor( diff / static_cast< double >( num_procs ) ) + ( std::abs( diff ) % num_procs > 0 );
  return positions_.at( position_id );
}

template < int D >
template < class Ins >
void
FreeLayer< D >::communicate_positions_( Ins iter, NodeCollectionPTR node_collection )
{
  // This array will be filled with node ID,pos_x,pos_y[,pos_z] for local nodes:
  std::vector< double > local_node_id_pos;

  NodeCollection::const_iterator nc_begin = node_collection->MPI_local_begin();
  NodeCollection::const_iterator nc_end = node_collection->end();

  auto pos_it = positions_.begin();

  local_node_id_pos.reserve( ( D + 1 ) * node_collection->size() );

  for ( NodeCollection::const_iterator nc_it = nc_begin; nc_it < nc_end; ++nc_it, ++pos_it )
  {
    assert( pos_it != positions_.end() );
    // Push node ID into array to communicate
    local_node_id_pos.push_back( ( *nc_it ).node_id );
    // Push coordinates one by one
    for ( int j = 0; j < D; ++j )
    {
      local_node_id_pos.push_back( ( *pos_it )[ j ] );
    }
  }

  // This array will be filled with node ID,pos_x,pos_y[,pos_z] for global nodes:
  std::vector< double > global_node_id_pos;
  std::vector< int > displacements;
  kernel().mpi_manager.communicate( local_node_id_pos, global_node_id_pos, displacements );

  // To avoid copying the vector one extra time in order to sort, we
  // sneakishly use reinterpret_cast
  NodePositionData* pos_ptr;
  NodePositionData* pos_end;
  pos_ptr = reinterpret_cast< NodePositionData* >( &global_node_id_pos[ 0 ] );
  pos_end = pos_ptr + global_node_id_pos.size() / ( D + 1 );

  // Get rid of any multiple entries
  std::sort( pos_ptr, pos_end );
  pos_end = std::unique( pos_ptr, pos_end );

  // Unpack node IDs and coordinates
  for ( ; pos_ptr < pos_end; pos_ptr++ )
  {
    *iter++ = std::pair< Position< D >, index >( pos_ptr->get_position(), pos_ptr->get_node_id() );
  }
}

template < int D >
void
FreeLayer< D >::insert_global_positions_ntree_( Ntree< D, index >& tree, NodeCollectionPTR node_collection )
{

  communicate_positions_( std::inserter( tree, tree.end() ), node_collection );
}

// Helper function to compare node IDs used for sorting (Position,node ID) pairs
template < int D >
static bool
node_id_less( const std::pair< Position< D >, index >& a, const std::pair< Position< D >, index >& b )
{
  return a.second < b.second;
}

template < int D >
void
FreeLayer< D >::insert_global_positions_vector_( std::vector< std::pair< Position< D >, index > >& vec,
  NodeCollectionPTR node_collection )
{

  communicate_positions_( std::back_inserter( vec ), node_collection );

  // Sort vector to ensure consistent results
  std::sort( vec.begin(), vec.end(), node_id_less< D > );
}

} // namespace nest

#endif
