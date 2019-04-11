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
#include "nest_datums.h"

// Includes from topology:
#include "grid_layer.h"
#include "grid_mask.h"

namespace nest {

template < int D >
lockPTR< Ntree< D, index > > Layer< D >::cached_ntree_;

template < int D >
std::vector< std::pair< Position< D >, index > >* Layer< D >::cached_vector_ = 0;

template < int D >
Selector Layer< D >::cached_selector_;

template < int D >
Position< D >
Layer< D >::compute_displacement( const Position< D >& from_pos, const Position< D >& to_pos ) const
{
  Position< D > displ = to_pos - from_pos;
  for ( int i = 0; i < D; ++i ) {
    if ( periodic_[ i ] ) {
      displ[ i ] = -0.5 * extent_[ i ] + std::fmod( displ[ i ] + 0.5 * extent_[ i ], extent_[ i ] );
      if ( displ[ i ] < -0.5 * extent_[ i ] ) {
        displ[ i ] += extent_[ i ];
      }
    }
  }
  return displ;
}

template < int D >
void
Layer< D >::set_status( const DictionaryDatum& d )
{
  if ( d->known( names::extent ) ) {
    Position< D > center = get_center();
    extent_ = getValue< std::vector< double > >( d, names::extent );
    lower_left_ = center - extent_ / 2;
  }
  if ( d->known( names::center ) ) {
    lower_left_ = getValue< std::vector< double > >( d, names::center );
    lower_left_ -= extent_ / 2;
  }
  if ( d->known( names::edge_wrap ) ) {
    if ( getValue< bool >( d, names::edge_wrap ) ) {
      periodic_ = ( 1 << D ) - 1; // All dimensions periodic
    }
  }

  Subnet::set_status( d );
}

template < int D >
void
Layer< D >::get_status( DictionaryDatum& d ) const
{
  Subnet::get_status( d );

  DictionaryDatum topology_dict( new Dictionary );

  ( *topology_dict )[ names::depth ] = depth_;
  ( *topology_dict )[ names::extent ] = std::vector< double >( extent_ );
  ( *topology_dict )[ names::center ] = std::vector< double >( lower_left_ + extent_ / 2 );

  if ( periodic_.none() ) {
    ( *topology_dict )[ names::edge_wrap ] = BoolDatum( false );
  }
  else if ( periodic_.count() == D ) {
    ( *topology_dict )[ names::edge_wrap ] = true;
  }
  ( *d )[ names::topology ] = topology_dict;
}

template < int D >
void
Layer< D >::connect( AbstractLayer& target_layer, ConnectionCreator& connector )
{
  try {
    Layer< D >& tgt = dynamic_cast< Layer< D >& >( target_layer );
    connector.connect( *this, tgt );
  }
  catch ( std::bad_cast& e ) {
    throw BadProperty( "Target layer must have same number of dimensions as source layer." );
  }
}

template < int D >
lockPTR< Ntree< D, index > >
Layer< D >::get_local_positions_ntree( Selector filter )
{
  lockPTR< Ntree< D, index > > ntree( new Ntree< D, index >( this->lower_left_, this->extent_, this->periodic_ ) );

  insert_local_positions_ntree_( *ntree, filter );

  return ntree;
}

template < int D >
lockPTR< Ntree< D, index > >
Layer< D >::get_global_positions_ntree( Selector filter )
{
  if ( ( cached_ntree_layer_ == get_gid() ) and ( cached_selector_ == filter ) ) {
    assert( cached_ntree_.valid() );
    return cached_ntree_;
  }

  clear_ntree_cache_();

  cached_ntree_ =
    lockPTR< Ntree< D, index > >( new Ntree< D, index >( this->lower_left_, this->extent_, this->periodic_ ) );

  return do_get_global_positions_ntree_( filter );
}

template < int D >
lockPTR< Ntree< D, index > >
Layer< D >::get_global_positions_ntree( Selector filter,
  std::bitset< D > periodic,
  Position< D > lower_left,
  Position< D > extent )
{
  clear_ntree_cache_();
  clear_vector_cache_();

  // Keep layer geometry for non-periodic dimensions
  for ( int i = 0; i < D; ++i ) {
    if ( not periodic[ i ] ) {
      extent[ i ] = extent_[ i ];
      lower_left[ i ] = lower_left_[ i ];
    }
  }

  cached_ntree_ = lockPTR< Ntree< D, index > >( new Ntree< D, index >( this->lower_left_, extent, periodic ) );

  do_get_global_positions_ntree_( filter );

  // Do not use cache since the periodic bits and extents were altered.
  cached_ntree_layer_ = -1;

  return cached_ntree_;
}

template < int D >
lockPTR< Ntree< D, index > >
Layer< D >::do_get_global_positions_ntree_( const Selector& filter )
{
  if ( ( cached_vector_layer_ == get_gid() ) and ( cached_selector_ == filter ) ) {
    // Convert from vector to Ntree

    typename std::insert_iterator< Ntree< D, index > > to = std::inserter( *cached_ntree_, cached_ntree_->end() );

    for ( typename std::vector< std::pair< Position< D >, index > >::iterator from = cached_vector_->begin();
          from != cached_vector_->end();
          ++from ) {
      *to = *from;
    }
  }
  else {

    insert_global_positions_ntree_( *cached_ntree_, filter );
  }

  clear_vector_cache_();

  cached_ntree_layer_ = get_gid();
  cached_selector_ = filter;

  return cached_ntree_;
}

template < int D >
std::vector< std::pair< Position< D >, index > >*
Layer< D >::get_global_positions_vector( Selector filter )
{
  if ( ( cached_vector_layer_ == get_gid() ) and ( cached_selector_ == filter ) ) {
    assert( cached_vector_ );
    return cached_vector_;
  }

  clear_vector_cache_();

  cached_vector_ = new std::vector< std::pair< Position< D >, index > >;

  if ( ( cached_ntree_layer_ == get_gid() ) and ( cached_selector_ == filter ) ) {
    // Convert from NTree to vector

    typename std::back_insert_iterator< std::vector< std::pair< Position< D >, index > > > to =
      std::back_inserter( *cached_vector_ );

    for ( typename Ntree< D, index >::iterator from = cached_ntree_->begin(); from != cached_ntree_->end(); ++from ) {
      *to = *from;
    }
  }
  else {

    insert_global_positions_vector_( *cached_vector_, filter );
  }

  clear_ntree_cache_();

  cached_vector_layer_ = get_gid();
  cached_selector_ = filter;

  return cached_vector_;
}

template < int D >
std::vector< std::pair< Position< D >, index > >
Layer< D >::get_global_positions_vector( Selector filter,
  const MaskDatum& mask,
  const Position< D >& anchor,
  bool allow_oversized )
{
  MaskedLayer< D > masked_layer( *this, filter, mask, true, allow_oversized );
  std::vector< std::pair< Position< D >, index > > positions;

  for ( typename Ntree< D, index >::masked_iterator iter = masked_layer.begin( anchor ); iter != masked_layer.end();
        ++iter ) {
    positions.push_back( *iter );
  }

  return positions;
}

template < int D >
std::vector< index >
Layer< D >::get_global_nodes( const MaskDatum& mask, const std::vector< double >& anchor, bool allow_oversized )
{
  MaskedLayer< D > masked_layer( *this, Selector(), mask, true, allow_oversized );
  std::vector< index > nodes;
  for ( typename Ntree< D, index >::masked_iterator i = masked_layer.begin( anchor ); i != masked_layer.end(); ++i ) {
    nodes.push_back( i->second );
  }
  return nodes;
}

template < int D >
void
Layer< D >::dump_nodes( std::ostream& out ) const
{
  for ( index i = 0; i < nodes_.size(); ++i ) {
    const index gid = nodes_[ i ]->get_gid();
    out << gid << ' ';
    get_position( i ).print( out );
    out << std::endl;
  }
}

template < int D >
void
Layer< D >::dump_connections( std::ostream& out, const Token& syn_model )
{
  std::vector< std::pair< Position< D >, index > >* src_vec = get_global_positions_vector();

  // Dictionary with parameters for get_connections()
  DictionaryDatum gcdict( new Dictionary );
  def( gcdict, names::synapse_model, syn_model );

  // Avoid setting up new array for each iteration of the loop
  std::vector< index > source_array( 1 );

  for ( typename std::vector< std::pair< Position< D >, index > >::iterator src_iter = src_vec->begin();
        src_iter != src_vec->end();
        ++src_iter ) {

    const index source_gid = src_iter->second;
    const Position< D > source_pos = src_iter->first;

    source_array[ 0 ] = source_gid;
    def( gcdict, names::source, source_array );
    ArrayDatum connectome = kernel().connection_manager.get_connections( gcdict );

    // Print information about all local connections for current source
    for ( size_t i = 0; i < connectome.size(); ++i ) {
      ConnectionDatum con_id = getValue< ConnectionDatum >( connectome.get( i ) );
      DictionaryDatum result_dict = kernel().connection_manager.get_synapse_status( con_id.get_source_gid(),
        con_id.get_target_gid(),
        con_id.get_target_thread(),
        con_id.get_synapse_model_id(),
        con_id.get_port() );

      long target_gid = getValue< long >( result_dict, names::target );
      double weight = getValue< double >( result_dict, names::weight );
      double delay = getValue< double >( result_dict, names::delay );

      Node const* const target = kernel().node_manager.get_node( target_gid );
      assert( target );

      // Print source, target, weight, delay, rports
      out << source_gid << ' ' << target_gid << ' ' << weight << ' ' << delay;

      Layer< D >* tgt_layer = dynamic_cast< Layer< D >* >( target->get_parent() );
      if ( tgt_layer == 0 ) {

        // Happens if target does not belong to layer, eg spike_detector.
        // We then print NaNs for the displacement.
        for ( int n = 0; n < D; ++n ) {
          out << " NaN";
        }
      }
      else {

        out << ' ';
        tgt_layer->compute_displacement( source_pos, target->get_subnet_index() ).print( out );
      }

      out << '\n';
    }
  }
}

template < int D >
void
MaskedLayer< D >::check_mask_( Layer< D >& layer, bool allow_oversized )
{
  if ( not mask_.valid() ) {
    mask_ = new AllMask< D >();
  }

  try // Try to cast to GridMask
  {
    const GridMask< D >& grid_mask = dynamic_cast< const GridMask< D >& >( *mask_ );

    // If the above cast succeeds, then this is a grid mask

    GridLayer< D >* grid_layer = dynamic_cast< GridLayer< D >* >( &layer );
    if ( grid_layer == 0 ) {
      throw BadProperty( "Grid masks can only be used with grid layers." );
    }

    Position< D > ext = grid_layer->get_extent();
    Position< D, index > dims = grid_layer->get_dims();

    if ( not allow_oversized ) {
      bool oversize = false;
      for ( int i = 0; i < D; ++i ) {
        oversize |= layer.get_periodic_mask()[ i ]
          and ( grid_mask.get_lower_right()[ i ] - grid_mask.get_upper_left()[ i ] ) > ( int ) dims[ i ];
      }
      if ( oversize ) {
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
  catch ( std::bad_cast& ) {

    // Not a grid mask

    try // Try to cast to correct dimension Mask
    {
      const Mask< D >& mask = dynamic_cast< const Mask< D >& >( *mask_ );

      if ( not allow_oversized ) {
        const Box< D > bb = mask.get_bbox();
        bool oversize = false;
        for ( int i = 0; i < D; ++i ) {
          oversize |=
            layer.get_periodic_mask()[ i ] and ( bb.upper_right[ i ] - bb.lower_left[ i ] ) > layer.get_extent()[ i ];
        }
        if ( oversize ) {
          throw BadProperty(
            "Mask size must not exceed layer size; set allow_oversized_mask to "
            "override." );
        }
      }
    }
    catch ( std::bad_cast& ) {
      throw BadProperty( "Mask is incompatible with layer." );
    }
  }
}

} // namespace nest

#endif
