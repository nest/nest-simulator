/*
 *  layer_impl.h
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

#ifndef LAYER_IMPL_H
#define LAYER_IMPL_H

#include "layer.h"

// Includes from nestkernel:
#include "node_collection.h"
#include "nest_datums.h"
#include "booldatum.h"

// Includes from spatial:
#include "grid_layer.h"
#include "grid_mask.h"

namespace nest
{

template < int D >
std::shared_ptr< Ntree< D, index > > Layer< D >::cached_ntree_;

template < int D >
std::vector< std::pair< Position< D >, index > >* Layer< D >::cached_vector_ = 0;

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
Layer< D >::get_status( DictionaryDatum& d ) const
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
  assert( target_abs != 0 );

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
std::shared_ptr< Ntree< D, index > >
Layer< D >::get_global_positions_ntree( NodeCollectionPTR node_collection )
{
  if ( cached_ntree_md_ == node_collection->get_metadata() )
  {
    assert( cached_ntree_.get() );
    return cached_ntree_;
  }

  clear_ntree_cache_();

  cached_ntree_ =
    std::shared_ptr< Ntree< D, index > >( new Ntree< D, index >( this->lower_left_, this->extent_, this->periodic_ ) );

  return do_get_global_positions_ntree_( node_collection );
}

template < int D >
std::shared_ptr< Ntree< D, index > >
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

  cached_ntree_ = std::shared_ptr< Ntree< D, index > >( new Ntree< D, index >( this->lower_left_, extent, periodic ) );

  do_get_global_positions_ntree_( node_collection );

  // Do not use cache since the periodic bits and extents were altered.
  cached_ntree_md_ = NodeCollectionMetadataPTR( 0 );

  return cached_ntree_;
}

template < int D >
std::shared_ptr< Ntree< D, index > >
Layer< D >::do_get_global_positions_ntree_( NodeCollectionPTR node_collection )
{
  if ( cached_vector_md_ == node_collection->get_metadata() )
  {
    // Convert from vector to Ntree

    typename std::insert_iterator< Ntree< D, index > > to = std::inserter( *cached_ntree_, cached_ntree_->end() );

    for ( typename std::vector< std::pair< Position< D >, index > >::iterator from = cached_vector_->begin();
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
std::vector< std::pair< Position< D >, index > >*
Layer< D >::get_global_positions_vector( NodeCollectionPTR node_collection )
{
  if ( cached_vector_md_ == node_collection->get_metadata() )
  {
    assert( cached_vector_ );
    return cached_vector_;
  }

  clear_vector_cache_();

  cached_vector_ = new std::vector< std::pair< Position< D >, index > >;

  if ( cached_ntree_md_ == node_collection->get_metadata() )
  {
    // Convert from NTree to vector

    typename std::back_insert_iterator< std::vector< std::pair< Position< D >, index > > > to =
      std::back_inserter( *cached_vector_ );

    for ( typename Ntree< D, index >::iterator from = cached_ntree_->begin(); from != cached_ntree_->end(); ++from )
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
std::vector< std::pair< Position< D >, index > >
Layer< D >::get_global_positions_vector( const MaskDatum& mask,
  const Position< D >& anchor,
  bool allow_oversized,
  NodeCollectionPTR node_collection )
{
  MaskedLayer< D > masked_layer( *this, mask, allow_oversized, node_collection );
  std::vector< std::pair< Position< D >, index > > positions;

  for ( typename Ntree< D, index >::masked_iterator iter = masked_layer.begin( anchor ); iter != masked_layer.end();
        ++iter )
  {
    positions.push_back( *iter );
  }

  return positions;
}

template < int D >
std::vector< index >
Layer< D >::get_global_nodes( const MaskDatum& mask,
  const std::vector< double >& anchor,
  bool allow_oversized,
  NodeCollectionPTR node_collection )
{
  MaskedLayer< D > masked_layer( *this, mask, allow_oversized, node_collection );
  std::vector< index > nodes;
  for ( typename Ntree< D, index >::masked_iterator i = masked_layer.begin( anchor ); i != masked_layer.end(); ++i )
  {
    nodes.push_back( i->second );
  }
  return nodes;
}

template < int D >
void
Layer< D >::dump_nodes( std::ostream& out ) const
{
  for ( NodeCollection::const_iterator it = this->node_collection_->MPI_local_begin();
        it < this->node_collection_->end();
        ++it )
  {
    out << ( *it ).node_id << ' ';
    get_position( ( *it ).lid ).print( out );
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
  std::vector< std::pair< Position< D >, index > >* src_vec = get_global_positions_vector( node_collection );

  // Dictionary with parameters for get_connections()
  DictionaryDatum ncdict( new Dictionary );
  def( ncdict, names::synapse_model, syn_model );

  // Avoid setting up new array for each iteration of the loop
  std::vector< index > source_array( 1 );

  for ( typename std::vector< std::pair< Position< D >, index > >::iterator src_iter = src_vec->begin();
        src_iter != src_vec->end();
        ++src_iter )
  {

    const index source_node_id = src_iter->second;
    const Position< D > source_pos = src_iter->first;

    source_array[ 0 ] = source_node_id;
    def( ncdict, names::source, NodeCollectionDatum( NodeCollection::create( source_array ) ) );
    ArrayDatum connectome = kernel().connection_manager.get_connections( ncdict );

    // Print information about all local connections for current source
    for ( size_t i = 0; i < connectome.size(); ++i )
    {
      ConnectionDatum con_id = getValue< ConnectionDatum >( connectome.get( i ) );
      DictionaryDatum result_dict = kernel().connection_manager.get_synapse_status( con_id.get_source_node_id(),
        con_id.get_target_node_id(),
        con_id.get_target_thread(),
        con_id.get_synapse_model_id(),
        con_id.get_port() );

      long target_node_id = getValue< long >( result_dict, names::target );
      double weight = getValue< double >( result_dict, names::weight );
      double delay = getValue< double >( result_dict, names::delay );

      // Print source, target, weight, delay, rports
      out << source_node_id << ' ' << target_node_id << ' ' << weight << ' ' << delay;

      Layer< D >* tgt_layer = dynamic_cast< Layer< D >* >( target_layer.get() );

      out << ' ';
      const index tnode_id = tgt_layer->node_collection_->find( target_node_id );
      tgt_layer->compute_displacement( source_pos, tnode_id ).print( out );
      out << '\n';
    }
  }
}

template < int D >
void
MaskedLayer< D >::check_mask_( Layer< D >& layer, bool allow_oversized )
{
  if ( not mask_.get() )
  {
    mask_ = new AllMask< D >();
    return;
  }

  try // Try to cast to GridMask
  {
    const GridMask< D >& grid_mask = dynamic_cast< const GridMask< D >& >( *mask_ );

    // If the above cast succeeds, then this is a grid mask

    GridLayer< D >* grid_layer = dynamic_cast< GridLayer< D >* >( &layer );
    if ( grid_layer == 0 )
    {
      throw BadProperty( "Grid masks can only be used with grid layers." );
    }

    Position< D > ext = grid_layer->get_extent();
    Position< D, index > dims = grid_layer->get_dims();

    if ( not allow_oversized )
    {
      bool oversize = false;
      for ( int i = 0; i < D; ++i )
      {
        oversize |= layer.get_periodic_mask()[ i ]
          and ( grid_mask.get_lower_right()[ i ] - grid_mask.get_upper_left()[ i ] ) > ( int ) dims[ i ];
      }
      if ( oversize )
      {
        throw BadProperty(
          "Mask size must not exceed layer size; set allow_oversized_mask to "
          "override." );
      }
    }

    Position< D > lower_left = ext / dims * grid_mask.get_upper_left() - ext / dims * 0.5;
    Position< D > upper_right = ext / dims * grid_mask.get_lower_right() - ext / dims * 0.5;

    double y = lower_left[ 1 ];
    lower_left[ 1 ] = -upper_right[ 1 ];
    upper_right[ 1 ] = -y;

    mask_ = new BoxMask< D >( lower_left, upper_right );
  }
  catch ( std::bad_cast& )
  {

    // Not a grid mask

    try // Try to cast to correct dimension Mask
    {
      const Mask< D >& mask = dynamic_cast< const Mask< D >& >( *mask_ );

      if ( not allow_oversized )
      {
        const Box< D > bb = mask.get_bbox();
        bool oversize = false;
        for ( int i = 0; i < D; ++i )
        {
          oversize |=
            layer.get_periodic_mask()[ i ] and ( bb.upper_right[ i ] - bb.lower_left[ i ] ) > layer.get_extent()[ i ];
        }
        if ( oversize )
        {
          throw BadProperty(
            "Mask size must not exceed layer size; set allow_oversized_mask to "
            "override." );
        }
      }
    }
    catch ( std::bad_cast& )
    {
      throw BadProperty( "Mask is incompatible with layer." );
    }
  }
}

} // namespace nest

#endif
