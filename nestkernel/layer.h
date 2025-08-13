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
#include "nest_names.h"
#include "nest_types.h"

// Includes from sli:
#include "booldatum.h"
#include "dictutils.h"
#include "grid_mask.h"

// Includes from spatial:
#include "connection_creator.h"
#include "ntree.h"
#include "position.h"

namespace nest
{

template < int D >
class GridLayer;

class AbstractLayer;
typedef std::shared_ptr< AbstractLayer > AbstractLayerPTR;

/**
 * Abstract base class for Layers of unspecified dimension.
 */
class AbstractLayer
{
public:
  AbstractLayer()
    : node_collection_( NodeCollectionPTR( nullptr ) )
  {
  }

  virtual ~AbstractLayer();

  /**
   * Change properties of the layer according to the
   * entries in the dictionary.
   * @param d Dictionary with named parameter settings.
   */
  virtual void set_status( const DictionaryDatum& ) = 0;

  /**
   * Export properties of the layer by setting
   * entries in the status dictionary, respects slicing of given NodeCollection
   * @param d Dictionary.
   *
   * @note If nullptr is passed for NodeCollection*, full metadata irrespective of any slicing is returned.
   */
  virtual void get_status( DictionaryDatum&, NodeCollection const* ) const = 0;

  virtual unsigned int get_num_dimensions() const = 0;

  /**
   * Get position of node. Only possible for local nodes.
   *
   * @param lid global index of node within layer
   * @returns position of node as std::vector
   */
  virtual std::vector< double > get_position_vector( const size_t lid ) const = 0;

  /**
   * Returns displacement of node from given position. When using periodic
   * boundary conditions, will return minimum displacement.
   *
   * @param from_pos  position vector in layer
   * @param to        node in layer to which displacement is to be computed
   * @returns vector pointing from from_pos to node to's position
   */
  virtual std::vector< double > compute_displacement( const std::vector< double >& from_pos,
    const size_t to ) const = 0;
  virtual double compute_displacement( const std::vector< double >& from_pos,
    const std::vector< double >& to_pos,
    const unsigned int dimension ) const = 0;

  /**
   * Returns distance to node from given position. When using periodic
   * boundary conditions, will return minimum distance.
   *
   * @param from_pos  position vector in layer
   * @param to        node in layer to which displacement is to be computed
   * @returns length of vector pointing from from_pos to node to's position
   */
  virtual double compute_distance( const std::vector< double >& from_pos, const size_t lid ) const = 0;
  virtual double compute_distance( const std::vector< double >& from_pos,
    const std::vector< double >& to_pos ) const = 0;

  /**
   * Connect this layer to the given target layer. The actual connections
   * are made in class ConnectionCreator.
   *
   * @param source_nc NodeCollection of the source layer
   * @param target    target layer to connect to. Must have same dimension
   *                  as this layer.
   * @param target_nc NodeCollection of the target layer
   * @param connector connection properties
   */
  virtual void connect( NodeCollectionPTR source_nc,
    AbstractLayerPTR target,
    NodeCollectionPTR target_nc,
    ConnectionCreator& connector ) = 0;

  /**
   * Factory function for layers. The supplied dictionary contains
   * parameters which specify the layer type and type-specific
   * parameters.
   * @returns pointer to NodeCollection for new layer
   */
  static NodeCollectionPTR create_layer( const DictionaryDatum& );

  /**
   * Return a vector with the node IDs of the nodes inside the mask.
   *
   * @param mask            mask to apply.
   * @param anchor          position to center mask in.
   * @param allow_oversized allow mask to be greater than layer
   * @param node_collection NodeCollection of the layer
   * @returns nodes in layer inside mask.
   */
  virtual std::vector< size_t > get_global_nodes( const MaskDatum& mask,
    const std::vector< double >& anchor,
    bool allow_oversized,
    NodeCollectionPTR node_collection ) = 0;

  /**
   * Write layer data to stream.
   *
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
   *
   * @param out output stream
   * @param node_collection NodeCollection of the layer
   * @param target_layer Target layer
   * @param synapse_id type of connection
   */
  virtual void dump_connections( std::ostream& out,
    NodeCollectionPTR node_collection,
    AbstractLayerPTR target_layer,
    const Token& syn_model ) = 0;

