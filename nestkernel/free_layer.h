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
#include "node_manager.h"

// Includes from sli:
#include "dictutils.h"

// Includes from spatial:
#include "layer.h"

namespace nest
{

/**
 * Layer with free positioning of neurons, positions specified by user.
 */
template < int D >
class FreeLayer : public Layer< D >
{
public:
  Position< D > get_position( size_t sind ) const override;
  void set_status( const DictionaryDatum& ) override;
  void get_status( DictionaryDatum&, NodeCollection const* ) const override;

protected:
  /**
   * Communicate positions across MPI processes
   * @param iter Insert iterator which will receive pairs of Position,node ID
   * @param node_collection NodeCollection of the layer
   */
  template < class Ins >
  void communicate_positions_( Ins iter, NodeCollectionPTR node_collection );

  void insert_global_positions_ntree_( Ntree< D, size_t >& tree, NodeCollectionPTR node_collection ) override;
  void insert_global_positions_vector_( std::vector< std::pair< Position< D >, size_t > >& vec,
    NodeCollectionPTR node_collection ) override;

  /**
   * Calculate the index in the position vector on this MPI process based on the local ID.
   *
   * @param lid global index of node within layer
   * @return index in the local position vector
   */
  size_t lid_to_position_id_( size_t lid ) const;

  //! Vector of positions.
  std::vector< Position< D > > positions_;

  size_t num_local_nodes_ = 0;

  /**
   * Class to be used when communicating positions across MPI processes.
   */
  class NodePositionData
  {
  public:
    size_t
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

} // namespace nest

#endif
