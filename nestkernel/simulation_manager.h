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

// C includes:
#include <sys/time.h>

// C++ includes:
#include <vector>

// Includes from libnestutil:
#include "manager_interface.h"
#include "stopwatch.h"

// Includes from nestkernel:
#include "nest_time.h"
#include "nest_types.h"

// Includes from sli:
#include "dictdatum.h"

namespace nest
{
class Node;

class SimulationManager : public ManagerInterface
{
public:
  SimulationManager();

  virtual void initialize();
  virtual void finalize();

  virtual void set_status( const DictionaryDatum& );
  virtual void get_status( DictionaryDatum& );

  /**
      check for errors in time before run
      @throws KernelException if illegal time passed
  */
  void assert_valid_simtime( Time const& );

  /*
     Simulate can be broken up into .. prepare... run.. run.. cleanup..
     instead of calling simulate multiple times, and thus reduplicating
     effort in prepare, cleanup many times.
  */

  /**
     Initialize simulation for a set of run calls.
     Must be called before a sequence of runs, and again after cleanup.
  */
  void prepare();
  /**
     Run a simulation for another `Time`. Can be repeated ad infinitum with
     calls to get_status(), but any changes to the network are undefined,
     leading serious risk of incorrect results.
  */
  void run( Time const& );
  /**
     Closes a set of runs, doing finalizations such as file closures.
     After cleanup() is called, no more run()s can be called before another
     prepare() call.
  */
  void cleanup();

  /**
   * Returns true if waveform relaxation is used.
   */
  bool use_wfr() const;

  /**
   * Get the desired communication interval for the waveform relaxation
   */
  double get_wfr_comm_interval() const;

  /**
   * Get the convergence tolerance of the waveform relaxation method
   */
  double get_wfr_tol() const;

  /**
   * Get the interpolation order of the waveform relaxation method
   */
  size_t get_wfr_interpolation_order() const;

  /**
   * Get the time at the beginning of the current time slice.
   */
  Time const& get_slice_origin() const;

  /**
   * Get the time at the beginning of the previous time slice.
   */
  Time const get_previous_slice_origin() const;

  /**
   * Precise time of simulation.
   * @note The precise time of the simulation is defined only
   *       while the simulation is not in progress.
   */
  Time const get_time() const;

  /**
   * Return true, if the SimulationManager has already been simulated for some
   * time. This does NOT indicate that simulate has been called (i.e. if
   * Simulate is called with 0 as argument, the flag is still set to false.)
   */
  bool has_been_simulated() const;

  /**
   * Return true, if the SimulationManager has been prepared for simulation.
   * This is the case from the time when the Prepare function is called until
   * the simulation context is left by a call to Cleanup.
   */
  bool has_been_prepared() const;

  /**
   * Get slice number. Increased by one for each slice. Can be used
   * to choose alternating buffers.
   */
  size_t get_slice() const;

  //! Return current simulation time.
  // TODO: Precisely how defined? Rename!
  Time const& get_clock() const;

  /**
   * Get the simulation duration in the current call to run().
   */
  Time run_duration() const;

  /**
   * Get the start time of the current call to run().
   */
  Time run_start_time() const;

  /**
   * Get the simulation's time at the end of the current call to run().
   */
  Time run_end_time() const;

  //! Return start of current time slice, in steps.
  // TODO: rename / precisely how defined?
  delay get_from_step() const;

  //! Return end of current time slice, in steps.
  // TODO: rename / precisely how defined?
  delay get_to_step() const;

  //! Sorts source table and connections and create new target table.
  void update_connection_infrastructure( const thread tid );

  /**
   * Set time measurements for internal profiling to zero (reg. prep.)
   */
  virtual void reset_timers_for_preparation();

