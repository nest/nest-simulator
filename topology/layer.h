/*
 *  layer.h
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

#ifndef LAYER_H
#define LAYER_H

// C++ includes:
#include <bitset>
#include <iostream>
#include <utility>

// Includes from nestkernel:
#include "kernel_manager.h"
#include "nest_types.h"

// Includes from sli:
#include "dictutils.h"

// Includes from topology:
#include "connection_creator.h"
#include "ntree.h"
#include "position.h"
#include "topology_names.h"

namespace nest
{

class AbstractLayer;
typedef std::shared_ptr< AbstractLayer > AbstractLayerPTR;

/**
 * Abstract base class for Layers of unspecified dimension.
 */
class AbstractLayer
{
public:
  /**
   * Constructor.
   */
  AbstractLayer()
    : node_collection_( NodeCollectionPTR( 0 ) )
  {
  }

  /**
   * Virtual destructor
   */
  virtual ~AbstractLayer();

  /**
   * Change properties of the layer according to the
   * entries in the dictionary.
   * @param d Dictionary with named parameter settings.
   */
  virtual void set_status( const DictionaryDatum& ) = 0;

  /**
   * Export properties of the layer by setting
   * entries in the status dictionary.
   * @param d Dictionary.
   */
  virtual void get_status( DictionaryDatum& ) const = 0;

  virtual unsigned int get_num_dimensions() const = 0;

  /**
   * Get position of node. Only possible for local nodes.
   * @param lid index of node within layer
   * @returns position of node as std::vector
   */
  virtual std::vector< double > get_position_vector( const index lid ) const = 0;

  /**
   * Returns displacement of node from given position. When using periodic
   * boundary conditions, will return minimum displacement.
   * @param from_pos  position vector in layer
   * @param to        node in layer to which displacement is to be computed
   * @returns vector pointing from from_pos to node to's position
   */
  virtual std::vector< double > compute_displacement( const std::vector< double >& from_pos, const index to ) const = 0;
  virtual double compute_displacement( const std::vector< double >& from_pos,
    const std::vector< double >& to_pos,
    const unsigned int dimension ) const = 0;

  /**
   * Returns distance to node from given position. When using periodic
   * boundary conditions, will return minimum distance.
   * @param from_pos  position vector in layer
   * @param to        node in layer to which displacement is to be computed
   * @returns length of vector pointing from from_pos to node to's position
   */
  virtual double compute_distance( const std::vector< double >& from_pos, const index lid ) const = 0;
  virtual double compute_distance( const std::vector< double >& from_pos,
    const std::vector< double >& to_pos ) const = 0;

  /**
   * Connect this layer to the given target layer. The actual connections
   * are made in class ConnectionCreator.
   * @param target    target layer to connect to. Must have same dimension
   *                  as this layer.
   * @param connector connection properties
   */
  virtual void connect( AbstractLayerPTR target, NodeCollectionPTR target_nc, ConnectionCreator& connector ) = 0;

  /**
   * Factory function for layers. The supplied dictionary contains
   * parameters which specify the layer type and type-specific
   * parameters.
   * @returns pointer to NodeCollection for new layer
   */
  static NodeCollectionPTR create_layer( const DictionaryDatum& );

  /**
   * Return a vector with the node IDs of the nodes inside the mask.
   * @param mask            mask to apply.
   * @param anchor          position to center mask in.
   * @param allow_oversized allow mask to be greater than layer
   * @returns nodes in layer inside mask.
   */
  virtual std::vector< index >
  get_global_nodes( const MaskDatum& mask, const std::vector< double >& anchor, bool allow_oversized ) = 0;

  /**
   * Write layer data to stream.
   * For each node in layer, write one line to stream containing:
   * node ID x-position y-position [z-position]
   * @param os     output stream
   */
  virtual void dump_nodes( std::ostream& os ) const = 0;

  /**
   * Dumps information about all connections of the given type having their
   * source in
   * the given layer to the given output stream. For distributed simulations
   * this function will dump the connections with local targets only.
   * @param out output stream
   * @param synapse_id type of connection
   */
  virtual void dump_connections( std::ostream& out, AbstractLayerPTR target_layer, const Token& syn_model ) = 0;

protected:
  /**
   * The NodeCollection to which the layer belongs
   */
  NodeCollectionPTR node_collection_;

  /**
   * Metadata for the layer for which we cache global position information
   */
  static NodeCollectionMetadataPTR cached_ntree_md_;