  void set_node_collection( NodeCollectionPTR );
  NodeCollectionPTR get_node_collection();

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

  Layer( const Layer& other_layer );

  ~Layer() override;

  /**
   * Change properties of the layer according to the
   * entries in the dictionary.
   *
   * @param d Dictionary with named parameter settings.
   */
  void set_status( const DictionaryDatum& ) override;

  //! Retrieve status, slice according to node collection if given
  void get_status( DictionaryDatum&, NodeCollection const* ) const override;

  unsigned int
  get_num_dimensions() const override
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
   *
   * @param sind index of node
   * @returns position of node.
   */
  virtual Position< D > get_position( size_t sind ) const = 0;

  /**
   * @param sind index of node
   * @returns position of node as std::vector
   */
  std::vector< double > get_position_vector( const size_t sind ) const override;

  /**
   * Returns displacement of a position from another position. When using
   * periodic boundary conditions, will return minimum displacement.
   *
   * @param from_pos  position vector in layer
   * @param to_pos    position to which displacement is to be computed
   * @returns vector pointing from from_pos to to_pos
   */
  Position< D > compute_displacement( const Position< D >& from_pos, const Position< D >& to_pos ) const;
  double compute_displacement( const std::vector< double >& from_pos,
    const std::vector< double >& to_pos,
    const unsigned int dimension ) const override;

  /**
   * Returns displacement of node from given position. When using periodic
   * boundary conditions, will return minimum displacement.
   *
   * @param from_pos  position vector in layer
   * @param to        node in layer to which displacement is to be computed
   * @returns vector pointing from from_pos to node to's position
   */
  Position< D > compute_displacement( const Position< D >& from_pos, const size_t to ) const;

  std::vector< double > compute_displacement( const std::vector< double >& from_pos, const size_t to ) const override;

  /**
   * Returns distance to node from given position. When using periodic
   * boundary conditions, will return minimum distance.
   *
   * @param from_pos  position vector in layer
   * @param to        node in layer to which displacement is to be computed
   * @returns length of vector pointing from from_pos to node to's position
   */
  double compute_distance( const Position< D >& from_pos, const size_t lid ) const;

  double compute_distance( const std::vector< double >& from_pos, const size_t lid ) const override;

  double compute_distance( const std::vector< double >& from_pos, const std::vector< double >& to_pos ) const override;


  /**
   * Get positions for all nodes in layer, including nodes on other MPI processes.
   *
   * The positions will be cached so that subsequent calls for
   * the same layer are fast. One one layer is cached at the time, so the
   * user should group together all ConnectLayers calls using the same
   * pool layer.
   */
  std::shared_ptr< Ntree< D, size_t > > get_global_positions_ntree( NodeCollectionPTR node_collection );

  /**
   * Get positions globally, overriding the dimensions of the layer and the periodic flags.

   * The supplied lower left corner and extent
   * coordinates are only used for the dimensions where the supplied
   * periodic flag is set.
   */
  std::shared_ptr< Ntree< D, size_t > > get_global_positions_ntree( std::bitset< D > periodic,
    Position< D > lower_left,
    Position< D > extent,
    NodeCollectionPTR node_collection );

  std::vector< std::pair< Position< D >, size_t > >* get_global_positions_vector( NodeCollectionPTR node_collection );

  virtual std::vector< std::pair< Position< D >, size_t > > get_global_positions_vector( const MaskDatum& mask,
    const Position< D >& anchor,
    bool allow_oversized,
    NodeCollectionPTR node_collection );

  /**
   * Return a vector with the node IDs of the nodes inside the mask.
   */
  std::vector< size_t > get_global_nodes( const MaskDatum& mask,
    const std::vector< double >& anchor,
    bool allow_oversized,
    NodeCollectionPTR node_collection ) override;

  /**
   * Connect this layer to the given target layer. The actual connections
   * are made in class ConnectionCreator.
   *
   * @param source_nc NodeCollection to the source layer.
   * @param target    target layer to connect to. Must have same dimension
   *                  as this layer.
   * @param target_nc NodeCollection to the target layer.
   * @param connector connection properties
   */
  void connect( NodeCollectionPTR source_nc,
    AbstractLayerPTR target,
    NodeCollectionPTR target_nc,
    ConnectionCreator& connector ) override;

