/*
 *  connection_creator.h
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

#ifndef CONNECTION_CREATOR_H
#define CONNECTION_CREATOR_H

// C++ includes:
#include <vector>

// Includes from nestkernel:
#include "kernel_manager.h"
#include "nest_names.h"
#include "nestmodule.h"

// Includes from spatial:
#include "mask.h"
#include "position.h"
#include "vose.h"

namespace nest
{
template < int D >
class Layer;

template < int D >
class MaskedLayer;

/**
 * This class is a representation of the dictionary of connection
 * properties given as an argument to the ConnectLayers function. The
 * connect method is responsible for generating the connection according
 * to the given parameters. This method is templated with the dimension
 * of the layers, and is called via the Layer connect call using a
 * visitor pattern. The connect method relays to another method (e.g.,
 * fixed_indegree_) implementing the concrete connection
 * algorithm. It would be more elegant if this was a base class for
 * classes representing different connection algorithms with a virtual
 * connect method, but it is not possible to have a virtual template
 * method.
 *
 * The difference between the Pairwise_bernoulli_on_source and
 * Pairwise_bernoulli_on_target connection types is which layer
 * coordinates the mask and parameters are defined in.
 */
class ConnectionCreator
{
public:
  enum ConnectionType
  {
    Pairwise_bernoulli_on_source,
    Pairwise_bernoulli_on_target,
    Fixed_indegree,
    Fixed_outdegree
  };

  /**
   * Construct a ConnectionCreator with the properties defined in the
   * given dictionary. Parameters for a ConnectionCreator are:
   * - "connection_type": Either "convergent" or "divergent".
   * - "allow_autapses": Boolean, true if autapses are allowed.
   * - "allow_multapses": Boolean, true if multapses are allowed.
   * - "allow_oversized": Boolean, true if oversized masks are allowed.
   * - "number_of_connections": Integer, number of connections to make
   *   for each source or target.
   * - "mask": Mask definition (dictionary or masktype).
   * - "kernel": Kernel definition (dictionary, parametertype, or double).
   * - "synapse_model": The synapse model to use.
   * - "targets": Which targets (model or lid) to select (dictionary).
   * - "sources": Which targets (model or lid) to select (dictionary).
   * - "weight": Synaptic weight (dictionary, parametertype, or double).
   * - "delay": Synaptic delays (dictionary, parametertype, or double).
   * - other parameters are interpreted as synapse parameters, and may
   *   be defined by a dictionary, parametertype, or double.
   * @param dict dictionary containing properties for the connections.
   */
  ConnectionCreator( DictionaryDatum dict );

  /**
   * Connect two layers.
   * @param source source layer.
   * @param source NodeCollection of the source.
   * @param target target layer.
   * @param target NodeCollection of the target.
   */
  template < int D >
  void connect( Layer< D >& source, NodeCollectionPTR source_nc, Layer< D >& target, NodeCollectionPTR target_nc );

private:
  /**
   * Wrapper for masked and unmasked pools.
   *
   * The purpose is to avoid code doubling for cases with and without masks.
   * Essentially, the class works as a fancy union.
   */
  template < int D >
  class PoolWrapper_
  {
  public:
    PoolWrapper_();
    ~PoolWrapper_();
    void define( MaskedLayer< D >* );
    void define( std::vector< std::pair< Position< D >, index > >* );

    typename Ntree< D, index >::masked_iterator masked_begin( const Position< D >& pos ) const;
    typename Ntree< D, index >::masked_iterator masked_end() const;

    typename std::vector< std::pair< Position< D >, index > >::iterator begin() const;
    typename std::vector< std::pair< Position< D >, index > >::iterator end() const;

  private:
    MaskedLayer< D >* masked_layer_;
    std::vector< std::pair< Position< D >, index > >* positions_;
  };

  template < typename Iterator, int D >
  void connect_to_target_( Iterator from,
    Iterator to,
    Node* tgt_ptr,
    const Position< D >& tgt_pos,
    thread tgt_thread,
    const Layer< D >& source );

  template < int D >
  void pairwise_bernoulli_on_source_( Layer< D >& source,
    NodeCollectionPTR source_nc,
    Layer< D >& target,
    NodeCollectionPTR target_nc );

  template < int D >
  void pairwise_bernoulli_on_target_( Layer< D >& source,
    NodeCollectionPTR source_nc,
    Layer< D >& target,
    NodeCollectionPTR target_nc );

  template < int D >
  void
  fixed_indegree_( Layer< D >& source, NodeCollectionPTR source_nc, Layer< D >& target, NodeCollectionPTR target_nc );

  template < int D >
  void
  fixed_outdegree_( Layer< D >& source, NodeCollectionPTR source_nc, Layer< D >& target, NodeCollectionPTR target_nc );

  ConnectionType type_;
  bool allow_autapses_;
  bool allow_multapses_;
  bool allow_oversized_;
  index number_of_connections_;
  std::shared_ptr< AbstractMask > mask_;
  std::shared_ptr< Parameter > kernel_;
  std::vector< index > synapse_model_;
  std::vector< std::shared_ptr< Parameter > > weight_;
  std::vector< std::shared_ptr< Parameter > > delay_;

  //! Empty dictionary to pass to connect functions, one per thread
  std::vector< DictionaryDatum > dummy_param_dicts_;
};

} // namespace nest

#endif