  /**
   * Metadata for the layer for which we cache global position information
   */
  static NodeCollectionMetadataPTR cached_vector_md_;

  /**
   * Clear the cache for global position information
   */
  virtual void clear_ntree_cache_() const = 0;

  /**
   * Clear the cache for global position information
   */
  virtual void clear_vector_cache_() const = 0;

  /**
   * Gets metadata of the NodeCollection to which this layer belongs.
   */
  NodeCollectionMetadataPTR get_metadata() const;
};

template < int D >
class MaskedLayer;

/**
 * Abstract base class for Layer of given dimension (D=2 or 3).
 */
template < int D >
class Layer : public AbstractLayer
{
public:
  /**
   * Creates an empty layer.
   */
  Layer();

  /**
   * Copy constructor.
   */
  Layer( const Layer& other_layer );

  /**
   * Virtual destructor
   */
  ~Layer();

  /**
   * Change properties of the layer according to the
   * entries in the dictionary.
   * @param d Dictionary with named parameter settings.
   */
  void set_status( const DictionaryDatum& );

  /**
   * Export properties of the layer by setting
   * entries in the status dictionary.
   * @param d Dictionary.
   */
  void get_status( DictionaryDatum& ) const;

  unsigned int
  get_num_dimensions() const
  {
    return D;
  }

  /**
   * @returns The bottom left position of the layer
   */
  const Position< D >&
  get_lower_left() const
  {
    return lower_left_;
  }

  /**
   * @returns extent of layer.
   */
  const Position< D >&
  get_extent() const
  {
    return extent_;
  }

  /**
   * @returns center of layer.
   */
  Position< D >
  get_center() const
  {
    return lower_left_ + extent_ / 2;
  }

  /**
   * @returns a bitmask specifying which directions are periodic
   */
  std::bitset< D >
  get_periodic_mask() const
  {
    return periodic_;
  }

  /**
   * Get position of node. Only possible for local nodes.
   * @param sind local subnet index of node
   * @returns position of node identified by Subnet local index value.
   */
  virtual Position< D > get_position( index sind ) const = 0;

  /**
   * @param sind local subnet index of node
   * @returns position of node as std::vector
   */
  std::vector< double > get_position_vector( const index sind ) const;

  /**
   * Returns displacement of a position from another position. When using
   * periodic boundary conditions, will return minimum displacement.
   * @param from_pos  position vector in layer
   * @param to_pos    position to which displacement is to be computed
   * @returns vector pointing from from_pos to to_pos
   */
  virtual Position< D > compute_displacement( const Position< D >& from_pos, const Position< D >& to_pos ) const;
  virtual double compute_displacement( const std::vector< double >& from_pos,
    const std::vector< double >& to_pos,
    const unsigned int dimension ) const;

  /**
   * Returns displacement of node from given position. When using periodic
   * boundary conditions, will return minimum displacement.
   * @param from_pos  position vector in layer
   * @param to        node in layer to which displacement is to be computed
   * @returns vector pointing from from_pos to node to's position
   */
  Position< D > compute_displacement( const Position< D >& from_pos, const index to ) const;

  std::vector< double > compute_displacement( const std::vector< double >& from_pos, const index to ) const;

  /**
   * Returns distance to node from given position. When using periodic
   * boundary conditions, will return minimum distance.
   * @param from_pos  position vector in layer
   * @param to        node in layer to which displacement is to be computed
   * @returns length of vector pointing from from_pos to node to's position
   */
  double compute_distance( const Position< D >& from_pos, const index lid ) const;

  double compute_distance( const std::vector< double >& from_pos, const index lid ) const;

  double compute_distance( const std::vector< double >& from_pos, const std::vector< double >& to_pos ) const;


  /**
   * Get positions for all nodes in layer, including nodes on other MPI
   * processes. The positions will be cached so that subsequent calls for
   * the same layer are fast. One one layer is cached at the time, so the
   * user should group together all ConnectLayers calls using the same
   * pool layer.
   */
  std::shared_ptr< Ntree< D, index > > get_global_positions_ntree();

  /**
   * Get positions globally, overriding the dimensions of the layer and
   * the periodic flags. The supplied lower left corner and extent
   * coordinates are only used for the dimensions where the supplied
   * periodic flag is set.
   */
  std::shared_ptr< Ntree< D, index > >
  get_global_positions_ntree( std::bitset< D > periodic, Position< D > lower_left, Position< D > extent );

