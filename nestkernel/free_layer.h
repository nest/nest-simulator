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

  /**
   * Calculate the index in the position vector on this MPI process based on the local ID.
   * @param lid local ID of the node
   * @return index in the local position vector
   */
  index lid_to_position_id_( index lid ) const;

  /// Vector of positions.
  std::vector< Position< D > > positions_;

  size_t num_local_nodes_ = 0;

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
    bool
    operator<( const NodePositionData& other ) const
    {
      return node_id_ < other.node_id_;
    }
    bool
    operator==( const NodePositionData& other ) const
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

  for ( int d = 0; d < D; ++d )
  {
    this->lower_left_[ d ] = std::numeric_limits< double >::infinity();
    max_point[ d ] = -std::numeric_limits< double >::infinity();
  }

  num_local_nodes_ = std::accumulate( this->node_collection_->begin(),
    this->node_collection_->end(),
    0,
    []( size_t a, NodeIDTriple b )
    {
      const auto node = kernel().node_manager.get_mpi_local_node_or_device_head( b.node_id );
      return node->is_proxy() ? a : a + 1;
    } );

  // Read positions from dictionary
  if ( d->known( names::positions ) )
  {
    const Token& tkn = d->lookup( names::positions );
    if ( tkn.is_a< TokenArray >() )
    {
      TokenArray pos = getValue< TokenArray >( tkn );

      positions_.clear();
      positions_.reserve( num_local_nodes_ );

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
      assert( positions_.size() == num_local_nodes_ );
    }
    else if ( tkn.is_a< ParameterDatum >() )
    {
      auto pd = dynamic_cast< ParameterDatum* >( tkn.datum() );
      auto pos = dynamic_cast< DimensionParameter* >( pd->get() );

      positions_.clear();
      positions_.reserve( num_local_nodes_ );

      RngPtr rng = get_rank_synced_rng();

      for ( auto nc_it = this->node_collection_->begin(); nc_it < this->node_collection_->end(); ++nc_it )
      {
        // We generate the position here, even if we do not store it, to do the same calculations of lower_left_ and
        // max_point on all processes.
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
      assert( positions_.size() == num_local_nodes_ );
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
    if ( this->node_collection_->size() <= 1 )
    {
      throw KernelException( "If only one node is created, 'extent' must be specified." );
    }

    const auto positional_extent = max_point - this->lower_left_;
    const auto center = ( max_point + this->lower_left_ ) / 2;
    for ( int d = 0; d < D; ++d )
    {
      // Set extent to be extent of the points, rounded up in each dimension.
      this->extent_[ d ] = std::ceil( positional_extent[ d ] );
    }

    // Adjust lower_left relative to the rounded center with the rounded up extent.
    this->lower_left_ = center - this->extent_ / 2;
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
  return positions_.at( lid_to_position_id_( lid ) );
}

template < int D >
template < class Ins >
void
FreeLayer< D >::communicate_positions_( Ins iter, NodeCollectionPTR node_collection )
{
  // This array will be filled with node ID,pos_x,pos_y[,pos_z] for local nodes:
  std::vector< double > local_node_id_pos;

  // If the NodeCollection has proxies, nodes and positions are distributed over MPI processes,
  // and we must iterate only the local nodes. If not, all nodes and positions are on all MPI processes.
  // All models in a layer are the same, so if has_proxies() for the NodeCollection returns true, we
  // know that all nodes in the NodeCollection have proxies. Likewise, if it returns false we know that
  // no nodes have proxies.
  NodeCollection::const_iterator nc_begin =
    node_collection->has_proxies() ? node_collection->MPI_local_begin() : node_collection->begin();
  NodeCollection::const_iterator nc_end = node_collection->end();

  // Reserve capacity in the vector based on number of local nodes. If the NodeCollection is sliced,
  // it may need less than the reserved capacity.
  local_node_id_pos.reserve( ( D + 1 ) * num_local_nodes_ );
  for ( NodeCollection::const_iterator nc_it = nc_begin; nc_it < nc_end; ++nc_it )
  {
    // Push node ID into array to communicate
    local_node_id_pos.push_back( ( *nc_it ).node_id );
    // Push coordinates one by one
    const auto pos = get_position( ( *nc_it ).lid );
    for ( int j = 0; j < D; ++j )
    {
      local_node_id_pos.push_back( pos[ j ] );
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

template < int D >
index
FreeLayer< D >::lid_to_position_id_( index lid ) const
{
  // If the NodeCollection has proxies, nodes and positions are distributed over MPI processes,
  // and we must iterate only the local nodes. If not, all nodes and positions are on all MPI processes.
  // All models in a layer are the same, so if has_proxies() for the NodeCollection returns true, we
  // know that all nodes in the NodeCollection have proxies. Likewise, if it returns false we know that
  // no nodes have proxies.
  if ( not this->node_collection_->has_proxies() )
  {
    return lid;
  }
  else
  {
    const auto num_procs = kernel().mpi_manager.get_num_processes();
    return lid / num_procs;
  }
}

} // namespace nest

#endif