  /**
   * Write layer data to stream.
   *
   * For each node in layer, write one line to stream containing:
   * node ID x-position y-position [z-position]
   * @param os     output stream
   */
  void dump_nodes( std::ostream& os ) const override;

  /**
   * Dumps information about all connections of the given type having their
   * source in the given layer to the given output stream.
   *
   * For distributed
   * simulations this function will dump the connections with local targets
   * only.
   * @param out output stream
   * @param node_collection NodeCollection of the layer
   * @param target_layer Target layer
   * @param synapse_id type of connection
   */
  void dump_connections( std::ostream& out,
    NodeCollectionPTR node_collection,
    AbstractLayerPTR target_layer,
    const Token& syn_model ) override;

protected:
  /**
   * Clear the cache for global position information
   */
  void clear_ntree_cache_() const override;

  /**
   * Clear the cache for global position information
   */
  void clear_vector_cache_() const override;

  std::shared_ptr< Ntree< D, size_t > > do_get_global_positions_ntree_( NodeCollectionPTR node_collection );

  /**
   * Insert global position info into ntree.
   */
  virtual void insert_global_positions_ntree_( Ntree< D, size_t >& tree, NodeCollectionPTR node_collection ) = 0;

  /**
   * Insert global position info into vector.
   */
  virtual void insert_global_positions_vector_( std::vector< std::pair< Position< D >, size_t > >&,
    NodeCollectionPTR ) = 0;

  //! lower left corner (minimum coordinates) of layer
  Position< D > lower_left_;
  Position< D > extent_;      //!< size of layer
  std::bitset< D > periodic_; //!< periodic b.c.

  /**
   * Global position information for a single layer
   */
  static std::shared_ptr< Ntree< D, size_t > > cached_ntree_;
  static std::vector< std::pair< Position< D >, size_t > >* cached_vector_;

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
   *
   * @param layer           The layer to mask
   * @param mask            The mask to apply to the layer
   * @param allow_oversized If true, allow larges masks than layers when using
   *                        periodic b.c.
   * @param node_collection NodeCollection of the layer
   */
  MaskedLayer( Layer< D >& layer, const MaskDatum& mask, bool allow_oversized, NodeCollectionPTR node_collection );

  /**
   * Constructor for applying "converse" mask to layer.
   *
   * To be used for applying a mask for the target layer to the source layer. The mask
   * will be mirrored about the origin, and settings for periodicity for
   * the target layer will be applied to the source layer.
   * @param layer           The layer to mask (source layer)
   * @param mask            The mask to apply to the layer
   * @param allow_oversized If true, allow larges masks than layers when using periodic b.c.
   * @param target          The layer which the given mask is defined for (target layer)
   * @param node_collection NodeCollection of the layer
   */
  MaskedLayer( Layer< D >& layer,
    const MaskDatum& mask,
    bool allow_oversized,
    Layer< D >& target,
    NodeCollectionPTR node_collection );

  ~MaskedLayer();

  /**
   * Iterate over nodes inside mask
   *
   * @param anchor Position to apply mask to
   * @returns an iterator for the nodes inside the mask centered on the anchor
   * position
   */
  typename Ntree< D, size_t >::masked_iterator begin( const Position< D >& anchor );

  /**
   * @return end iterator
   */
  typename Ntree< D, size_t >::masked_iterator end();

protected:
  /**
   * Will check that the mask can be applied to the layer.
   *
   * The mask must have the same dimensionality as the layer, and a grid mask may only
   * be applied to a grid layer. Unless the allow_oversized flag is set,
   * the mask must also not be larger than the layer in case of periodic
   * boundary conditions. Will throw an exception if the mask does not
   * fit.
   * @param layer The layer to check for
   * @param allow_oversized If true, oversized masks are allowed
   */
  void check_mask_( Layer< D >& layer, bool allow_oversized );

