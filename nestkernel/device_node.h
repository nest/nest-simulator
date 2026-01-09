/*
 *  device_node.h
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

#ifndef DEVICE_NODE_H
#define DEVICE_NODE_H

// Includes from nestkernel:
#include "node.h"

namespace nest
{

/**
 * Base class for device objects.
 */
class DeviceNode : public Node
{

public:
  DeviceNode();

  DeviceNode( DeviceNode const& dn );

  void set_local_device_id( const size_t ldid ) override;
  size_t get_local_device_id() const override;

protected:
  size_t local_device_id_;
};

} // namespace

#endif /* #ifndef DEVICE_NODE_H */
