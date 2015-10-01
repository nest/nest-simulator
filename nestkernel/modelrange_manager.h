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

#include <vector>
#include "nest_types.h"
#include "modelrange.h"
#include "manager_interface.h"

namespace nest
{

class ModelRangeManager : ManagerInterface
{
public:
  ModelRangeManager();
  ~ModelRangeManager()
  {
  }

  virtual void init();
  virtual void reset();

  virtual void set_status( const DictionaryDatum& )
  {
  }
  virtual void get_status( DictionaryDatum& )
  {
  }

  /**
   * Assign a range of gids for the given model
   */
  void add_range( index model, index first_gid, index last_gid );

  /**
   * Check whether a gid is with the range of assigned gids
   */
  bool is_in_range( index gid ) const;

  /**
   * Get the ID of the model to which this gid is assigned
   */
  index get_model_id( index gid ) const;

  /**
   * Check whether this model ID has any gids assigned to it
   */
  bool model_in_use( index i ) const;

  /**
   * 
   */
  const modelrange& get_range( index gid ) const;

private:
  std::vector< modelrange > modelranges_;
  index first_gid_;
  index last_gid_;
};
}

inline bool 
nest::ModelRangeManager::is_in_range( index gid ) const
{
  return ( ( gid <= last_gid_ ) && ( gid >= first_gid_ ) );
}

#endif
