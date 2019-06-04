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

// Includes from topology:
#include "mask.h"
#include "position.h"
#include "selector.h"
#include "topology_names.h"
#include "topologymodule.h"
#include "topology_parameter.h"
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
 * convergent_connect_) implementing the concrete connection
 * algorithm. It would be more elegant if this was a base class for
 * classes representing different connection algorithms with a virtual
 * connect method, but it is not possible to have a virtual template
 * method.
 *
 * This class distinguishes between target driven and convergent
 * connections, which are both called "convergent" in the Topology module
 * documentation, and between source driven and divergent
 * connections. The true convergent/divergent connections are those with
 * a fixed number of connections (fan in/out). The only difference
 * between source driven and target driven connections is which layer
 * coordinates the mask and parameters are defined in.
 */
class ConnectionCreator
{
public:
  enum ConnectionType
  {
    Target_driven,
    Source_driven,
    Convergent,
    Divergent
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
   * - "weights": Synaptic weight (dictionary, parametertype, or double).
   * - "delays": Synaptic delays (dictionary, parametertype, or double).
   * - other parameters are interpreted as synapse parameters, and may
   *   be defined by a dictionary, parametertype, or double.
   * @param dict dictionary containing properties for the connections.
   */
  ConnectionCreator( DictionaryDatum dict );

  /**
   * Connect two layers.
   * @param source source layer.
   * @param target target layer.
   */
  template < int D >
  void connect( Layer< D >& source, Layer< D >& target );

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

    typename Ntree< D, index >::masked_iterator masked_begin(
      const Position< D >& pos ) const;
    typename Ntree< D, index >::masked_iterator masked_end() const;

    typename std::vector< std::pair< Position< D >, index > >::iterator
    begin() const;
    typename std::vector< std::pair< Position< D >, index > >::iterator
    end() const;

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
  void target_driven_connect_( Layer< D >& source, Layer< D >& target );

  template < int D >
  void source_driven_connect_( Layer< D >& source, Layer< D >& target );

  template < int D >
  void convergent_connect_( Layer< D >& source, Layer< D >& target );

  template < int D >
  void divergent_connect_( Layer< D >& source, Layer< D >& target );

  void connect_( index s,
    Node* target,
    thread target_thread,
    double w,
    double d,
    index syn );

  /**
   * Calculate parameter values for this position.
   *
   * TODO: remove when all four connection variants are refactored
   */
  template < int D >
  void get_parameters_( const Position< D >& pos,
    librandom::RngPtr rng,
    double& weight,
    double& delay );

  ConnectionType type_;
  bool allow_autapses_;
  bool allow_multapses_;
  bool allow_oversized_;
  Selector source_filter_;
  Selector target_filter_;
  index number_of_connections_;
  lockPTR< AbstractMask > mask_;
  lockPTR< TopologyParameter > kernel_;
  index synapse_model_;
  lockPTR< TopologyParameter > weight_;
  lockPTR< TopologyParameter > delay_;

  //! Empty dictionary to pass to connect functions
  const static DictionaryDatum dummy_param_;
};

inline void
ConnectionCreator::connect_( index s,
  Node* target,
  thread target_thread,
  double w,
  double d,
  index syn )
{
  // check whether the target is on this process
  if ( kernel().node_manager.is_local_gid( target->get_gid() ) )
  {
    // check whether the target is on our thread
    thread tid = kernel().vp_manager.get_thread_id();
    if ( tid == target_thread )
    {
      // TODO implement in terms of nest-api
      kernel().connection_manager.connect(
        s, target, target_thread, syn, dummy_param_, d, w );
    }
  }
}

} // namespace nest

#endif
