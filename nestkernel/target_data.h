/*
 *  target_data.h
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

#ifndef TARGET_DATA_H
#define TARGET_DATA_H

// Includes from nestkernel:
#include "nest_types.h"
#include "target.h"

namespace nest
{

/**
 * Structure used to communicate part of the connection infrastructure
 * from post- to presynaptic side. These are the elements of the MPI
 * buffers.
 * SeeAlso: SpikeData
 */
struct TargetData
{
  Target target;
  index lid : 20;  //!< local id of presynaptic neuron
  thread tid : 10; //!< thread index of presynaptic neuron
  unsigned int marker : 2;
  static const unsigned int complete_marker = 1;
  static const unsigned int end_marker = 2;
  static const unsigned int invalid_marker = 3;
  TargetData();
  void reset_marker();
  void set_complete_marker();
  void set_end_marker();
  void set_invalid_marker();
  bool is_complete_marker() const;
  bool is_end_marker() const;
  bool is_invalid_marker() const;
};

inline TargetData::TargetData()
  : target( Target() )
  , lid( 0 )
  , tid( 0 )
  , marker( 0 )
{
}

inline void
TargetData::reset_marker()
{
  marker = 0;
}

inline void
TargetData::set_complete_marker()
{
  marker = complete_marker;
}

inline void
TargetData::set_end_marker()
{
  marker = end_marker;
}

inline void
TargetData::set_invalid_marker()
{
  marker = invalid_marker;
}

inline bool
TargetData::is_complete_marker() const
{
  return marker == complete_marker;
}

inline bool
TargetData::is_end_marker() const
{
  return marker == end_marker;
}

inline bool
TargetData::is_invalid_marker() const
{
  return marker == invalid_marker;
}

} // namespace nest

#endif // TARGET_DATA_H