  std::vector< std::pair< Position< D >, index > >* get_global_positions_vector();

  virtual std::vector< std::pair< Position< D >, index > >
  get_global_positions_vector( const MaskDatum& mask, const Position< D >& anchor, bool allow_oversized );

  /**
   * Return a vector with the node IDs of the nodes inside the mask.
   */
  std::vector< index >
  get_global_nodes( const MaskDatum& mask, const std::vector< double >& anchor, bool allow_oversized );

  /**
   * Connect this layer to the given target layer. The actual connections
   * are made in class ConnectionCreator.
   * @param target    target layer to connect to. Must have same dimension
   *                  as this layer.
   * @param target_nc NodeCollection to the target layer.
   * @param connector connection properties
   */
  void connect( AbstractLayerPTR target, NodeCollectionPTR target_nc, ConnectionCreator& connector );

  /**
   * Write layer data to stream.
   * For each node in layer, write one line to stream containing:
   * node ID x-position y-position [z-position]
   * @param os     output stream
   */
  void dump_nodes( std::ostream& os ) const;

  /**
   * Dumps information about all connections of the given type having their
   * source in the given layer to the given output stream. For distributed
   * simulations this function will dump the connections with local targets
   * only.
   * @param out output stream
   * @param synapse_id type of connection
   */
  void dump_connections( std::ostream& out, AbstractLayerPTR target_layer, const Token& syn_model );

protected:
  /**
   * Clear the cache for global position information
   */
  void clear_ntree_cache_() const;

  /**
   * Clear the cache for global position information
   */
  void clear_vector_cache_() const;

  std::shared_ptr< Ntree< D, index > > do_get_global_positions_ntree_();

  /**
   * Insert global position info into ntree.
   */
  virtual void insert_global_positions_ntree_( Ntree< D, index >& tree ) = 0;

  /**
   * Insert global position info into vector.
   */
  virtual void insert_global_positions_vector_( std::vector< std::pair< Position< D >, index > >& ) = 0;

  //! lower left corner (minimum coordinates) of layer
  Position< D > lower_left_;
  Position< D > extent_;      //!< size of layer
  std::bitset< D > periodic_; //!< periodic b.c.

  /**
   * Global position information for a single layer
   */
  static std::shared_ptr< Ntree< D, index > > cached_ntree_;
  static std::vector< std::pair< Position< D >, index > >* cached_vector_;

  friend class MaskedLayer< D >;
};

/**
 * Class for applying masks to layers. Contains begin and end methods to
 * iterate over nodes inside a mask.
 */
template < int D >
class MaskedLayer
{
public:
  /**
   * Regular constructor.
   * @param layer           The layer to mask
   * @param mask            The mask to apply to the layer
   * @param allow_oversized If true, allow larges masks than layers when using
   *                        periodic b.c.
   */
  MaskedLayer( Layer< D >& layer, const MaskDatum& mask, bool allow_oversized );

  /**
   * Constructor for applying "converse" mask to layer. To be used for
   * applying a mask for the target layer to the source layer. The mask
   * will be mirrored about the origin, and settings for periodicity for
   * the target layer will be applied to the source layer.
   * @param layer           The layer to mask (source layer)
   * @param mask            The mask to apply to the layer
   * @param allow_oversized If true, allow larges masks than layers when using
   * periodic b.c.
   * @param target          The layer which the given mask is defined for
   * (target layer)
   */
  MaskedLayer( Layer< D >& layer, const MaskDatum& mask, bool allow_oversized, Layer< D >& target );

  ~MaskedLayer();

  /**
   * Iterate over nodes inside mask
   * @param anchor Position to apply mask to
   * @returns an iterator for the nodes inside the mask centered on the anchor
   * position
   */
  typename Ntree< D, index >::masked_iterator begin( const Position< D >& anchor );

  /**
   * @return end iterator
   */
  typename Ntree< D, index >::masked_iterator end();

protected:
  /**
   * Will check that the mask can be applied to the layer. The mask must
   * have the same dimensionality as the layer, and a grid mask may only
   * be applied to a grid layer. Unless the allow_oversized flag is set,
   * the mask must also not be larger than the layer in case of periodic
   * boundary conditions. Will throw an exception if the mask does not
   * fit.
   * @param layer The layer to check for
   * @param allow_oversized If true, oversized masks are allowed
   */
  void check_mask_( Layer< D >& layer, bool allow_oversized );