  /**
   * Set time measurements for internal profiling to zero (reg. sim. dyn.)
   */
  virtual void reset_timers_for_dynamics();

private:
  void call_update_(); //!< actually run simulation, aka wrap update_
  void update_();      //! actually perform simulation
  bool wfr_update_( Node* );
  void advance_time_();   //!< Update time to next time step
  void print_progress_(); //!< TODO: Remove, replace by logging!

  Time clock_;                     //!< SimulationManager clock, updated once per slice
  delay slice_;                    //!< current update slice
  delay to_do_;                    //!< number of pending steps
  delay to_do_total_;              //!< number of requested steps in current simulation
  delay from_step_;                //!< update clock_+from_step<=T<clock_+to_step_
  delay to_step_;                  //!< update clock_+from_step<=T<clock_+to_step_
  timeval t_slice_begin_;          //!< Wall-clock time at the begin of a time slice
  timeval t_slice_end_;            //!< Wall-clock time at the end of time slice
  long t_real_;                    //!< Accumulated wall-clock time spent simulating (in us)
  bool prepared_;                  //!< Indicates whether the SimulationManager is in a prepared
                                   //!< state
  bool simulating_;                //!< true if simulation in progress
  bool simulated_;                 //!< indicates whether the SimulationManager has already been
                                   //!< simulated for sometime
  bool inconsistent_state_;        //!< true after exception during update_
                                   //!< simulation must not be resumed
  bool print_time_;                //!< Indicates whether time should be printed during
                                   //!< simulations (or not)
  bool use_wfr_;                   //!< Indicates wheter waveform relaxation is used
  double wfr_comm_interval_;       //!< Desired waveform relaxation communication
                                   //!< interval (in ms)
  double wfr_tol_;                 //!< Convergence tolerance of waveform relaxation method
  long wfr_max_iterations_;        //!< maximal number of iterations used for waveform
                                   //!< relaxation
  size_t wfr_interpolation_order_; //!< interpolation order for waveform
                                   //!< relaxation method

  // private stop watches for benchmarking purposes
  Stopwatch sw_simulate_;
  Stopwatch sw_communicate_prepare_;
#ifdef TIMER_DETAILED
  // intended for internal core developers, not for use in the public API
  Stopwatch sw_gather_spike_data_;
  Stopwatch sw_update_;
  Stopwatch sw_gather_target_data_;
#endif
};

inline Time const&
SimulationManager::get_slice_origin() const
{
  return clock_;
}

inline Time const
SimulationManager::get_time() const
{
  assert( not simulating_ );
  return clock_ + Time::step( from_step_ );
}

inline bool
SimulationManager::has_been_simulated() const
{
  return simulated_;
}

inline bool
SimulationManager::has_been_prepared() const
{
  return prepared_;
}

inline size_t
SimulationManager::get_slice() const
{
  return slice_;
}

inline Time const&
SimulationManager::get_clock() const
{
  return clock_;
}

inline Time
SimulationManager::run_duration() const
{
  return to_do_total_ * Time::get_resolution();
}

inline Time
SimulationManager::run_start_time() const
{
  assert( not simulating_ ); // implicit due to using get_time()
  return get_time() - ( to_do_total_ - to_do_ ) * Time::get_resolution();
}

inline Time
SimulationManager::run_end_time() const
{
  assert( not simulating_ ); // implicit due to using get_time()
  return ( get_time().get_steps() + to_do_ ) * Time::get_resolution();
}

inline delay
SimulationManager::get_from_step() const
{
  return from_step_;
}

inline delay
SimulationManager::get_to_step() const
{
  return to_step_;
}

inline bool
SimulationManager::use_wfr() const
{
  return use_wfr_;
}

inline double
SimulationManager::get_wfr_comm_interval() const
{
  return wfr_comm_interval_;
}

inline double
SimulationManager::get_wfr_tol() const
{
  return wfr_tol_;
}

inline size_t
SimulationManager::get_wfr_interpolation_order() const
{
  return wfr_interpolation_order_;
}
}


#endif /* SIMULATION_MANAGER_H */
