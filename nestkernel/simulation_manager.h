/*
 *  simulation_manager.h
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

#ifndef SIMULATION_MANAGER_H
#define SIMULATION_MANAGER_H

#include "nest_types.h"
#include "manager_interface.h"
#include "dictdatum.h"

namespace nest
{

class SimulationManager : public ManagerInterface
{
public:
  SimulationManager();

  virtual void init();
  virtual void reset();

  virtual void set_status( const DictionaryDatum& );
  virtual void get_status( DictionaryDatum& );

  /*
  void set_status( index, const DictionaryDatum& );
  DictionaryDatum get_status( index );
  void simulate( Time const& );
  void resume();
  void terminate();
  Time const& get_slice_origin() const;
  Time get_previous_slice_origin() const;
  Time const get_time() const;
  bool get_simulated() const; // => is_simulated
  void calibrate_clock();
  void reset_network();
  void prepare_simulation();
  void finalize_simulation();
  void update();
  size_t get_slice() const;
  void advance_time_();
  void print_progress_();
  */

  /*
    bool simulating_; //!< true if simulation in progress
Time clock_; //!< Network clock, updated once per slice
delay slice_; //!< current update slice
delay to_do_; //!< number of pending cycles.
delay to_do_total_; //!< number of requested cycles in current simulation.
delay from_step_; //!< update clock_+from_step<=T<clock_+to_step_
delay to_step_; //!< update clock_+from_step<=T<clock_+to_step_
timeval t_slice_begin_; //!< Wall-clock time at the begin of a time slice
timeval t_slice_end_; //!< Wall-clock time at the end of time slice
long t_real_; //!< Accumunated wall-clock time spent simulating (in us)
bool terminate_; //!< Terminate on signal or error
bool simulated_; //!< indicates whether the network has already been simulated for some time
bool print_time_; //!< Indicates whether time should be printed during simulations (or not)
   */

};
}

#endif /* SIMULATION_MANAGER_H */
