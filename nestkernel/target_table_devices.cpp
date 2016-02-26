/*
 *  target_table_devices.cpp
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

#include "target_table_devices.h"

// Includes from nestkernel:
#include "kernel_manager.h"
#include "connector_base.h"

nest::TargetTableDevices::TargetTableDevices()
{
}

nest::TargetTableDevices::~TargetTableDevices()
{
}

void
nest::TargetTableDevices::initialize()
{
  const thread num_threads = kernel().vp_manager.get_num_threads();
  target_to_devices_.resize( num_threads );
  target_from_devices_.resize( num_threads );
  for( thread tid = 0; tid < num_threads; ++tid)
  {
    target_to_devices_[ tid ] = new std::vector< HetConnector* >( 0 );
    target_from_devices_[ tid ] = new std::vector< HetConnector* >( 0 );
  }
}

void
nest::TargetTableDevices::finalize()
{
  for( std::vector< std::vector< HetConnector* >* >::iterator it =
         target_to_devices_.begin(); it != target_to_devices_.end(); ++it )
  {
    for ( std::vector< HetConnector* >::iterator jt = (*it)->begin(); jt != (*it)->end(); ++jt )
    {
      delete *jt;
    }
    delete *it;
  }
  for( std::vector< std::vector< HetConnector* >* >::iterator it =
         target_from_devices_.begin(); it != target_from_devices_.end(); ++it )
  {
    for ( std::vector< HetConnector* >::iterator jt = (*it)->begin(); jt != (*it)->end(); ++jt )
    {
      delete *jt;
    }
    delete *it;
  }
  target_to_devices_.clear();
  target_from_devices_.clear();
}

void
nest::TargetTableDevices::resize()
{
  const thread num_threads = kernel().vp_manager.get_num_threads();
  for( thread tid = 0; tid < num_threads; ++tid)
  {
    size_t old_size_to_devices = target_to_devices_[ tid ]->size();
    target_to_devices_[ tid ]->resize( kernel().node_manager.get_max_num_local_nodes() );
    for ( size_t i = old_size_to_devices; i < target_to_devices_[ tid ]->size(); ++i )
    {
      (*target_to_devices_[ tid ])[ i ] = new HetConnector();
    }
    size_t old_size_from_devices = target_from_devices_[ tid ]->size();
    target_from_devices_[ tid ]->resize( kernel().node_manager.get_num_local_devices() );
    for ( size_t i = old_size_from_devices; i < target_from_devices_[ tid ]->size(); ++i )
    {
      (*target_from_devices_[ tid ])[ i ] = new HetConnector();
    }
  }
}
