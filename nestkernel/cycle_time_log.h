/*
 *  cycle_time_log.h
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

#ifndef CYCLE_TIME_LOG_H
#define CYCLE_TIME_LOG_H

// C++ includes:
#include <vector>

// Includes from sli:
#include "dictdatum.h"

namespace nest
{

/**
 * Collect cycle duration and spike-count information for individual update cycles.
 */
class CycleTimeLog
{
public:
  CycleTimeLog();
  void clear();
  void add_entry( double cycle_update_time, double communicate_time, long local_spike_counter );
  void to_dict( DictionaryDatum& ) const;

private:
  std::vector< double > cycle_update_time_;  //!< Time of one update cycle
  std::vector< double > communicate_time_;   // Time of communicate in current cycle
  std::vector< long > local_spike_counter_;  // Local spike count in current cycle
};

}

#endif /* cycle_time_log_h */
