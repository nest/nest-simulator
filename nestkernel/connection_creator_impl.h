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
ConnectionCreator::connect( Layer< D >& source,
  NodeCollectionPTR source_nc,
  Layer< D >& target,
  NodeCollectionPTR target_nc )
{
  switch ( type_ )
  {
  case Pairwise_bernoulli_on_source:

    pairwise_bernoulli_on_source_( source, source_nc, target, target_nc );
    break;

  case Fixed_indegree:

    fixed_indegree_( source, source_nc, target, target_nc );
    break;

  case Fixed_outdegree:

    fixed_outdegree_( source, source_nc, target, target_nc );
    break;

  case Pairwise_bernoulli_on_target:

    pairwise_bernoulli_on_target_( source, source_nc, target, target_nc );
    break;

  default:
    throw BadProperty( "Unknown connection type." );
  }
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

  // We create a source pos vector here that can be updated with the
  // source position. This is done to avoid creating and destroying
  // unnecessarily many vectors.
  std::vector< double > source_pos( D );
  const std::vector< double > target_pos = tgt_pos.get_vector();

  const bool without_kernel = not kernel_.get();
  for ( Iterator iter = from; iter != to; ++iter )
  {
    if ( ( not allow_autapses_ ) and ( iter->second == tgt_ptr->get_node_id() ) )
    {
      continue;
    }
    iter->first.get_vector( source_pos );

    if ( without_kernel or rng->drand() < kernel_->value( rng, source_pos, target_pos, source ) )
    {
      for ( size_t indx = 0; indx < synapse_model_.size(); ++indx )
      {
        kernel().connection_manager.connect( iter->second,
          tgt_ptr,
          tgt_thread,
          synapse_model_[ indx ],
          param_dicts_[ indx ][ tgt_thread ],
          delay_[ indx ]->value( rng, source_pos, target_pos, source ),
          weight_[ indx ]->value( rng, source_pos, target_pos, source ) );
      }
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
ConnectionCreator::pairwise_bernoulli_on_source_( Layer< D >& source,
  NodeCollectionPTR source_nc,
  Layer< D >& target,
  NodeCollectionPTR target_nc )
{
  // Connect using pairwise Bernoulli drawing source nodes (target driven)
  // For each local target node:
  //  1. Apply Mask to source layer
  //  2. For each source node: Compute probability, draw random number, make
  //     connection conditionally

  // retrieve global positions, either for masked or unmasked pool
  PoolWrapper_< D > pool;
  if ( mask_.get() ) // MaskedLayer will be freed by PoolWrapper d'tor
  {
    pool.define( new MaskedLayer< D >( source, mask_, allow_oversized_, source_nc ) );
  }
  else
  {
    pool.define( source.get_global_positions_vector( source_nc ) );
  }

  std::vector< std::shared_ptr< WrappedThreadException > > exceptions_raised_( kernel().vp_manager.get_num_threads() );

// sharing specs on next line commented out because gcc 4.2 cannot handle them
#pragma omp parallel // default(none) shared(source, target, masked_layer,
                     // target_begin, target_end)
  {
    const int thread_id = kernel().vp_manager.get_thread_id();
    try
    {
      NodeCollection::const_iterator target_begin = target_nc->begin();
      NodeCollection::const_iterator target_end = target_nc->end();

      for ( NodeCollection::const_iterator tgt_it = target_begin; tgt_it < target_end; ++tgt_it )
      {
        Node* const tgt = kernel().node_manager.get_node_or_proxy( ( *tgt_it ).node_id, thread_id );

        if ( not tgt->is_proxy() )
        {
          const Position< D > target_pos = target.get_position( ( *tgt_it ).lid );

          if ( mask_.get() )
          {
            connect_to_target_(
              pool.masked_begin( target_pos ), pool.masked_end(), tgt, target_pos, thread_id, source );
          }
          else
          {
            connect_to_target_( pool.begin(), pool.end(), tgt, target_pos, thread_id, source );
          }
        }
      } // for target_begin
    }
    catch ( std::exception& err )
    {
      // We must create a new exception here, err's lifetime ends at
      // the end of the catch block.
      exceptions_raised_.at( thread_id ) =
        std::shared_ptr< WrappedThreadException >( new WrappedThreadException( err ) );
    }
  } // omp parallel
  // check if any exceptions have been raised
  for ( thread thr = 0; thr < kernel().vp_manager.get_num_threads(); ++thr )
  {
    if ( exceptions_raised_.at( thr ).get() )
    {
      throw WrappedThreadException( *( exceptions_raised_.at( thr ) ) );
    }
  }
}


template < int D >
void
ConnectionCreator::pairwise_bernoulli_on_target_( Layer< D >& source,
  NodeCollectionPTR source_nc,
  Layer< D >& target,
  NodeCollectionPTR target_nc )
{
  // Connecting using pairwise Bernoulli drawing target nodes (source driven)
  // It is actually implemented as pairwise Bernoulli on source nodes,
  // but with displacements computed in the target layer. The Mask has been
  // reversed so that it can be applied to the source instead of the target.
  // For each local target node:
  //  1. Apply (Converse)Mask to source layer
  //  2. For each source node: Compute probability, draw random number, make
  //     connection conditionally

  PoolWrapper_< D > pool;
  if ( mask_.get() ) // MaskedLayer will be freed by PoolWrapper d'tor
  {
    // By supplying the target layer to the MaskedLayer constructor, the
    // mask is mirrored so it may be applied to the source layer instead
    pool.define( new MaskedLayer< D >( source, mask_, allow_oversized_, target, source_nc ) );
  }
  else
  {
    pool.define( source.get_global_positions_vector( source_nc ) );
  }

  std::vector< std::shared_ptr< WrappedThreadException > > exceptions_raised_( kernel().vp_manager.get_num_threads() );

  // We only need to check the first in the NodeCollection
  Node* const first_in_tgt = kernel().node_manager.get_node_or_proxy( target_nc->operator[]( 0 ) );
  if ( not first_in_tgt->has_proxies() )
  {
    throw IllegalConnection( "Spatial Connect with pairwise_bernoulli to devices is not possible." );
  }

// sharing specs on next line commented out because gcc 4.2 cannot handle them
#pragma omp parallel // default(none) shared(source, target, masked_layer,
                     // target_begin, target_end)
  {
    const int thread_id = kernel().vp_manager.get_thread_id();
    try
    {
      NodeCollection::const_iterator target_begin = target_nc->local_begin();
      NodeCollection::const_iterator target_end = target_nc->end();

      for ( NodeCollection::const_iterator tgt_it = target_begin; tgt_it < target_end; ++tgt_it )
      {
        Node* const tgt = kernel().node_manager.get_node_or_proxy( ( *tgt_it ).node_id, thread_id );

        assert( not tgt->is_proxy() );

        const Position< D > target_pos = target.get_position( ( *tgt_it ).lid );

        if ( mask_.get() )
        {
          // We do the same as in the target driven case, except that we calculate displacements in the target layer.
          // We therefore send in target as last parameter.
          connect_to_target_( pool.masked_begin( target_pos ), pool.masked_end(), tgt, target_pos, thread_id, target );
        }
        else
        {
          // We do the same as in the target driven case, except that we calculate displacements in the target layer.
          // We therefore send in target as last parameter.
          connect_to_target_( pool.begin(), pool.end(), tgt, target_pos, thread_id, target );
        }

      } // end for
    }
    catch ( std::exception& err )
    {
      // We must create a new exception here, err's lifetime ends at the end of the catch block.
      exceptions_raised_.at( thread_id ) =
        std::shared_ptr< WrappedThreadException >( new WrappedThreadException( err ) );
    }
  } // omp parallel
  // check if any exceptions have been raised
  for ( thread thr = 0; thr < kernel().vp_manager.get_num_threads(); ++thr )
  {
    if ( exceptions_raised_.at( thr ).get() )
    {
      throw WrappedThreadException( *( exceptions_raised_.at( thr ) ) );
    }
  }
}

template < int D >
void
ConnectionCreator::fixed_indegree_( Layer< D >& source,
  NodeCollectionPTR source_nc,
  Layer< D >& target,
  NodeCollectionPTR target_nc )
{
  if ( number_of_connections_ < 1 )
  {
    return;
  }

  // fixed_indegree connections (fixed fan in)
  //
  // For each local target node:
  // 1. Apply Mask to source layer
  // 2. Compute connection probability for each source position
  // 3. Draw source nodes and make connections

  // We only need to check the first in the NodeCollection
  Node* const first_in_tgt = kernel().node_manager.get_node_or_proxy( target_nc->operator[]( 0 ) );
  if ( not first_in_tgt->has_proxies() )
  {
    throw IllegalConnection( "Spatial Connect with fixed_indegree to devices is not possible." );
  }

  NodeCollection::const_iterator target_begin = target_nc->MPI_local_begin();
  NodeCollection::const_iterator target_end = target_nc->end();

  // protect against connecting to devices without proxies
  // we need to do this before creating the first connection to leave
  // the network untouched if any target does not have proxies
  for ( NodeCollection::const_iterator tgt_it = target_begin; tgt_it < target_end; ++tgt_it )
  {
    Node* const tgt = kernel().node_manager.get_node_or_proxy( ( *tgt_it ).node_id );

    assert( not tgt->is_proxy() );
  }

  if ( mask_.get() )
  {
    MaskedLayer< D > masked_source( source, mask_, allow_oversized_, source_nc );
    const auto masked_source_end = masked_source.end();

    std::vector< std::pair< Position< D >, index > > positions;

    for ( NodeCollection::const_iterator tgt_it = target_begin; tgt_it < target_end; ++tgt_it )
    {
      index target_id = ( *tgt_it ).node_id;
      Node* const tgt = kernel().node_manager.get_node_or_proxy( target_id );

      thread target_thread = tgt->get_thread();
      librandom::RngPtr rng = get_vp_rng( target_thread );
      Position< D > target_pos = target.get_position( ( *tgt_it ).lid );

      // We create a source pos vector here that can be updated with the
      // source position. This is done to avoid creating and destroying
      // unnecessarily many vectors.
      std::vector< double > source_pos_vector( D );
      const std::vector< double > target_pos_vector = target_pos.get_vector();

      // Get (position,node ID) pairs for sources inside mask
      positions.resize( std::distance( masked_source.begin( target_pos ), masked_source_end ) );
      std::copy( masked_source.begin( target_pos ), masked_source_end, positions.begin() );

      // We will select `number_of_connections_` sources within the mask.
      // If there is no kernel, we can just draw uniform random numbers,
      // but with a kernel we have to set up a probability distribution
      // function using the Vose class.
      if ( kernel_.get() )
      {

        std::vector< double > probabilities;
        probabilities.reserve( positions.size() );

        // Collect probabilities for the sources
        for ( typename std::vector< std::pair< Position< D >, index > >::iterator iter = positions.begin();
              iter != positions.end();
              ++iter )
        {
          iter->first.get_vector( source_pos_vector );
          probabilities.push_back( kernel_->value( rng, source_pos_vector, target_pos_vector, source ) );
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
          positions[ random_id ].first.get_vector( source_pos_vector );
          for ( size_t indx = 0; indx < synapse_model_.size(); ++indx )
          {
            const double w = weight_[ indx ]->value( rng, source_pos_vector, target_pos_vector, source );
            const double d = delay_[ indx ]->value( rng, source_pos_vector, target_pos_vector, source );
            kernel().connection_manager.connect(
              source_id, tgt, target_thread, synapse_model_[ indx ], param_dicts_[ indx ][ target_thread ], d, w );
          }

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
          positions[ random_id ].first.get_vector( source_pos_vector );
          index source_id = positions[ random_id ].second;
          for ( size_t indx = 0; indx < synapse_model_.size(); ++indx )
          {
            const double w = weight_[ indx ]->value( rng, source_pos_vector, target_pos_vector, source );
            const double d = delay_[ indx ]->value( rng, source_pos_vector, target_pos_vector, source );
            kernel().connection_manager.connect(
              source_id, tgt, target_thread, synapse_model_[ indx ], param_dicts_[ indx ][ target_thread ], d, w );
          }

          is_selected[ random_id ] = true;
        }
      }
    }
  }
  else
  {
    // no mask

    // Get (position,node ID) pairs for all nodes in source layer
    std::vector< std::pair< Position< D >, index > >* positions = source.get_global_positions_vector( source_nc );

    for ( NodeCollection::const_iterator tgt_it = target_begin; tgt_it < target_end; ++tgt_it )
    {
      index target_id = ( *tgt_it ).node_id;
      Node* const tgt = kernel().node_manager.get_node_or_proxy( target_id );
      thread target_thread = tgt->get_thread();
      librandom::RngPtr rng = get_vp_rng( target_thread );
      Position< D > target_pos = target.get_position( ( *tgt_it ).lid );

      std::vector< double > source_pos_vector( D );
      const std::vector< double > target_pos_vector = target_pos.get_vector();

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
      if ( kernel_.get() )
      {

        std::vector< double > probabilities;
        probabilities.reserve( positions->size() );

        // Collect probabilities for the sources
        for ( typename std::vector< std::pair< Position< D >, index > >::iterator iter = positions->begin();
              iter != positions->end();
              ++iter )
        {
          iter->first.get_vector( source_pos_vector );
          probabilities.push_back( kernel_->value( rng, source_pos_vector, target_pos_vector, source ) );
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

          ( *positions )[ random_id ].first.get_vector( source_pos_vector );
          for ( size_t indx = 0; indx < synapse_model_.size(); ++indx )
          {
            const double w = weight_[ indx ]->value( rng, source_pos_vector, target_pos_vector, source );
            const double d = delay_[ indx ]->value( rng, source_pos_vector, target_pos_vector, source );
            kernel().connection_manager.connect(
              source_id, tgt, target_thread, synapse_model_[ indx ], param_dicts_[ indx ][ target_thread ], d, w );
          }

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

          ( *positions )[ random_id ].first.get_vector( source_pos_vector );
          for ( size_t indx = 0; indx < synapse_model_.size(); ++indx )
          {
            const double w = weight_[ indx ]->value( rng, source_pos_vector, target_pos_vector, source );
            const double d = delay_[ indx ]->value( rng, source_pos_vector, target_pos_vector, source );
            kernel().connection_manager.connect(
              source_id, tgt, target_thread, synapse_model_[ indx ], param_dicts_[ indx ][ target_thread ], d, w );
          }

          is_selected[ random_id ] = true;
        }
      }
    }
  }
}


template < int D >
void
ConnectionCreator::fixed_outdegree_( Layer< D >& source,
  NodeCollectionPTR source_nc,
  Layer< D >& target,
  NodeCollectionPTR target_nc )
{
  if ( number_of_connections_ < 1 )
  {
    return;
  }

  // protect against connecting to devices without proxies
  // we need to do this before creating the first connection to leave
  // the network untouched if any target does not have proxies

  // We only need to check the first in the NodeCollection
  Node* const first_in_tgt = kernel().node_manager.get_node_or_proxy( target_nc->operator[]( 0 ) );
  if ( not first_in_tgt->has_proxies() )
  {
    throw IllegalConnection( "Spatial Connect with fixed_outdegree to devices is not possible." );
  }

  NodeCollection::const_iterator target_begin = target_nc->MPI_local_begin();
  NodeCollection::const_iterator target_end = target_nc->end();

  for ( NodeCollection::const_iterator tgt_it = target_begin; tgt_it < target_end; ++tgt_it )
  {
    Node* const tgt = kernel().node_manager.get_node_or_proxy( ( *tgt_it ).node_id );

    assert( not tgt->is_proxy() );
  }

  // Fixed_outdegree connections (fixed fan out)
  //
  // For each (global) source: (All connections made on all mpi procs)
  // 1. Apply mask to global targets
  // 2. If using kernel: Compute connection probability for each global target
  // 3. Draw connections to make using global rng

  MaskedLayer< D > masked_target( target, mask_, allow_oversized_, target_nc );
  const auto masked_target_end = masked_target.end();

  // We create a target positions vector here that can be updated with the
  // position and node ID pairs. This is done to avoid creating and destroying
  // unnecessarily many vectors.
  std::vector< std::pair< Position< D >, index > > target_pos_node_id_pairs;
  std::vector< std::pair< Position< D >, index > > source_pos_node_id_pairs =
    *source.get_global_positions_vector( source_nc );

  for ( const auto& source_pos_node_id_pair : source_pos_node_id_pairs )
  {
    const Position< D > source_pos = source_pos_node_id_pair.first;
    const index source_id = source_pos_node_id_pair.second;
    const std::vector< double > source_pos_vector = source_pos.get_vector();

    // We create a target pos vector here that can be updated with the
    // target position. This is done to avoid creating and destroying
    // unnecessarily many vectors.
    std::vector< double > target_pos_vector( D );
    std::vector< double > probabilities;

    // Find potential targets and probabilities
    librandom::RngPtr rng = get_global_rng();
    target_pos_node_id_pairs.resize( std::distance( masked_target.begin( source_pos ), masked_target_end ) );
    std::copy( masked_target.begin( source_pos ), masked_target_end, target_pos_node_id_pairs.begin() );

    probabilities.reserve( target_pos_node_id_pairs.size() );
    if ( kernel_.get() )
    {
      for ( const auto& target_pos_node_id_pair : target_pos_node_id_pairs )
      {
        // TODO: Why is probability calculated in source layer, but weight and delay in target layer?
        target_pos_node_id_pair.first.get_vector( target_pos_vector );
        probabilities.push_back( kernel_->value( rng, source_pos_vector, target_pos_vector, source ) );
      }
    }
    else
    {
      probabilities.resize( target_pos_node_id_pairs.size(), 1.0 );
    }

    if ( target_pos_node_id_pairs.empty()
      or ( ( not allow_multapses_ ) and ( target_pos_node_id_pairs.size() < number_of_connections_ ) ) )
    {
      std::string msg = String::compose( "Global source ID %1: Not enough targets found", source_id );
      throw KernelException( msg.c_str() );
    }

    // Draw targets.  A Vose object draws random integers with a
    // non-uniform distribution.
    Vose lottery( probabilities );

    // If multapses are not allowed, we must keep track of which
    // targets have been selected already.
    std::vector< bool > is_selected( target_pos_node_id_pairs.size() );

    // Draw `number_of_connections_` targets
    for ( long i = 0; i < ( long ) number_of_connections_; ++i )
    {
      index random_id = lottery.get_random_id( get_global_rng() );
      if ( ( not allow_multapses_ ) and ( is_selected[ random_id ] ) )
      {
        --i;
        continue;
      }
      index target_id = target_pos_node_id_pairs[ random_id ].second;
      if ( ( not allow_autapses_ ) and ( source_id == target_id ) )
      {
        --i;
        continue;
      }

      is_selected[ random_id ] = true;

      target_pos_node_id_pairs[ random_id ].first.get_vector( target_pos_vector );

      std::vector< double > rng_weight_vec;
      std::vector< double > rng_delay_vec;
      for ( size_t indx = 0; indx < weight_.size(); ++indx )
      {
        rng_weight_vec.push_back( weight_[ indx ]->value( rng, source_pos_vector, target_pos_vector, target ) );
        rng_delay_vec.push_back( delay_[ indx ]->value( rng, source_pos_vector, target_pos_vector, target ) );
      }

      // We bail out for non-local neurons only now after all possible
      // random numbers haven been drawn. Bailing out any earlier may lead
      // to desynchronized global rngs.
      if ( not kernel().node_manager.is_local_node_id( target_id ) )
      {
        continue;
      }

      Node* target_ptr = kernel().node_manager.get_node_or_proxy( target_id );
      thread target_thread = target_ptr->get_thread();

      for ( size_t indx = 0; indx < synapse_model_.size(); ++indx )
      {
        kernel().connection_manager.connect( source_id,
          target_ptr,
          target_thread,
          synapse_model_[ indx ],
          param_dicts_[ indx ][ target_thread ],
          rng_delay_vec[ indx ],
          rng_weight_vec[ indx ] );
      }
    }
  }
}

} // namespace nest

#endif
