/*
 *  modelrange_manager.h
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

#ifndef MODELRANGEMANAGER_H
#define MODELRANGEMANAGER_H

// C++ includes:
#include <vector>

// Includes from libnestutil:
#include "manager_interface.h"

// Includes from nestkernel:
#include "modelrange.h"
#include "nest_types.h"

namespace nest
{
class Model;

class ModelRangeManager : public ManagerInterface
{
public:
  ModelRangeManager();
  ~ModelRangeManager() override
  {
  }

  void initialize( const bool ) override;
  void finalize( const bool ) override;
  void set_status( const DictionaryDatum& ) override;
  void get_status( DictionaryDatum& ) override;

  /**
   * Assign a range of node IDs for the given model
   */
  void add_range( size_t model, size_t first_node_id, size_t last_node_id );

  /**
   * Check whether a node ID is with the range of assigned node IDs
   */
  bool is_in_range( size_t node_id ) const;

  /**
   * Get the ID of the model to which this node ID is assigned
   */
  size_t get_model_id( size_t node_id ) const;

  /**
   * Return the Model for a given node ID.
   */
  Model* get_model_of_node_id( size_t );

  /**
   * Return the contiguous range of IDs of nodes assigned to the same model
   * as the node with the given node ID.
   */
  const modelrange& get_contiguous_node_id_range( size_t node_id ) const;

  std::vector< modelrange >::const_iterator begin() const;

  std::vector< modelrange >::const_iterator end() const;

private:
  std::vector< modelrange > modelranges_;
  size_t first_node_id_;
  size_t last_node_id_;
};


} // namespace nest end

#endif