  std::shared_ptr< Ntree< D, index > > ntree_;
  MaskDatum mask_;
};

template < int D >
inline MaskedLayer< D >::MaskedLayer( Layer< D >& layer, const MaskDatum& maskd, bool allow_oversized )
  : mask_( maskd )
{
  ntree_ = layer.get_global_positions_ntree();

  check_mask_( layer, allow_oversized );
}

template < int D >
inline MaskedLayer< D >::MaskedLayer( Layer< D >& layer,
  const MaskDatum& maskd,
  bool allow_oversized,
  Layer< D >& target )
  : mask_( maskd )
{
  ntree_ = layer.get_global_positions_ntree( target.get_periodic_mask(), target.get_lower_left(), target.get_extent() );

  check_mask_( target, allow_oversized );
  mask_ = new ConverseMask< D >( dynamic_cast< const Mask< D >& >( *mask_ ) );
}

template < int D >
inline MaskedLayer< D >::~MaskedLayer()
{
}

template < int D >
inline typename Ntree< D, index >::masked_iterator
MaskedLayer< D >::begin( const Position< D >& anchor )
{
  try
  {
    return ntree_->masked_begin( dynamic_cast< const Mask< D >& >( *mask_ ), anchor );
  }
  catch ( std::bad_cast& e )
  {
    throw BadProperty( "Mask is incompatible with layer." );
  }
}

template < int D >
inline typename Ntree< D, index >::masked_iterator
MaskedLayer< D >::end()
{
  return ntree_->masked_end();
}

template < int D >
inline Layer< D >::Layer()
{
  // Default center (0,0) and extent (1,1)
  for ( int i = 0; i < D; ++i )
  {
    lower_left_[ i ] = -0.5;
    extent_[ i ] = 1.0;
  }
}

template < int D >
inline Layer< D >::Layer( const Layer& other_layer )
  : AbstractLayer( other_layer )
  , lower_left_( other_layer.lower_left_ )
  , extent_( other_layer.extent_ )
  , periodic_( other_layer.periodic_ )
{
}

template < int D >
inline Layer< D >::~Layer()
{
  if ( cached_ntree_md_ == get_metadata() )
  {
    clear_ntree_cache_();
  }

  if ( cached_vector_md_ == get_metadata() )
  {
    clear_vector_cache_();
  }
}

template < int D >
inline Position< D >
Layer< D >::compute_displacement( const Position< D >& from_pos, const index to_lid ) const
{
  return compute_displacement( from_pos, get_position( to_lid ) );
}

template < int D >
inline std::vector< double >
Layer< D >::compute_displacement( const std::vector< double >& from_pos, const index to_lid ) const
{
  return std::vector< double >( compute_displacement( Position< D >( from_pos ), to_lid ).get_vector() );
}

template < int D >
inline double
Layer< D >::compute_distance( const Position< D >& from_pos, const index lid ) const
{
  if ( lid < 0 )
  {
    throw KernelException( "node ID not in NodeCollection." );
  }
  return compute_displacement( from_pos, lid ).length();
}

template < int D >
inline double
Layer< D >::compute_distance( const std::vector< double >& from_pos, const index lid ) const
{
  if ( lid < 0 )
  {
    throw KernelException( "node ID not in NodeCollection." );
  }
  return compute_displacement( Position< D >( from_pos ), lid ).length();
}

template < int D >
inline double
Layer< D >::compute_distance( const std::vector< double >& from_pos, const std::vector< double >& to_pos ) const
{
  double squared_displacement = 0;
  for ( unsigned int i = 0; i < D; ++i )
  {
    const double displacement = compute_displacement( from_pos, to_pos, i );
    squared_displacement += displacement * displacement;
  }
  return std::sqrt( squared_displacement );
}

template < int D >
inline std::vector< double >
Layer< D >::get_position_vector( const index sind ) const
{
  return get_position( sind ).get_vector();
}

template < int D >
inline void
Layer< D >::clear_ntree_cache_() const
{
  cached_ntree_ = std::shared_ptr< Ntree< D, index > >();
  cached_ntree_md_ = NodeCollectionMetadataPTR( 0 );
}

template < int D >
inline void
Layer< D >::clear_vector_cache_() const
{
  if ( cached_vector_ != 0 )
  {
    delete cached_vector_;
  }
  cached_vector_ = 0;
  cached_vector_md_ = NodeCollectionMetadataPTR( 0 );
}

} // namespace nest

#endif