  std::shared_ptr< Ntree< D, size_t > > ntree_;
  MaskDatum mask_;
};

inline void
AbstractLayer::set_node_collection( NodeCollectionPTR node_collection )
{
  node_collection_ = node_collection;
}


inline NodeCollectionPTR
AbstractLayer::get_node_collection()
{
  return node_collection_;
}

template < int D >
inline MaskedLayer< D >::MaskedLayer( Layer< D >& layer,
  const MaskDatum& maskd,
  bool allow_oversized,
  NodeCollectionPTR node_collection )
  : mask_( maskd )
{
  ntree_ = layer.get_global_positions_ntree( node_collection );

  check_mask_( layer, allow_oversized );
}

template < int D >
inline MaskedLayer< D >::MaskedLayer( Layer< D >& layer,
  const MaskDatum& maskd,
  bool allow_oversized,
  Layer< D >& target,
  NodeCollectionPTR node_collection )
  : mask_( maskd )
{
  ntree_ = layer.get_global_positions_ntree(
    target.get_periodic_mask(), target.get_lower_left(), target.get_extent(), node_collection );

  check_mask_( target, allow_oversized );
  mask_ = new ConverseMask< D >( dynamic_cast< const Mask< D >& >( *mask_ ) );
}

template < int D >
inline MaskedLayer< D >::~MaskedLayer()
{
}

template < int D >
inline typename Ntree< D, size_t >::masked_iterator
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
inline typename Ntree< D, size_t >::masked_iterator
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
Layer< D >::compute_displacement( const Position< D >& from_pos, const size_t to_lid ) const
{
  return compute_displacement( from_pos, get_position( to_lid ) );
}

template < int D >
inline std::vector< double >
Layer< D >::compute_displacement( const std::vector< double >& from_pos, const size_t to_lid ) const
{
  return std::vector< double >( compute_displacement( Position< D >( from_pos ), to_lid ).get_vector() );
}

template < int D >
inline double
Layer< D >::compute_distance( const Position< D >& from_pos, const size_t lid ) const
{
  return compute_displacement( from_pos, lid ).length();
}

template < int D >
inline double
Layer< D >::compute_distance( const std::vector< double >& from_pos, const size_t lid ) const
{
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
Layer< D >::get_position_vector( const size_t sind ) const
{
  return get_position( sind ).get_vector();
}

template < int D >
inline void
Layer< D >::clear_ntree_cache_() const
{
  cached_ntree_ = std::shared_ptr< Ntree< D, size_t > >();
  cached_ntree_md_ = NodeCollectionMetadataPTR( nullptr );
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
  cached_vector_md_ = NodeCollectionMetadataPTR( nullptr );
}

template < int D >
std::shared_ptr< Ntree< D, size_t > > Layer< D >::cached_ntree_;

template < int D >
std::vector< std::pair< Position< D >, size_t > >* Layer< D >::cached_vector_ = 0;

template < int D >
Position< D >
Layer< D >::compute_displacement( const Position< D >& from_pos, const Position< D >& to_pos ) const
{
  Position< D > displ = to_pos;
  for ( int i = 0; i < D; ++i )
  {
    displ[ i ] -= from_pos[ i ];
    if ( periodic_[ i ] )
    {
      displ[ i ] = -0.5 * extent_[ i ] + std::fmod( displ[ i ] + 0.5 * extent_[ i ], extent_[ i ] );
      if ( displ[ i ] < -0.5 * extent_[ i ] )
      {
        displ[ i ] += extent_[ i ];
      }
    }
  }
  return displ;
}

template < int D >
double
Layer< D >::compute_displacement( const std::vector< double >& from_pos,
  const std::vector< double >& to_pos,
  const unsigned int dimension ) const
{
  double displacement = to_pos[ dimension ] - from_pos[ dimension ];
  if ( periodic_[ dimension ] )
  {
    displacement -= extent_[ dimension ] * std::round( displacement * ( 1 / extent_[ dimension ] ) );
  }
  return displacement;
}

template < int D >
void
Layer< D >::set_status( const DictionaryDatum& d )
{
  if ( d->known( names::edge_wrap ) )
  {
    if ( getValue< bool >( d, names::edge_wrap ) )
    {
      periodic_ = ( 1 << D ) - 1; // All dimensions periodic
    }
  }
}

template < int D >
void
Layer< D >::get_status( DictionaryDatum& d, NodeCollection const* nc ) const
{
  ( *d )[ names::extent ] = std::vector< double >( extent_.get_vector() );
  ( *d )[ names::center ] = std::vector< double >( ( lower_left_ + extent_ / 2 ).get_vector() );

  if ( periodic_.none() )
  {
    ( *d )[ names::edge_wrap ] = BoolDatum( false );
  }
  else if ( periodic_.count() == D )
  {
    ( *d )[ names::edge_wrap ] = true;
  }

  if ( nc )
  {
    // This is for backward compatibility with some tests and scripts
    // TODO: Rename parameter
    ( *d )[ names::network_size ] = nc->size();
  }
}

template < int D >
void
Layer< D >::connect( NodeCollectionPTR source_nc,
  AbstractLayerPTR target_layer,
  NodeCollectionPTR target_nc,
  ConnectionCreator& connector )
{
  // We need to extract the real pointer here to be able to cast to the
  // dimension-specific subclass.
  AbstractLayer* target_abs = target_layer.get();
  assert( target_abs );

  try
  {
    Layer< D >& tgt = dynamic_cast< Layer< D >& >( *target_abs );
    connector.connect( *this, source_nc, tgt, target_nc );
  }
  catch ( std::bad_cast& e )
  {
    throw BadProperty( "Target layer must have same number of dimensions as source layer." );
  }
}

template < int D >
std::shared_ptr< Ntree< D, size_t > >
Layer< D >::get_global_positions_ntree( NodeCollectionPTR node_collection )
{
  if ( cached_ntree_md_ == node_collection->get_metadata() )
  {
    assert( cached_ntree_.get() );
    return cached_ntree_;
  }

  clear_ntree_cache_();

  cached_ntree_ = std::shared_ptr< Ntree< D, size_t > >(
    new Ntree< D, size_t >( this->lower_left_, this->extent_, this->periodic_ ) );

  return do_get_global_positions_ntree_( node_collection );
}

template < int D >
std::shared_ptr< Ntree< D, size_t > >
Layer< D >::get_global_positions_ntree( std::bitset< D > periodic,
  Position< D > lower_left,
  Position< D > extent,
  NodeCollectionPTR node_collection )
{
  clear_ntree_cache_();
  clear_vector_cache_();

  // Keep layer geometry for non-periodic dimensions
  for ( int i = 0; i < D; ++i )
  {
    if ( not periodic[ i ] )
    {
      extent[ i ] = extent_[ i ];
      lower_left[ i ] = lower_left_[ i ];
    }
  }

  cached_ntree_ =
    std::shared_ptr< Ntree< D, size_t > >( new Ntree< D, size_t >( this->lower_left_, extent, periodic ) );

  do_get_global_positions_ntree_( node_collection );

  // Do not use cache since the periodic bits and extents were altered.
  cached_ntree_md_ = NodeCollectionMetadataPTR( nullptr );

  return cached_ntree_;
}

template < int D >
std::shared_ptr< Ntree< D, size_t > >
Layer< D >::do_get_global_positions_ntree_( NodeCollectionPTR node_collection )
{
  if ( cached_vector_md_ == node_collection->get_metadata() )
  {
    // Convert from vector to Ntree

    typename std::insert_iterator< Ntree< D, size_t > > to = std::inserter( *cached_ntree_, cached_ntree_->end() );

    for ( typename std::vector< std::pair< Position< D >, size_t > >::iterator from = cached_vector_->begin();
          from != cached_vector_->end();
          ++from )
    {
      *to = *from;
    }
  }
  else
  {

    insert_global_positions_ntree_( *cached_ntree_, node_collection );
  }

  clear_vector_cache_();

  cached_ntree_md_ = node_collection->get_metadata();

  return cached_ntree_;
}

template < int D >
std::vector< std::pair< Position< D >, size_t > >*
Layer< D >::get_global_positions_vector( NodeCollectionPTR node_collection )
{
  if ( cached_vector_md_ == node_collection->get_metadata() )
  {
    assert( cached_vector_ );
    return cached_vector_;
  }

  clear_vector_cache_();

  cached_vector_ = new std::vector< std::pair< Position< D >, size_t > >;

  if ( cached_ntree_md_ == node_collection->get_metadata() )
  {
    // Convert from NTree to vector

    typename std::back_insert_iterator< std::vector< std::pair< Position< D >, size_t > > > to =
      std::back_inserter( *cached_vector_ );

    for ( typename Ntree< D, size_t >::iterator from = cached_ntree_->begin(); from != cached_ntree_->end(); ++from )
    {
      *to = *from;
    }
  }
  else
  {
    insert_global_positions_vector_( *cached_vector_, node_collection );
  }

  clear_ntree_cache_();

  cached_vector_md_ = node_collection->get_metadata();

  return cached_vector_;
}

template < int D >
std::vector< std::pair< Position< D >, size_t > >
Layer< D >::get_global_positions_vector( const MaskDatum& mask,
  const Position< D >& anchor,
  bool allow_oversized,
  NodeCollectionPTR node_collection )
{
  MaskedLayer< D > masked_layer( *this, mask, allow_oversized, node_collection );
  std::vector< std::pair< Position< D >, size_t > > positions;

  for ( typename Ntree< D, size_t >::masked_iterator iter = masked_layer.begin( anchor ); iter != masked_layer.end();
        ++iter )
  {
    positions.push_back( *iter );
  }

  return positions;
}

template < int D >
std::vector< size_t >
Layer< D >::get_global_nodes( const MaskDatum& mask,
  const std::vector< double >& anchor,
  bool allow_oversized,
  NodeCollectionPTR node_collection )
{
  MaskedLayer< D > masked_layer( *this, mask, allow_oversized, node_collection );
  std::vector< size_t > nodes;
  for ( typename Ntree< D, size_t >::masked_iterator i = masked_layer.begin( anchor ); i != masked_layer.end(); ++i )
  {
    nodes.push_back( i->second );
  }
  return nodes;
}

template < int D >
void
Layer< D >::dump_nodes( std::ostream& out ) const
{
  for ( NodeCollection::const_iterator it = this->node_collection_->rank_local_begin();
        it < this->node_collection_->end();
        ++it )
  {
    out << ( *it ).node_id << ' ';
    get_position( ( *it ).nc_index ).print( out );
    out << std::endl;
  }
}

template < int D >
void
Layer< D >::dump_connections( std::ostream& out,
  NodeCollectionPTR node_collection,
  AbstractLayerPTR target_layer,
  const Token& syn_model )
{
  // Find all connections for given sources, targets and synapse model
  DictionaryDatum conn_filter( new Dictionary );
  def( conn_filter, names::source, NodeCollectionDatum( node_collection ) );
  def( conn_filter, names::target, NodeCollectionDatum( target_layer->get_node_collection() ) );
  def( conn_filter, names::synapse_model, syn_model );
  ArrayDatum connectome = kernel::manager< ConnectionManager >().get_connections( conn_filter );

  // Get positions of remote nodes
  std::vector< std::pair< Position< D >, size_t > >* src_vec = get_global_positions_vector( node_collection );

  // Iterate over connectome and write every connection, looking up source position only if source neuron changes
  size_t previous_source_node_id = 0; // dummy initial value, cannot be node_id of any node
  Position< D > source_pos;           // dummy value
  for ( const auto& entry : connectome )
  {
    ConnectionDatum conn = getValue< ConnectionDatum >( entry );
    const size_t source_node_id = conn.get_source_node_id();

    // Search source_pos for source node only if it is a different node
    if ( source_node_id != previous_source_node_id )
    {
      const auto it = std::find_if( src_vec->begin(),
        src_vec->end(),
        [ source_node_id ]( const std::pair< Position< D >, size_t >& p ) { return p.second == source_node_id; } );
      assert( it != src_vec->end() ); // internal error if node not found

      source_pos = it->first;
      previous_source_node_id = source_node_id;
    }

    DictionaryDatum result_dict = kernel::manager< ConnectionManager >().get_synapse_status( source_node_id,
      conn.get_target_node_id(),
      conn.get_target_thread(),
      conn.get_synapse_model_id(),
      conn.get_port() );
    const long target_node_id = getValue< long >( result_dict, names::target );
    const double weight = getValue< double >( result_dict, names::weight );
    const double delay = getValue< double >( result_dict, names::delay );
    const Layer< D >* const tgt_layer = dynamic_cast< Layer< D >* >( target_layer.get() );
    const long tnode_lid = tgt_layer->node_collection_->get_nc_index( target_node_id );
    assert( tnode_lid >= 0 );

    // Print source, target, weight, delay, rports
    out << source_node_id << ' ' << target_node_id << ' ' << weight << ' ' << delay << ' ';
    tgt_layer->compute_displacement( source_pos, tnode_lid ).print( out );
    out << '\n';
  }
}

} // namespace nest

#endif
