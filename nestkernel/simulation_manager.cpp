/*
 *  simulation_manager.cpp
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

#include "simulation_manager.h"

#include <sys/time.h>
#include <vector>

#include "kernel_manager.h"
#include "sibling_container.h"

#include "dictutils.h"
#include "network.h"
#include "psignal.h"
#include "nest_time.h"

nest::SimulationManager::SimulationManager()
  : simulating_( false )
  , clock_( Time::tic( 0L ) )
  , slice_( 0L )
  , to_do_( 0L )
  , to_do_total_( 0L )
  , from_step_( 0L )
  , to_step_( 0L ) // consistent with to_do_ == 0
  , t_real_( 0L )
  , terminate_( false )
  , simulated_( false )
  , print_time_( false )
{
}

void
nest::SimulationManager::init()
{
  // set resolution, ensure clock is calibrated to new resolution
  Time::reset_resolution();
  clock_.calibrate();
  kernel().event_delivery_manager.init_moduli();  // TODO: move elsewhere?

  simulated_ = false;
}

void
nest::SimulationManager::reset()
{
  nest::Time::reset_to_defaults();

  clock_.set_to_zero(); // ensures consistent state
  to_do_ = 0;
  slice_ = 0;
  from_step_ = 0;
  to_step_ = 0; // consistent with to_do_ = 0
}

void
nest::SimulationManager::set_status( const DictionaryDatum& d )
{
  // Create an instance of time converter here to capture the current
  // representation of time objects: TICS_PER_MS and TICS_PER_STEP
  // will be stored in time_converter.
  // This object can then be used to convert times in steps
  // (e.g. Connection::delay_) or tics to the new representation.
  // We pass this object to ConnectionManager::calibrate to update
  // all time objects in the connection system to the new representation.
  // MH 08-04-14
  TimeConverter time_converter;

  double_t time;
  if ( updateValue< double_t >( d, "time", time ) )
  {
    if ( time != 0.0 )
      throw BadProperty( "The simulation time can only be set to 0.0." );

    if ( clock_ > TimeZero )
    {
      // reset only if time has passed
      LOG( M_WARNING,
        "Network::set_status",
        "Simulation time reset to t=0.0. Resetting the simulation time is not "
        "fully supported in NEST at present. Some spikes may be lost, and "
        "stimulating devices may behave unexpectedly. PLEASE REVIEW YOUR "
        "SIMULATION OUTPUT CAREFULLY!" );

      clock_ = Time::step( 0 );
      from_step_ = 0;
      slice_ = 0;
      kernel().event_delivery_manager.configure_spike_buffers(); // clear all old spikes
    }
  }

  updateValue< bool >( d, "print_time", print_time_ );

  // tics_per_ms and resolution must come after local_num_thread / total_num_threads
  // because they might reset the network and the time representation
  nest::double_t tics_per_ms;
  bool tics_per_ms_updated = updateValue< nest::double_t >( d, "tics_per_ms", tics_per_ms );
  double_t resd;
  bool res_updated = updateValue< double_t >( d, "resolution", resd );

  if ( tics_per_ms_updated || res_updated )
  {
    if ( kernel().node_manager.size() > 1 ) // root always exists
    {
      LOG( M_ERROR,
        "Network::set_status",
        "Cannot change time representation after nodes have been created. Please call ResetKernel "
        "first." );
      throw KernelException();
    }
    else if ( has_been_simulated() ) // someone may have simulated empty network
    {
      LOG( M_ERROR,
        "Network::set_status",
        "Cannot change time representation after the network has been simulated. Please call "
        "ResetKernel first." );
      throw KernelException();
    }
    else if ( kernel().connection_builder_manager.get_num_connections() != 0 )
    {
      LOG( M_ERROR,
        "Network::set_status",
        "Cannot change time representation after connections have been created. Please call "
        "ResetKernel first." );
      throw KernelException();
    }
    else if ( res_updated
      && tics_per_ms_updated ) // only allow TICS_PER_MS to be changed together with resolution
    {
      if ( resd < 1.0 / tics_per_ms )
      {
        LOG( M_ERROR,
          "Network::set_status",
          "Resolution must be greater than or equal to one tic. Value unchanged." );
        throw KernelException();
      }
      else
      {
        nest::Time::set_resolution( tics_per_ms, resd );
        clock_.calibrate(); // adjust to new resolution
        kernel().connection_builder_manager.calibrate(
          time_converter ); // adjust delays in the connection system to new resolution
        LOG( M_INFO, "Network::set_status", "tics per ms and resolution changed." );
      }
    }
    else if ( res_updated ) // only resolution changed
    {
      if ( resd < Time::get_ms_per_tic() )
      {
        LOG( M_ERROR,
          "Network::set_status",
          "Resolution must be greater than or equal to one tic. Value unchanged." );
        throw KernelException();
      }
      else
      {
        Time::set_resolution( resd );
        clock_.calibrate(); // adjust to new resolution
        kernel().connection_builder_manager.calibrate(
          time_converter ); // adjust delays in the connection system to new resolution
        LOG( M_INFO, "Network::set_status", "Temporal resolution changed." );
      }
    }
    else
    {
      LOG( M_ERROR,
        "Network::set_status",
        "change of tics_per_step requires simultaneous specification of resolution." );
      throw KernelException();
    }
  }
}

void
nest::SimulationManager::get_status( DictionaryDatum& d )
{
  def< double >( d, "ms_per_tic", Time::get_ms_per_tic() );
  def< double >( d, "tics_per_ms", Time::get_tics_per_ms() );
  def< long >( d, "tics_per_step", Time::get_tics_per_step() );
  def< double >( d, "resolution", Time::get_resolution().get_ms() );

  def< double >( d, "T_min", Time::min().get_ms() );
  def< double >( d, "T_max", Time::max().get_ms() );

  def< double_t >( d, "time", get_time().get_ms() );
  def< long >( d, "to_do", to_do_ );
  def< bool >( d, "print_time", print_time_ );
}

void
nest::SimulationManager::simulate( Time const& t )
{
  assert( kernel().is_initialized() );

  t_real_ = 0;
  t_slice_begin_ = timeval();
  t_slice_end_ = timeval();

  if ( t == Time::ms( 0.0 ) )
    return;

  if ( t < Time::step( 1 ) )
  {
    LOG( M_ERROR,
      "Network::simulate",
      String::compose(
           "Simulation time must be >= %1 ms (one time step).", Time::get_resolution().get_ms() ) );
    throw KernelException();
  }

  if ( t.is_finite() )
  {
    Time time1 = clock_ + t;
    if ( !time1.is_finite() )
    {
      std::string msg = String::compose(
        "A clock overflow will occur after %1 of %2 ms. Please reset network "
        "clock first!",
        ( Time::max() - clock_ ).get_ms(),
        t.get_ms() );
      LOG( M_ERROR, "Network::simulate", msg );
      throw KernelException();
    }
  }
  else
  {
    std::string msg = String::compose(
      "The requested simulation time exceeds the largest time NEST can handle "
      "(T_max = %1 ms). Please use a shorter time!",
      Time::max().get_ms() );
    LOG( M_ERROR, "Network::simulate", msg );
    throw KernelException();
  }

  to_do_ += t.get_steps();
  to_do_total_ = to_do_;

  prepare_simulation_();

  // from_step_ is not touched here.  If we are at the beginning
  // of a simulation, it has been reset properly elsewhere.  If
  // a simulation was ended and is now continued, from_step_ will
  // have the proper value.  to_step_ is set as in advance_time().

  delay end_sim = from_step_ + to_do_;
  if ( kernel().connection_builder_manager.get_min_delay() < end_sim )
    to_step_ = kernel().connection_builder_manager.get_min_delay(); // update to end of time slice
  else
    to_step_ = end_sim; // update to end of simulation time

  // Warn about possible inconsistencies, see #504.
  // This test cannot come any earlier, because we first need to compute min_delay_
  // above.
  if ( t.get_steps() % kernel().connection_builder_manager.get_min_delay() != 0 )
    LOG( M_WARNING,
      "Network::simulate",
      "The requested simulation time is not an integer multiple of the minimal delay in the "
      "network. "
      "This may result in inconsistent results under the following conditions: (i) A network "
      "contains "
      "more than one source of randomness, e.g., two different poisson_generators, and (ii) "
      "Simulate "
      "is called repeatedly with simulation times that are not multiples of the minimal delay." );

  resume_();

  finalize_simulation_();
}

void
nest::SimulationManager::resume_()
{
  assert( kernel().is_initialized() );

  terminate_ = false;

  if ( to_do_ == 0 )
    return;

  if ( print_time_ )
  {
    // TODO: Remove direct output
    std::cout << std::endl;
    print_progress_();
  }

  simulating_ = true;
  simulated_ = true;

#ifndef _OPENMP
  if ( kernel().vp_manager.get_num_threads() > 1 )
  {
    LOG( M_ERROR, "SimulationManager::resume", "No multithreading available, using single threading" );
  }
#endif

  update_();

  simulating_ = false;

  if ( print_time_ )
    std::cout << std::endl;

  Communicator::synchronize();

  if ( terminate_ )
  {
    LOG( M_ERROR, "Network::resume", "Exiting on error or user signal." );
    LOG( M_ERROR, "Network::resume", "Network: Use 'ResumeSimulation' to resume." );

    if ( SLIsignalflag != 0 )
    {
      SystemSignal signal( SLIsignalflag );
      SLIsignalflag = 0;
      throw signal;
    }
    else
      throw SimulationError();
  }

  LOG( M_INFO, "Network::resume", "Simulation finished." );
}

void
nest::SimulationManager::prepare_simulation_()
{
  if ( to_do_ == 0 )
    return;

  // find shortest and longest delay across all MPI processes
  // this call sets the member variables
  kernel().connection_builder_manager.update_delay_extrema_();

  // Check for synchronicity of global rngs over processes.
  // We need to do this ahead of any simulation in case random numbers
  // have been consumed on the SLI level.
  if ( kernel().mpi_manager.get_num_processes() > 1 )
  {
    if ( !Communicator::grng_synchrony( kernel().rng_manager.get_grng()->ulrand( 100000 ) ) )
    {
      LOG( M_ERROR,
        "Network::simulate",
        "Global Random Number Generators are not synchronized prior to simulation." );
      throw KernelException();
    }
  }

  // if at the beginning of a simulation, set up spike buffers
  if ( !simulated_ )
    kernel().event_delivery_manager.configure_spike_buffers();

  kernel().node_manager.ensure_valid_thread_local_ids();
  kernel().node_manager.prepare_nodes();

#ifdef HAVE_MUSIC
  // we have to do enter_runtime after prepre_nodes, since we use
  // calibrate to map the ports of MUSIC devices, which has to be done
  // before enter_runtime
  if ( !simulated_ ) // only enter the runtime mode once
  {
    publish_music_in_ports_();

    double tick = Time::get_resolution().get_ms() * min_delay_;
    std::string msg = String::compose( "Entering MUSIC runtime with tick = %1 ms", tick );
    LOG( M_INFO, "Network::resume", msg );
    Communicator::enter_runtime( tick );
  }
#endif
}

void
nest::SimulationManager::update_()
{
#ifdef _OPENMP
  LOG( M_INFO, "Network::update", "Simulating using OpenMP." );
#endif

  std::vector< lockPTR< WrappedThreadException > > exceptions_raised(
    kernel().vp_manager.get_num_threads() );
// parallel section begins
#pragma omp parallel
  {
    const int thrd = kernel().vp_manager.get_thread_id();

    do
    {
      if ( print_time_ )
        gettimeofday( &t_slice_begin_, NULL );

      if ( from_step_ == 0 ) // deliver only at beginning of slice
      {
        kernel().event_delivery_manager.deliver_events( thrd );
#ifdef HAVE_MUSIC
// advance the time of music by one step (min_delay * h) must
// be done after deliver_events_() since it calls
// music_event_out_proxy::handle(), which hands the spikes over to
// MUSIC *before* MUSIC time is advanced

// wait until all threads are done -> synchronize
#pragma omp barrier
// the following block is executed by the master thread only
// the other threads are enforced to wait at the end of the block
#pragma omp master
        {
          // advance the time of music by one step (min_delay * h) must
          // be done after deliver_events_() since it calls
          // music_event_out_proxy::handle(), which hands the spikes over to
          // MUSIC *before* MUSIC time is advanced
          if ( slice_ > 0 )
            Communicator::advance_music_time( 1 );

          // the following could be made thread-safe
          update_music_event_handlers_( clock_, from_step_, to_step_ );
        }
// end of master section, all threads have to synchronize at this point
#pragma omp barrier
#endif
      }

      const std::vector< Node* >& thread_local_nodes
         = kernel().node_manager.get_nodes_on_thread(thrd);
      for ( std::vector< Node* >::const_iterator node = thread_local_nodes.begin();
            node != thread_local_nodes.end(); ++node )
      {
        // We update in a parallel region. Therefore, we need to catch exceptions
        // here and then handle them after the parallel region.
        try
        {
          if ( not( *node )->is_frozen() )
            ( *node )->update( clock_, from_step_, to_step_ );
        }
        catch ( std::exception& e )
        {
          // so throw the exception after parallel region
          exceptions_raised.at( thrd ) =
            lockPTR< WrappedThreadException >( new WrappedThreadException( e ) );
          terminate_ = true;
        }
      }

// parallel section ends, wait until all threads are done -> synchronize
#pragma omp barrier

// the following block is executed by the master thread only
// the other threads are enforced to wait at the end of the block
#pragma omp master
      {
        if ( to_step_
          == kernel().connection_builder_manager.get_min_delay() ) // gather only at end of slice
          kernel().event_delivery_manager.gather_events();

        advance_time_();

        if ( SLIsignalflag != 0 )
        {
          LOG( M_INFO, "Network::update", "Simulation exiting on user signal." );
          terminate_ = true;
        }

        if ( print_time_ )
        {
          gettimeofday( &t_slice_end_, NULL );
          print_progress_();
        }
      }
// end of master section, all threads have to synchronize at this point
#pragma omp barrier

    } while ( ( to_do_ != 0 ) && ( !terminate_ ) );

  } // end of #pragma parallel omp

  // check if any exceptions have been raised
  for ( index thrd = 0; thrd < kernel().vp_manager.get_num_threads(); ++thrd )
    if ( exceptions_raised.at( thrd ).valid() )
      throw WrappedThreadException( *( exceptions_raised.at( thrd ) ) );
}

void
nest::SimulationManager::finalize_simulation_()
{
  if ( not simulated_ )
    return;

  // Check for synchronicity of global rngs over processes
  // TODO: This seems double up, there is such a test at end of simulate()
  if ( kernel().mpi_manager.get_num_processes() > 1 )
    if ( !Communicator::grng_synchrony( kernel().rng_manager.get_grng()->ulrand( 100000 ) ) )
    {
      LOG( M_ERROR,
        "Network::simulate",
        "Global Random Number Generators are not synchronized after simulation." );
      throw KernelException();
    }

  kernel().node_manager.finalize_nodes();
}

void
nest::SimulationManager::reset_network()
{
  if ( not kernel().simulation_manager.has_been_simulated() )
    return; // nothing to do


  kernel().event_delivery_manager.clear_pending_spikes();

  kernel().node_manager.reset_nodes_state();

  // ConnectionManager doesn't support resetting dynamic synapses yet
  LOG( M_WARNING,
    "ResetNetwork",
    "Synapses with internal dynamics (facilitation, STDP) are not reset.\n"
    "This will be implemented in a future version of NEST." );
}

void
nest::SimulationManager::advance_time_()
{
  // time now advanced time by the duration of the previous step
  to_do_ -= to_step_ - from_step_;

  // advance clock, update modulos, slice counter only if slice completed
  if ( ( delay ) to_step_ == kernel().connection_builder_manager.get_min_delay() )
  {
    clock_ += Time::step( kernel().connection_builder_manager.get_min_delay() );
    ++slice_;
    kernel().event_delivery_manager.update_moduli();
    from_step_ = 0;
  }
  else
    from_step_ = to_step_;

  long_t end_sim = from_step_ + to_do_;

  if ( kernel().connection_builder_manager.get_min_delay() < ( delay ) end_sim )
    to_step_ = kernel().connection_builder_manager.get_min_delay(); // update to end of time slice
  else
    to_step_ = end_sim; // update to end of simulation time

  assert( to_step_ - from_step_ <= ( long_t ) kernel().connection_builder_manager.get_min_delay() );
}

void
nest::SimulationManager::print_progress_()
{
  double_t rt_factor = 0.0;

  if ( t_slice_end_.tv_sec != 0 )
  {
    long t_real_s = ( t_slice_end_.tv_sec - t_slice_begin_.tv_sec ) * 1e6;   // usec
    t_real_ += t_real_s + ( t_slice_end_.tv_usec - t_slice_begin_.tv_usec ); // usec
    double_t t_real_acc = ( t_real_ ) / 1000.;                               // ms
    double_t t_sim_acc = ( to_do_total_ - to_do_ ) * Time::get_resolution().get_ms();
    rt_factor = t_sim_acc / t_real_acc;
  }

  int_t percentage = ( 100 - int( float( to_do_ ) / to_do_total_ * 100 ) );

  std::cout << "\r" << std::setw( 3 ) << std::right << percentage << " %: "
            << "network time: " << std::fixed << std::setprecision( 1 ) << clock_.get_ms()
            << " ms, "
            << "realtime factor: " << std::setprecision( 4 ) << rt_factor
            << std::resetiosflags( std::ios_base::floatfield );
  std::flush( std::cout );
}

// inline
nest::Time
nest::SimulationManager::get_previous_slice_origin() const
{
  return clock_ - Time::step( kernel().connection_builder_manager.get_min_delay() );
}
