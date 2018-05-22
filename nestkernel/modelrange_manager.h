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
  ~ModelRangeManager()
  {
  }

  virtual void initialize();
  virtual void finalize();

  virtual void
  set_status( const DictionaryDatum& )
  {
  }
  virtual void
  get_status( DictionaryDatum& )
  {
  }

  /**
   * Assign a range of GIDs for the given model
   */
  void add_range( index model, index first_gid, index last_gid );

  /**
   * Check whether a GID is with the range of assigned gids
   */
  bool is_in_range( index gid ) const;

  /**
   * Get the ID of the model to which this GID is assigned
   */
  index get_model_id( index gid ) const;

  /**
   * Return the Model for a given GID.
   */
  Model* get_model_of_gid( index );

  /**
   * Check whether this model ID has any gids assigned to it
   */
  bool model_in_use( index i ) const;

  /**
   * Return the contiguous range of IDs of nodes assigned to the same model
   * as the node with the given GID.
   */
  const modelrange& get_contiguous_gid_range( index gid ) const;

  std::vector< modelrange >::const_iterator begin() const;

  std::vector< modelrange >::const_iterator end() const;

private:
  std::vector< modelrange > modelranges_;
  index first_gid_;
  index last_gid_;
};


inline bool
nest::ModelRangeManager::is_in_range( index gid ) const
{
  return ( ( gid <= last_gid_ ) and ( gid >= first_gid_ ) );
}

inline std::vector< modelrange >::const_iterator
nest::ModelRangeManager::begin() const
{
  return modelranges_.begin();
}

inline std::vector< modelrange >::const_iterator
nest::ModelRangeManager::end() const
{
  return modelranges_.end();
}

} // namespace nest end

#endif
