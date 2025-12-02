/*
 *  buffer_resize_log.h
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

#ifndef BUFFER_RESIZE_LOG_H
#define BUFFER_RESIZE_LOG_H

// C++ includes:
#include <vector>

// Includes from libnestutil:
#include "dictionary.h"

namespace nest
{

/**
 * Collect information on spike transmission buffer resizing.
 */
class BufferResizeLog
{
public:
  BufferResizeLog();
  void clear();
  void add_entry( size_t global_max_spikes_sent, size_t new_buffer_size );
  void to_dict( Dictionary& ) const;

private:
  std::vector< long > time_steps_;             //!< Time of resize event in steps
  std::vector< long > global_max_spikes_sent_; //!< Spike number that triggered resize
  std::vector< long > new_buffer_size_;        //!< Buffer size after resize
};

}

#endif /* buffer_resize_log_h */
