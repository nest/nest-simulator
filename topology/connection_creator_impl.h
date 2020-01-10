/*
 *  connection_creator_impl.h
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

#ifndef CONNECTION_CREATOR_IMPL_H
#define CONNECTION_CREATOR_IMPL_H

#include "connection_creator.h"

// C++ includes:
#include <vector>

// Includes from librandom:
#include "binomial_randomdev.h"

// Includes from nestkernel:
#include "kernel_manager.h"
#include "nest.h"

namespace nest
{
template < int D >
void
ConnectionCreator::connect( Layer< D >& source, Layer< D >& target )
{
  switch ( type_ )
  {
  case Target_driven:

    target_driven_connect_( source, target );
    break;

  case Convergent:

    convergent_connect_( source, target );
    break;

  case Divergent:

    divergent_connect_( source, target );
    break;

  case Source_driven:

    source_driven_connect_( source, target );
    break;

  default:
    throw BadProperty( "Unknown connection type." );
  }
}

template < int D >
void
ConnectionCreator::get_parameters_( const Position< D >& pos, librandom::RngPtr rng, double& weight, double& delay )
{
  // keeping this function temporarily until all connection variants are cleaned
  // up
  weight = weight_->value( pos, rng );
  delay = delay_->value( pos, rng );
}

template < typename Iterator, int D >
void
ConnectionCreator::connect_to_target_( Iterator from,
  Iterator to,
  Node* tgt_ptr,
  const Position< D >& tgt_pos,
  thread tgt_thread,
  const Layer< D >& source )
{
  librandom::RngPtr rng = get_vp_rng( tgt_thread );

  const bool without_kernel = not kernel_.valid();
  for ( Iterator iter = from; iter != to; ++iter )
  {
    if ( ( not allow_autapses_ ) and ( iter->second == tgt_ptr->get_gid() ) )
    {
      continue;
    }

    if ( without_kernel or rng->drand() < kernel_->value( source.compute_displacement( tgt_pos, iter->first ), rng ) )
    {
      const Position< D > disp = source.compute_displacement( tgt_pos, iter->first );
      connect_(
        iter->second, tgt_ptr, tgt_thread, weight_->value( disp, rng ), delay_->value( disp, rng ), synapse_model_ );
    }
  }
}

template < int D >
ConnectionCreator::PoolWrapper_< D >::PoolWrapper_()
  : masked_layer_( 0 )
  , positions_( 0 )
{
}

template < int D >
ConnectionCreator::PoolWrapper_< D >::~PoolWrapper_()
{
  if ( masked_layer_ )
  {
    delete masked_layer_;
  }
}

template < int D >
void
ConnectionCreator::PoolWrapper_< D >::define( MaskedLayer< D >* ml )
{
  assert( masked_layer_ == 0 );
  assert( positions_ == 0 );
  assert( ml != 0 );
  masked_layer_ = ml;
}

template < int D >
void
ConnectionCreator::PoolWrapper_< D >::define( std::vector< std::pair< Position< D >, index > >* pos )
{
  assert( masked_layer_ == 0 );
  assert( positions_ == 0 );
  assert( pos != 0 );
  positions_ = pos;
}

template < int D >
typename Ntree< D, index >::masked_iterator
ConnectionCreator::PoolWrapper_< D >::masked_begin( const Position< D >& pos ) const
{
  return masked_layer_->begin( pos );
}

template < int D >
typename Ntree< D, index >::masked_iterator
ConnectionCreator::PoolWrapper_< D >::masked_end() const
{
  return masked_layer_->end();
}

template < int D >
typename std::vector< std::pair< Position< D >, index > >::iterator
ConnectionCreator::PoolWrapper_< D >::begin() const
{
  return positions_->begin();
}

template < int D >
typename std::vector< std::pair< Position< D >, index > >::iterator
ConnectionCreator::PoolWrapper_< D >::end() const
{
  return positions_->end();
}


template < int D >
void
ConnectionCreator::target_driven_connect_( Layer< D >& source, Layer< D >& target )
{
  // Target driven connect
  // For each local target node:
  //  1. Apply Mask to source layer
  //  2. For each source node: Compute probability, draw random number, make
  //     connection conditionally

  // Nodes in the subnet are grouped by depth, so to select by depth, we
  // just adjust the begin and end pointers:
  std::vector< Node* >::const_iterator target_begin;
  std::vector< Node* >::const_iterator target_end;
  if ( target_filter_.select_depth() )
  {
    target_begin = target.local_begin( target_filter_.depth );
    target_end = target.local_end( target_filter_.depth );
  }
  else
  {
    target_begin = target.local_begin();
    target_end = target.local_end();
  }

  // retrieve global positions, either for masked or unmasked pool
  PoolWrapper_< D > pool;
  if ( mask_.valid() ) // MaskedLayer will be freed by PoolWrapper d'tor
  {
    pool.define( new MaskedLayer< D >( source, source_filter_, mask_, true, allow_oversized_ ) );
  }
  else
  {
    pool.define( source.get_global_positions_vector( source_filter_ ) );
  }

// sharing specs on next line commented out because gcc 4.2 cannot handle them
#pragma omp parallel // default(none) shared(source, target, masked_layer,
                     // target_begin, target_end)
  {
    const int thread_id = kernel().vp_manager.get_thread_id();

    for ( std::vector< Node* >::const_iterator tgt_it = target_begin; tgt_it != target_end; ++tgt_it )
    {
      Node* const tgt = kernel().node_manager.get_node( ( *tgt_it )->get_gid(), thread_id );
      const thread target_thread = tgt->get_thread();

      // check whether the target is on our thread
      if ( thread_id != target_thread )
      {
        continue;
      }

      if ( target_filter_.select_model() && ( tgt->get_model_id() != target_filter_.model ) )
      {
        continue;
      }

      const Position< D > target_pos = target.get_position( tgt->get_subnet_index() );

      if ( mask_.valid() )
      {
        connect_to_target_( pool.masked_begin( target_pos ), pool.masked_end(), tgt, target_pos, thread_id, source );
      }
      else
      {
        connect_to_target_( pool.begin(), pool.end(), tgt, target_pos, thread_id, source );
      }
    } // for target_begin
  }   // omp parallel
}


template < int D >
void
ConnectionCreator::source_driven_connect_( Layer< D >& source, Layer< D >& target )
{
  // Source driven connect is actually implemented as target driven,
  // but with displacements computed in the target layer. The Mask has been
  // reversed so that it can be applied to the source instead of the target.
  // For each local target node:
  //  1. Apply (Converse)Mask to source layer
  //  2. For each source node: Compute probability, draw random number, make
  //     connection conditionally

  // Nodes in the subnet are grouped by depth, so to select by depth, we
  // just adjust the begin and end pointers:
  std::vector< Node* >::const_iterator target_begin;
  std::vector< Node* >::const_iterator target_end;
  if ( target_filter_.select_depth() )
  {
    target_begin = target.local_begin( target_filter_.depth );
    target_end = target.local_end( target_filter_.depth );
  }
  else
  {
    target_begin = target.local_begin();
    target_end = target.local_end();
  }

  // protect against connecting to devices without proxies
  // we need to do this before creating the first connection to leave
  // the network untouched if any target does not have proxies
  for ( std::vector< Node* >::const_iterator tgt_it = target_begin; tgt_it != target_end; ++tgt_it )
  {
    if ( not( *tgt_it )->has_proxies() )
    {
      throw IllegalConnection(
        "Topology Divergent connections"
        " to devices are not possible." );
    }
  }

  if ( mask_.valid() )
  {

    // By supplying the target layer to the MaskedLayer constructor, the
    // mask is mirrored so it may be applied to the source layer instead
    MaskedLayer< D > masked_layer( source, source_filter_, mask_, true, allow_oversized_, target );

    for ( std::vector< Node* >::const_iterator tgt_it = target_begin; tgt_it != target_end; ++tgt_it )
    {

      if ( target_filter_.select_model() && ( ( *tgt_it )->get_model_id() != target_filter_.model ) )
      {
        continue;
      }

      index target_id = ( *tgt_it )->get_gid();
      thread target_thread = ( *tgt_it )->get_thread();
      librandom::RngPtr rng = get_vp_rng( target_thread );
      Position< D > target_pos = target.get_position( ( *tgt_it )->get_subnet_index() );

      // If there is a kernel, we create connections conditionally,
      // otherwise all sources within the mask are created. Test moved
      // outside the loop for efficiency.
      if ( kernel_.valid() )
      {

        for ( typename Ntree< D, index >::masked_iterator iter = masked_layer.begin( target_pos );
              iter != masked_layer.end();
              ++iter )
        {

          if ( ( not allow_autapses_ ) and ( iter->second == target_id ) )
          {
            continue;
          }

          if ( rng->drand() < kernel_->value( target.compute_displacement( iter->first, target_pos ), rng ) )
          {
            double w, d;
            get_parameters_( target.compute_displacement( iter->first, target_pos ), rng, w, d );
            kernel().connection_manager.connect(
              iter->second, *tgt_it, target_thread, synapse_model_, dummy_param_dicts_[ target_thread ], d, w );
          }
        }
      }
      else
      {

        // no kernel

        for ( typename Ntree< D, index >::masked_iterator iter = masked_layer.begin( target_pos );
              iter != masked_layer.end();
              ++iter )
        {

          if ( ( not allow_autapses_ ) and ( iter->second == target_id ) )
          {
            continue;
          }
          double w, d;
          get_parameters_( target.compute_displacement( iter->first, target_pos ), rng, w, d );
          kernel().connection_manager.connect(
            iter->second, *tgt_it, target_thread, synapse_model_, dummy_param_dicts_[ target_thread ], d, w );
        }
      }
    }
  }
  else
  {
    // no mask

    std::vector< std::pair< Position< D >, index > >* positions = source.get_global_positions_vector( source_filter_ );
    for ( std::vector< Node* >::const_iterator tgt_it = target_begin; tgt_it != target_end; ++tgt_it )
    {

      if ( target_filter_.select_model() && ( ( *tgt_it )->get_model_id() != target_filter_.model ) )
      {
        continue;
      }

      index target_id = ( *tgt_it )->get_gid();
      thread target_thread = ( *tgt_it )->get_thread();
      librandom::RngPtr rng = get_vp_rng( target_thread );
      Position< D > target_pos = target.get_position( ( *tgt_it )->get_subnet_index() );

      // If there is a kernel, we create connections conditionally,
      // otherwise all sources within the mask are created. Test moved
      // outside the loop for efficiency.
      if ( kernel_.valid() )
      {

        for ( typename std::vector< std::pair< Position< D >, index > >::iterator iter = positions->begin();
              iter != positions->end();
              ++iter )
        {

          if ( ( not allow_autapses_ ) and ( iter->second == target_id ) )
          {
            continue;
          }

          if ( rng->drand() < kernel_->value( target.compute_displacement( iter->first, target_pos ), rng ) )
          {
            double w, d;
            get_parameters_( target.compute_displacement( iter->first, target_pos ), rng, w, d );
            kernel().connection_manager.connect(
              iter->second, *tgt_it, target_thread, synapse_model_, dummy_param_dicts_[ target_thread ], d, w );
          }
        }
      }
      else
      {

        for ( typename std::vector< std::pair< Position< D >, index > >::iterator iter = positions->begin();
              iter != positions->end();
              ++iter )
        {

          if ( ( not allow_autapses_ ) and ( iter->second == target_id ) )
          {
            continue;
          }

          double w, d;
          get_parameters_( target.compute_displacement( iter->first, target_pos ), rng, w, d );
          kernel().connection_manager.connect(
            iter->second, *tgt_it, target_thread, synapse_model_, dummy_param_dicts_[ target_thread ], d, w );
        }
      }
    }
  }
}

template < int D >
void
ConnectionCreator::convergent_connect_( Layer< D >& source, Layer< D >& target )
{
  if ( number_of_connections_ < 1 )
  {
    return;
  }

  // Convergent connections (fixed fan in)
  //
  // For each local target node:
  // 1. Apply Mask to source layer
  // 2. Compute connection probability for each source position
  // 3. Draw source nodes and make connections


  // Nodes in the subnet are grouped by depth, so to select by depth, we
  // just adjust the begin and end pointers:
  std::vector< Node* >::const_iterator target_begin;
  std::vector< Node* >::const_iterator target_end;
  if ( target_filter_.select_depth() )
  {
    target_begin = target.local_begin( target_filter_.depth );
    target_end = target.local_end( target_filter_.depth );
  }
  else
  {
    target_begin = target.local_begin();
    target_end = target.local_end();
  }

  // protect against connecting to devices without proxies
  // we need to do this before creating the first connection to leave
  // the network untouched if any target does not have proxies
  for ( std::vector< Node* >::const_iterator tgt_it = target_begin; tgt_it != target_end; ++tgt_it )
  {
    if ( not( *tgt_it )->has_proxies() )
    {
      throw IllegalConnection(
        "Topology Divergent connections"
        " to devices are not possible." );
    }
  }

  if ( mask_.valid() )
  {
    MaskedLayer< D > masked_source( source, source_filter_, mask_, true, allow_oversized_ );

    for ( std::vector< Node* >::const_iterator tgt_it = target_begin; tgt_it != target_end; ++tgt_it )
    {

      if ( target_filter_.select_model() && ( ( *tgt_it )->get_model_id() != target_filter_.model ) )
      {
        continue;
      }

      index target_id = ( *tgt_it )->get_gid();
      thread target_thread = ( *tgt_it )->get_thread();
      librandom::RngPtr rng = get_vp_rng( target_thread );
      Position< D > target_pos = target.get_position( ( *tgt_it )->get_subnet_index() );

      // Get (position,GID) pairs for sources inside mask
      const Position< D > anchor = target.get_position( ( *tgt_it )->get_subnet_index() );
      std::vector< std::pair< Position< D >, index > > positions;
      for ( typename Ntree< D, index >::masked_iterator iter = masked_source.begin( anchor );
            iter != masked_source.end();
            ++iter )
      {
        positions.push_back( *iter );
      }

      // We will select `number_of_connections_` sources within the mask.
      // If there is no kernel, we can just draw uniform random numbers,
      // but with a kernel we have to set up a probability distribution
      // function using the Vose class.
      if ( kernel_.valid() )
      {

        std::vector< double > probabilities;

        // Collect probabilities for the sources
        for ( typename std::vector< std::pair< Position< D >, index > >::iterator iter = positions.begin();
              iter != positions.end();
              ++iter )
        {

          probabilities.push_back( kernel_->value( source.compute_displacement( target_pos, iter->first ), rng ) );
        }

        if ( positions.empty()
          or ( ( not allow_autapses_ ) and ( positions.size() == 1 ) and ( positions[ 0 ].second == target_id ) )
          or ( ( not allow_multapses_ ) and ( positions.size() < number_of_connections_ ) ) )
        {
          std::string msg = String::compose( "Global target ID %1: Not enough sources found inside mask", target_id );
          throw KernelException( msg.c_str() );
        }

        // A Vose object draws random integers with a non-uniform
        // distribution.
        Vose lottery( probabilities );

        // If multapses are not allowed, we must keep track of which
        // sources have been selected already.
        std::vector< bool > is_selected( positions.size() );

        // Draw `number_of_connections_` sources
        for ( int i = 0; i < ( int ) number_of_connections_; ++i )
        {
          index random_id = lottery.get_random_id( rng );
          if ( ( not allow_multapses_ ) and ( is_selected[ random_id ] ) )
          {
            --i;
            continue;
          }

          index source_id = positions[ random_id ].second;
          if ( ( not allow_autapses_ ) and ( source_id == target_id ) )
          {
            --i;
            continue;
          }
          double w, d;
          get_parameters_( source.compute_displacement( target_pos, positions[ random_id ].first ), rng, w, d );
          kernel().connection_manager.connect(
            source_id, *tgt_it, target_thread, synapse_model_, dummy_param_dicts_[ target_thread ], d, w );
          is_selected[ random_id ] = true;
        }
      }
      else
      {

        // no kernel

        if ( positions.empty()
          or ( ( not allow_autapses_ ) and ( positions.size() == 1 ) and ( positions[ 0 ].second == target_id ) )
          or ( ( not allow_multapses_ ) and ( positions.size() < number_of_connections_ ) ) )
        {
          std::string msg = String::compose( "Global target ID %1: Not enough sources found inside mask", target_id );
          throw KernelException( msg.c_str() );
        }

        // If multapses are not allowed, we must keep track of which
        // sources have been selected already.
        std::vector< bool > is_selected( positions.size() );

        // Draw `number_of_connections_` sources
        for ( int i = 0; i < ( int ) number_of_connections_; ++i )
        {
          index random_id = rng->ulrand( positions.size() );
          if ( ( not allow_multapses_ ) and ( is_selected[ random_id ] ) )
          {
            --i;
            continue;
          }
          index source_id = positions[ random_id ].second;
          double w, d;
          get_parameters_( source.compute_displacement( target_pos, positions[ random_id ].first ), rng, w, d );
          kernel().connection_manager.connect(
            source_id, *tgt_it, target_thread, synapse_model_, dummy_param_dicts_[ target_thread ], d, w );
          is_selected[ random_id ] = true;
        }
      }
    }
  }
  else
  {
    // no mask

    // Get (position,GID) pairs for all nodes in source layer
    std::vector< std::pair< Position< D >, index > >* positions = source.get_global_positions_vector( source_filter_ );

    for ( std::vector< Node* >::const_iterator tgt_it = target_begin; tgt_it != target_end; ++tgt_it )
    {

      if ( target_filter_.select_model() && ( ( *tgt_it )->get_model_id() != target_filter_.model ) )
      {
        continue;
      }

      index target_id = ( *tgt_it )->get_gid();
      thread target_thread = ( *tgt_it )->get_thread();
      librandom::RngPtr rng = get_vp_rng( target_thread );
      Position< D > target_pos = target.get_position( ( *tgt_it )->get_subnet_index() );

      if ( ( positions->size() == 0 )
        or ( ( not allow_autapses_ ) and ( positions->size() == 1 ) and ( ( *positions )[ 0 ].second == target_id ) )
        or ( ( not allow_multapses_ ) and ( positions->size() < number_of_connections_ ) ) )
      {
        std::string msg = String::compose( "Global target ID %1: Not enough sources found", target_id );
        throw KernelException( msg.c_str() );
      }

      // We will select `number_of_connections_` sources within the mask.
      // If there is no kernel, we can just draw uniform random numbers,
      // but with a kernel we have to set up a probability distribution
      // function using the Vose class.
      if ( kernel_.valid() )
      {

        std::vector< double > probabilities;

        // Collect probabilities for the sources
        for ( typename std::vector< std::pair< Position< D >, index > >::iterator iter = positions->begin();
              iter != positions->end();
              ++iter )
        {
          probabilities.push_back( kernel_->value( source.compute_displacement( target_pos, iter->first ), rng ) );
        }

        // A Vose object draws random integers with a non-uniform
        // distribution.
        Vose lottery( probabilities );

        // If multapses are not allowed, we must keep track of which
        // sources have been selected already.
        std::vector< bool > is_selected( positions->size() );

        // Draw `number_of_connections_` sources
        for ( int i = 0; i < ( int ) number_of_connections_; ++i )
        {
          index random_id = lottery.get_random_id( rng );
          if ( ( not allow_multapses_ ) and ( is_selected[ random_id ] ) )
          {
            --i;
            continue;
          }

          index source_id = ( *positions )[ random_id ].second;
          if ( ( not allow_autapses_ ) and ( source_id == target_id ) )
          {
            --i;
            continue;
          }

          Position< D > source_pos = ( *positions )[ random_id ].first;
          double w, d;
          get_parameters_( source.compute_displacement( target_pos, source_pos ), rng, w, d );
          kernel().connection_manager.connect(
            source_id, *tgt_it, target_thread, synapse_model_, dummy_param_dicts_[ target_thread ], d, w );
          is_selected[ random_id ] = true;
        }
      }
      else
      {

        // no kernel

        // If multapses are not allowed, we must keep track of which
        // sources have been selected already.
        std::vector< bool > is_selected( positions->size() );

        // Draw `number_of_connections_` sources
        for ( int i = 0; i < ( int ) number_of_connections_; ++i )
        {
          index random_id = rng->ulrand( positions->size() );
          if ( ( not allow_multapses_ ) and ( is_selected[ random_id ] ) )
          {
            --i;
            continue;
          }

          index source_id = ( *positions )[ random_id ].second;
          if ( ( not allow_autapses_ ) and ( source_id == target_id ) )
          {
            --i;
            continue;
          }

          Position< D > source_pos = ( *positions )[ random_id ].first;
          double w, d;
          get_parameters_( source.compute_displacement( target_pos, source_pos ), rng, w, d );
          kernel().connection_manager.connect(
            source_id, *tgt_it, target_thread, synapse_model_, dummy_param_dicts_[ target_thread ], d, w );
          is_selected[ random_id ] = true;
        }
      }
    }
  }
}


template < int D >
void
ConnectionCreator::divergent_connect_( Layer< D >& source, Layer< D >& target )
{
  if ( number_of_connections_ < 1 )
  {
    return;
  }

  // protect against connecting to devices without proxies
  // we need to do this before creating the first connection to leave
  // the network untouched if any target does not have proxies
  // Nodes in the subnet are grouped by depth, so to select by depth, we
  // just adjust the begin and end pointers:
  std::vector< Node* >::const_iterator target_begin;
  std::vector< Node* >::const_iterator target_end;
  if ( target_filter_.select_depth() )
  {
    target_begin = target.local_begin( target_filter_.depth );
    target_end = target.local_end( target_filter_.depth );
  }
  else
  {
    target_begin = target.local_begin();
    target_end = target.local_end();
  }

  for ( std::vector< Node* >::const_iterator tgt_it = target_begin; tgt_it != target_end; ++tgt_it )
  {
    if ( not( *tgt_it )->has_proxies() )
    {
      throw IllegalConnection(
        "Topology Divergent connections"
        " to devices are not possible." );
    }
  }

  // Divergent connections (fixed fan out)
  //
  // For each (global) source: (All connections made on all mpi procs)
  // 1. Apply mask to global targets
  // 2. If using kernel: Compute connection probability for each global target
  // 3. Draw connections to make using global rng

  MaskedLayer< D > masked_target( target, target_filter_, mask_, true, allow_oversized_ );

  std::vector< std::pair< Position< D >, index > >* sources = source.get_global_positions_vector( source_filter_ );

  for ( typename std::vector< std::pair< Position< D >, index > >::iterator src_it = sources->begin();
        src_it != sources->end();
        ++src_it )
  {

    Position< D > source_pos = src_it->first;
    index source_id = src_it->second;
    std::vector< index > targets;
    std::vector< Position< D > > displacements;
    std::vector< double > probabilities;

    // Find potential targets and probabilities

    for ( typename Ntree< D, index >::masked_iterator tgt_it = masked_target.begin( source_pos );
          tgt_it != masked_target.end();
          ++tgt_it )
    {

      if ( ( not allow_autapses_ ) and ( source_id == tgt_it->second ) )
      {
        continue;
      }

      Position< D > target_displ = target.compute_displacement( source_pos, tgt_it->first );
      librandom::RngPtr rng = get_global_rng();

      targets.push_back( tgt_it->second );
      displacements.push_back( target_displ );

      if ( kernel_.valid() )
      {
        probabilities.push_back( kernel_->value( target_displ, rng ) );
      }
      else
      {
        probabilities.push_back( 1.0 );
      }
    }

    if ( targets.empty() or ( ( not allow_multapses_ ) and ( targets.size() < number_of_connections_ ) ) )
    {
      std::string msg = String::compose( "Global source ID %1: Not enough targets found", source_id );
      throw KernelException( msg.c_str() );
    }

    // Draw targets.  A Vose object draws random integers with a
    // non-uniform distribution.
    Vose lottery( probabilities );

    // If multapses are not allowed, we must keep track of which
    // targets have been selected already.
    std::vector< bool > is_selected( targets.size() );

    // Draw `number_of_connections_` targets
    for ( long i = 0; i < ( long ) number_of_connections_; ++i )
    {
      index random_id = lottery.get_random_id( get_global_rng() );
      if ( ( not allow_multapses_ ) and ( is_selected[ random_id ] ) )
      {
        --i;
        continue;
      }
      is_selected[ random_id ] = true;
      Position< D > target_displ = displacements[ random_id ];
      index target_id = targets[ random_id ];

      double w, d;
      get_parameters_( target_displ, get_global_rng(), w, d );

      // We bail out for non-local neurons only now after all possible
      // random numbers haven been drawn. Bailing out any earlier may lead
      // to desynchronized global rngs.
      if ( not kernel().node_manager.is_local_gid( target_id ) )
      {
        continue;
      }

      Node* target_ptr = kernel().node_manager.get_node( target_id );
      kernel().connection_manager.connect( source_id,
        target_ptr,
        target_ptr->get_thread(),
        synapse_model_,
        dummy_param_dicts_[ target_ptr->get_thread() ],
        d,
        w );
    }
  }
}

} // namespace nest

#endif
