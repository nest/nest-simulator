/*
 *  lowpassfilter_spike_detector.cpp
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

#include "lowpassfilter_spike_detector.h"

// C++ includes:
#include <numeric>

// Includes from libnestutil:
#include "compose.hpp"
#include "logging.h"

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "kernel_manager.h"
#include "sibling_container.h"

// Includes from sli:
#include "arraydatum.h"
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"

nest::lowpassfilter_spike_detector::lowpassfilter_spike_detector()
  : Node()
  , user_set_precise_times_( false )
  , has_proxies_( false )
  , local_receiver_( true )
  , spikes_device_( *this, RecordingDevice::SPIKE_DETECTOR, "gdf", true, true )
  , filtered_device_( *this,
      RecordingDevice::SPIKE_DETECTOR,
      "gdf",
      true,
      true )
  , P_()
  , S_()
  , V_()
  , B_()
{
}

nest::lowpassfilter_spike_detector::lowpassfilter_spike_detector(
  const lowpassfilter_spike_detector& n )
  : Node( n )
  , user_set_precise_times_( n.user_set_precise_times_ )
  , has_proxies_( false )
  , local_receiver_( true )
  , spikes_device_( *this, n.spikes_device_ )
  , filtered_device_( *this, n.filtered_device_ )
  , P_( n.P_ )
  , S_()
  , V_()
  , B_()
{
}

nest::lowpassfilter_spike_detector::Parameters_::Parameters_()
  : record_spikes_( false )
  , tau_( 30.0 )
  , filter_start_times_()
  , filter_stop_times_()
  , filter_report_interval_( Time::ms( 1.0 ) )
{
}

nest::lowpassfilter_spike_detector::Parameters_::Parameters_(
  const Parameters_& p )
  : record_spikes_( p.record_spikes_ )
  , tau_( p.tau_ )
  , filter_start_times_( p.filter_start_times_ )
  , filter_stop_times_( p.filter_stop_times_ )
  , filter_report_interval_( p.filter_report_interval_ )
{
  filter_report_interval_.calibrate();
}

nest::lowpassfilter_spike_detector::State_::State_()
{
}

void
nest::lowpassfilter_spike_detector::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::filter_start_times ] = filter_start_times_;
  ( *d )[ names::filter_stop_times ] = filter_stop_times_;
  ( *d )[ names::filter_report_interval ] = filter_report_interval_.get_ms();
  ( *d )[ names::tau_filter ] = tau_;
  ( *d )[ names::record_spikes ] = record_spikes_;
}

nest::lowpassfilter_spike_detector::Variables_::Variables_()
  : filter_block_index_( 0 )
{
}

void
nest::lowpassfilter_spike_detector::init_state_( const Node& np )
{
  const lowpassfilter_spike_detector& sd =
    dynamic_cast< const lowpassfilter_spike_detector& >( np );
  spikes_device_.init_state( sd.spikes_device_ );
  filtered_device_.init_state( sd.filtered_device_ );

  init_buffers_();
}

void
nest::lowpassfilter_spike_detector::init_buffers_()
{
  spikes_device_.init_buffers();
  filtered_device_.init_buffers();

  std::vector< double > tmp_filter_times( 0 );
  S_.filter_times_.swap( tmp_filter_times );

  std::vector< std::vector< double > > tmp_node_traces(
    S_.node_gids_.size(), std::vector< double >() );
  S_.node_traces_.swap( tmp_node_traces );

  // Size is known.
  std::vector< double > tmp_trace_times( S_.node_gids_.size(), 0.0 );
  B_.trace_times_.swap( tmp_trace_times );

  // Size is known.
  std::vector< double > tmp_traces( S_.node_gids_.size(), 0.0 );
  B_.traces_.swap( tmp_traces );

  std::vector< long > tmp_steps_to_filter( 0 );
  B_.steps_to_filter_.swap( tmp_steps_to_filter );

  std::vector< std::vector< std::vector< Event* > > > tmp_node_spikes(
    2,
    std::vector< std::vector< Event* > >(
      S_.node_gids_.size(), std::vector< Event* >() ) );
  B_.node_spikes_.swap( tmp_node_spikes );
}

void
nest::lowpassfilter_spike_detector::calibrate()
{
  if ( !user_set_precise_times_
    && kernel().event_delivery_manager.get_off_grid_communication() )
  {
    spikes_device_.set_precise( true, 15 );

    LOG( M_INFO,
      "lowpassfilter_spike_detector:calibrate",
      String::compose(
           "Precise neuron models exist: the property precise_times "
           "of the %1 with gid %2 has been set to true, precision has "
           "been set to 15.",
           get_name(),
           get_gid() ) );
  }

  if ( !user_set_precise_times_
    && kernel().event_delivery_manager.get_off_grid_communication() )
  {
    LOG( M_INFO,
      "lowpassfilter_spike_detector::calibrate",
      String::compose(
           "Precise neuron models exist: this version is not made to calculate"
           "traces for precise models.",
           get_name(),
           get_gid() ) );
  }

  if ( P_.filter_start_times_.size() == 0 )
  {
    LOG(
      M_INFO,
      "lowpassfilter_spike_detector::calibrate",
      String::compose(
        "Properties filter_start_times and filter_stop_times not specified. ",
        get_name(),
        get_gid() ) );
  }

  spikes_device_.calibrate();
  filtered_device_.calibrate();
}

void
nest::lowpassfilter_spike_detector::print_value( long sender,
  double time,
  double value )
{
  filtered_device_.print_value( sender, false );
  filtered_device_.print_value( time, false );
  filtered_device_.print_value( value, true );
}

long
nest::lowpassfilter_spike_detector::filter_step_( long update_start )
{
  // This method returns the first step to report based on the current filter
  // block.

  long interval_step = P_.filter_report_interval_.get_steps();
  long filterblock_start_step =
    Time( Time::ms( P_.filter_start_times_[ V_.filter_block_index_ ] ) )
      .get_steps();
  long filter_step;

  if ( filterblock_start_step + interval_step > update_start )
  {
    filter_step = filterblock_start_step + interval_step;
  }

  else
  {
    filter_step = filterblock_start_step + interval_step
      + std::ceil( ( ( double )( ( update_start + 1 )
                       - ( filterblock_start_step + interval_step ) )
          / interval_step ) ) * interval_step;

    if ( ( filter_step - ( filterblock_start_step + interval_step ) )
        % interval_step
      != 0 )
    {
      filter_step = filter_step - ( filter_step % interval_step );
    }
  }

  return filter_step;
}

double
nest::lowpassfilter_spike_detector::add_impulse_( long node )
{
  B_.traces_[ node ] += 1.0 / P_.tau_;
  return B_.traces_[ node ];
}

double
nest::lowpassfilter_spike_detector::calculate_decay_( long node,
  double to_time )
{
  B_.traces_[ node ] =
    exp( ( B_.trace_times_[ node ] - to_time ) / P_.tau_ ) * B_.traces_[ node ];
  B_.trace_times_[ node ] = to_time;
  return B_.traces_[ node ];
}

void
nest::lowpassfilter_spike_detector::update( Time const& origin,
  const long from,
  const long to )
{

  /**
   *
   * Simulations progress in cycles defined by the minimum delay but the
   * filter_report_interval can be lower than the minimum delay. So it
   * is possible that multiple filter blocks fall within an update
   * window.
   *
   * There are three stages:
   * Stage 1: Finding out all the steps to report within this update window.
   * Finding out the recording steps (RS), steps that have to be reported
   * within this update window.
   *
   * Stage 2: Iterating over all nodes and events for each node and filtering
   * the data. Two parts are completed in each iteration.
   * Part 1. Adding to the buffer the trace for all RS that are equal to or
   * lower than the latest spike's step for each node.
   * Part 2. After all events are processed for a particular node, recording
   * any RS that exceeds the latest event's step for each node.
   *
   * Stage 3: Recording filtered data.
   * Committing the processed data to memory or printing it to file/screen.
   */

  /**
   *
   * Stage 1: Finding out all the steps to report within this update window.
   * Finding out the recording steps (RS), steps that have to be reported within
   * this update window.
   */
  if ( B_.steps_to_filter_.size() == 0 && origin.get_steps() != 0
    && ( ( origin.get_steps() + from )
           % kernel().connection_manager.get_min_delay()
         == 0 ) )
  {

    long filter_step;
    bool filter_steps_exist = false;

    // If the statement below is true,
    // there is at least one filter block in the remainder of the simulation.
    if ( V_.filter_block_index_ < P_.filter_start_times_.size() )
    {
      filter_step = filter_step_(
        origin.get_steps() - kernel().connection_manager.get_min_delay() );
      if ( filter_step
          > ( origin.get_steps() - kernel().connection_manager.get_min_delay() )
        && filter_step <= origin.get_steps() )
      {
        filter_steps_exist = true;
      }
    }

    if ( filter_steps_exist )
    {

      // If there are steps in filter block(s) that are within this update
      // window:
      while (
        ( filter_step
          <= Time( Time::ms( P_.filter_stop_times_[ V_.filter_block_index_ ] ) )
               .get_steps() ) && filter_step <= ( origin.get_steps() ) )
      {
        B_.steps_to_filter_.push_back( filter_step );
        filter_step += P_.filter_report_interval_.get_steps();

        if ( filter_step
          > Time( Time::ms( P_.filter_stop_times_[ V_.filter_block_index_ ] ) )
              .get_steps() )
        {
          // Every time the simulation time exceeds filter_stop of a filtering
          // block, V_.filter_block_index_ is incremented.
          V_.filter_block_index_ += 1;

          // If true, the remaining filter block(s) are not in this update
          // window.
          if ( filter_step > origin.get_steps() )
          {
            break;
          }

          // If true, there are no more filter blocks to process in the
          // remainder of the simulation.
          if ( V_.filter_block_index_ >= P_.filter_start_times_.size() )
          {
            break;
          }

          filter_step = filter_step_(
            origin.get_steps() - kernel().connection_manager.get_min_delay() );
        }
      }
    }
  }

  /**
   *
   * Stage 2: Iterating over all nodes and events for each node and filtering
   * the data.
   * All spikes are also recorded in this stage if /record_spikes is true.
   */
  std::vector< std::vector< nest::Event* > >::iterator node_iter;
  long node_idx;
  for (
    node_iter =
      B_.node_spikes_[ kernel().event_delivery_manager.read_toggle() ].begin(),
    node_idx = 0;
    node_iter
      != B_.node_spikes_[ kernel().event_delivery_manager.read_toggle() ].end();
    ++node_iter, ++node_idx )
  {

    /**
     *
     * Stage 2 (Part 1): Iterating over all nodes and events for each node and
     * recording all RS
     * that are equal to or lower than the latest spike's step for each node.
     */
    size_t report_step_idx = 0; // Stores position in steps buffer.
    bool addition = false;      // Stores whether steps pushed.
    for ( std::vector< Event* >::iterator event_iter = ( *node_iter ).begin();
          event_iter != ( *node_iter ).end();
          ++event_iter )
    {
      assert( *event_iter != 0 );


      if ( B_.steps_to_filter_.size() == 0 )
      {
        // If there are no steps to report within this interval, it is not
        // necessary to record anything.
        // But it is still necessary to calculate the trace.
        calculate_decay_( node_idx, ( *event_iter )->get_stamp().get_ms() );
        add_impulse_( node_idx );
      }

      else
      {
        // If there are steps to report within this interval, they must be
        // recorded.

        if ( ( *event_iter )->get_stamp().get_steps()
          == Time( Time::ms( B_.trace_times_[ node_idx ] ) ).get_steps() )
        {
          // if multiple spikes happened at the same step, a fast procedure can
          // be used.

          if ( B_.steps_to_filter_[ report_step_idx - addition ]
            == ( *event_iter )->get_stamp().get_steps() )
          {
            S_.node_traces_[ node_idx ].back() = add_impulse_( node_idx );
          }
          else
            add_impulse_( node_idx );
        }

        else
        {

          addition = false;
          if ( report_step_idx < B_.steps_to_filter_.size() )
          {
            while ( B_.steps_to_filter_[ report_step_idx ]
              <= ( *event_iter )->get_stamp().get_steps() )
            {
              long step_to_report = B_.steps_to_filter_[ report_step_idx ];
              S_.node_traces_[ node_idx ].push_back( calculate_decay_(
                node_idx, Time( Time::step( step_to_report ) ).get_ms() ) );
              report_step_idx++;
              addition = true;
              if ( report_step_idx >= B_.steps_to_filter_.size() )
              {
                break;
              }
            }
          }

          calculate_decay_( node_idx, ( *event_iter )->get_stamp().get_ms() );
          add_impulse_( node_idx );
          if ( B_.steps_to_filter_[ report_step_idx - addition ]
            == ( *event_iter )->get_stamp().get_steps() )
          {
            S_.node_traces_[ node_idx ].back() = B_.traces_[ node_idx ];
          }
        }
      }

      // record spike event (if the device is supposed to work as a
      // spike_detector).
      if ( P_.record_spikes_ )
      {
        spikes_device_.record_event( **event_iter );
      }

      // Deleting the event
      delete ( *event_iter );
    }

    /**
     *
     * Stage 2 (Part 2): After all events are processed for a particular node,
     * recording any RS that exceeds the latest event's step for each node.
     */
    if ( B_.steps_to_filter_.size() != 0
      && ( ( origin.get_steps() + from )
             % kernel().connection_manager.get_min_delay()
           == 0 ) )
    {
      // If true, there exists at least one step for which the trace must yet be
      // calculated and reported.
      if ( B_.steps_to_filter_.back()
        > Time( Time::ms( B_.trace_times_[ node_idx ] ) ).get_steps() )
      {
        // If there is only one element in steps_to_filter_
        if ( B_.steps_to_filter_.size() == 1 )
        {
          S_.node_traces_[ node_idx ].push_back( calculate_decay_( node_idx,
            Time( Time::step( B_.steps_to_filter_.back() ) ).get_ms() ) );
        }

        else
        {
          std::vector< long >::iterator begin_it =
            std::upper_bound( B_.steps_to_filter_.begin(),
              B_.steps_to_filter_.end(),
              Time( Time::ms( B_.trace_times_[ node_idx ] ) ).get_steps() );

          if ( Time( Time::ms( B_.trace_times_[ node_idx ] ) ).get_steps()
            < B_.steps_to_filter_[ 0 ] )
          {
            begin_it = B_.steps_to_filter_.begin();
          }

          while ( begin_it != B_.steps_to_filter_.end() )
          {
            // Calculating decay
            S_.node_traces_[ node_idx ].push_back( calculate_decay_(
              node_idx, Time( Time::step( *begin_it ) ).get_ms() ) );
            begin_it++;
          }
        }
      }
    }

    // Do not use swap here to clear, since we want to keep the reserved()
    // memory for the next round
    ( *node_iter ).clear();
  }

  /**
   *
   * Stage 3: Recording filtered data.
   * Committing the processed data to memory or printing it to file/screen.
   */
  if ( B_.steps_to_filter_.size() != 0
    && ( ( origin.get_steps() + from )
           % kernel().connection_manager.get_min_delay()
         == 0 ) )
  {

    if ( filtered_device_.to_file() || filtered_device_.to_screen() )
    {
      // For memory/screen recording

      for ( size_t i = 0; i < S_.node_gids_.size(); ++i )
      {
        for ( size_t j = 0; j < B_.steps_to_filter_.size(); ++j )
        {
          // If to_memory is enabled, S_.node_traces_[i] does not get cleared.
          // and it is necessary to iterate starting from the index that is
          // relevant. Hence the
          // j + S_.node_traces_[i].size() - B_.steps_to_filter_.size()
          print_value( S_.node_gids_[ i ],
            Time( Time::step( B_.steps_to_filter_[ j ] ) ).get_ms(),
            S_.node_traces_[ i ][ j + S_.node_traces_[ i ].size()
              - B_.steps_to_filter_.size() ] );
        }
      }
    }

    if ( filtered_device_.to_memory() )
    {
      // Adding all the filter step times within this update window to the
      // array.
      for ( std::vector< long >::iterator filter_step_iter =
              B_.steps_to_filter_.begin();
            filter_step_iter != B_.steps_to_filter_.end();
            ++filter_step_iter )
      {
        S_.filter_times_.push_back(
          Time( Time::step( *filter_step_iter ) ).get_ms() );
      }
    }

    else
    {
      // If it is not necessary to record to memory, buffers can be cleared to
      // save memory.
      for ( size_t i = 0; i < S_.node_gids_.size(); ++i )
      {
        // Do not use swap here to clear, since we want to keep the reserved()
        // memory for the next round
        S_.node_traces_[ i ].clear();
      }

      // Do not use swap here to clear, since we want to keep the reserved()
      // memory for the next round
      S_.filter_times_.clear();
    }

    // Do not use swap here to clear, since we want to keep the reserved()
    // memory for the next round
    B_.steps_to_filter_.clear();
  }
}

void
nest::lowpassfilter_spike_detector::get_status( DictionaryDatum& d ) const
{
  spikes_device_.get_status( d );

  // filter results array
  DictionaryDatum filter_events;
  if ( !d->known( names::filter_events ) )
  {
    filter_events = DictionaryDatum( new Dictionary );
  }
  else
  {
    filter_events = getValue< DictionaryDatum >( d, names::filter_events );
  }

  initialize_property_doublevector( filter_events, names::filter_values );
  initialize_property_doublevector( filter_events, names::times );
  initialize_property_intvector( filter_events, names::senders );

  size_t nr = S_.filter_times_.size() * S_.node_traces_.size();
  std::vector< double > filter_values( nr );
  std::vector< double > filter_times( nr );
  std::vector< long > senders( nr );

  // To maintain consistency with how recording to file/screen is done.
  for ( size_t i = 0; i < S_.filter_times_.size(); ++i )
  {
    for ( size_t j = 0; j < S_.node_traces_.size(); ++j )
    {
      size_t idx = ( i * S_.node_traces_.size() ) + j;

      filter_values[ idx ] = S_.node_traces_[ j ][ i ];
      filter_times[ idx ] = S_.filter_times_[ i ];
      senders[ idx ] = S_.node_gids_[ j ];
    }
  }

  append_property( filter_events, names::filter_values, filter_values );
  append_property( filter_events, names::times, filter_times );
  append_property( filter_events, names::senders, senders );

  ( *d )[ names::filter_events ] = filter_events;

  // if we are the device on thread 0, also get the data from the
  // siblings on other threads
  if ( local_receiver_ && get_thread() == 0 )
  {
    const SiblingContainer* siblings =
      kernel().node_manager.get_thread_siblings( get_gid() );
    std::vector< Node* >::const_iterator sibling;
    for ( sibling = siblings->begin() + 1; sibling != siblings->end();
          ++sibling )
    {
      ( *sibling )->get_status( d );
    }

    P_.get( d );
  }
}

void
nest::lowpassfilter_spike_detector::set_status( const DictionaryDatum& d )
{

  if ( d->known( names::precise_times ) )
  {
    user_set_precise_times_ = true;
  }

  updateValue< bool >( d, names::record_spikes, P_.record_spikes_ );

  if ( d->known( names::filter_report_interval ) )
  {
    double v;
    if ( updateValue< double >( d, names::filter_report_interval, v ) )
    {
      if ( Time( Time::ms( v ) ) < Time::get_resolution() )
      {
        throw BadProperty(
          "The filter_report_interval must be at least as long "
          "as the simulation resolution." );
      }

      // see if we can represent interval as multiple of step
      P_.filter_report_interval_ =
        Time::step( Time( Time::ms( v ) ).get_steps() );
      if ( std::abs( 1 - P_.filter_report_interval_.get_ms() / v ) > 10
          * std::numeric_limits< double >::epsilon() )
      {
        throw BadProperty(
          "The filter_report_interval must be a multiple of "
          "the simulation resolution" );
      }
    }
  }

  if ( d->known( names::filter_start_times )
    && d->known( names::filter_stop_times ) )
  {
    P_.filter_start_times_ =
      getValue< std::vector< double > >( d, names::filter_start_times );
    P_.filter_stop_times_ =
      getValue< std::vector< double > >( d, names::filter_stop_times );
  }

  // Checking to see if filter start and stop time sizes match.
  if ( P_.filter_start_times_.size() != P_.filter_stop_times_.size() )
  {
    throw BadProperty(
      "The number of elements in \"filter_start_times\" and "
      "\"filter_stop_times\" must match." );
  }

  bool invalid_blocks = false;
  std::vector< size_t > indices_to_remove;
  // filter start and stop times must be ordered:
  for ( size_t i = 0; i < P_.filter_start_times_.size(); ++i )
  {

    // Checking to see if filter blocks are ordered.
    if ( Time::delay_ms_to_steps( P_.filter_start_times_[ i ] )
      > Time::delay_ms_to_steps( P_.filter_stop_times_[ i ] ) )
    {
      throw BadProperty(
        "Each element in \"filter_start_times\" must be lower than "
        "its corresponding value in \"filter_stop_times\"" );
    }

    if ( i != P_.filter_start_times_.size() - 1 )
    {

      // Checking to see if filter_start_times are ordered.
      if ( Time::delay_ms_to_steps( P_.filter_start_times_[ i ] )
        > Time::delay_ms_to_steps( P_.filter_start_times_[ i + 1 ] ) )
      {
        throw BadProperty(
          "\"filter_start_times\" is not in ascending order. "
          "\"filter_start_times\" and \"filter_stop_times\" must be in "
          "ascending order." );
      }

      // Checking to see if filter_stop_times are ordered.
      if ( Time::delay_ms_to_steps( P_.filter_stop_times_[ i ] )
        > Time::delay_ms_to_steps( P_.filter_stop_times_[ i + 1 ] ) )
      {
        throw BadProperty(
          "\"filter_stop_times\" is not in ascending order. "
          "\"filter_start_times\" and \"filter_stop_times\" must be in "
          "ascending order." );
      }

      // Checking to see if filter blocks overlap.
      if ( Time::delay_ms_to_steps( P_.filter_stop_times_[ i ] )
        > Time::delay_ms_to_steps( P_.filter_start_times_[ i + 1 ] ) )
      {
        throw BadProperty(
          "The filter blocks specified using \"filter_start_times\" and "
          "\"filter_stop_times\" "
          "must not overlap." );
      }
    }

    // Checking to see if there are block(s) that are too small for any form of
    // recording to happen.
    if ( ( Time::delay_ms_to_steps( P_.filter_stop_times_[ i ] )
           - Time::delay_ms_to_steps( P_.filter_start_times_[ i ] ) )
      < P_.filter_report_interval_.get_steps() )
    {
      indices_to_remove.push_back( i );
      invalid_blocks = true;
    }
  }

  for ( size_t i = 0; i < indices_to_remove.size(); i++ )
  {
    std::vector< double >::iterator start_it =
      P_.filter_start_times_.begin() + indices_to_remove[ i ] - i;
    P_.filter_start_times_.erase( start_it );

    std::vector< double >::iterator stop_it =
      P_.filter_stop_times_.begin() + indices_to_remove[ i ] - i;
    P_.filter_stop_times_.erase( stop_it );
  }

  if ( invalid_blocks )
  {
    LOG( M_INFO,
      "lowpassfilter_spike_detector::set_status",
      String::compose(
           "%1 invalid filter block(s) were found and removed for %2 with gid "
           "%3. The difference "
           "between each element of \"filter_start_times\" to its "
           "corresponding element in \"filter_stop_times\" should be at "
           "least one \"filter_report_interval_\" long for anything to be "
           "recorded "
           "in that block.",
           indices_to_remove.size(),
           get_name(),
           get_gid() ) );
  }

  if ( P_.filter_start_times_.size() != P_.filter_stop_times_.size() )
  {
    throw BadProperty(
      "The size of \"filter_start_times\" and "
      "\"filter_stop_times\" properties must match." );
  }

  updateValue< double >( d, names::tau_filter, P_.tau_ );

  spikes_device_.set_status( d );

  // Creating a dictionary for the recording_device recording filtered activity.
  DictionaryDatum f_d = DictionaryDatum( d );

  // Setting a custom label for the filtered recordings.
  if ( d->known( names::label ) )
  {
    ( *f_d )[ names::label ] =
      getValue< std::string >( d, names::label ) + "-filtered";
  }
  else
  {
    ( *f_d )[ names::label ] = "spike_detector-filtered";
  }

  // Setting the file_extension of filtered recordings to dat if it's not
  // specified.
  if ( !d->known( names::file_extension ) )
  {
    ( *f_d )[ names::file_extension ] = "dat";
  }

  filtered_device_.set_status( f_d );

  // If n_filter_events is set to 0, all data on memory should be cleared.
  long ne = 0;
  if ( updateValue< long >( d, names::n_events, ne ) )
  {
    if ( ne == 0 )
    {
      S_.filter_times_.clear();

      for ( size_t i = 0; i < S_.node_traces_.size(); ++i )
      {
        S_.node_traces_[ i ].clear();
      }
    }

    else
    {
      throw BadProperty( "n_events can only be set to 0." );
    }
  }
}

void
nest::lowpassfilter_spike_detector::handle( SpikeEvent& e )
{
  // accept spikes only if detector was active when spike was
  // emitted
  if ( filtered_device_.is_active( e.get_stamp() ) )
  {
    assert( e.get_multiplicity() > 0 );

    long dest_buffer;
    if ( kernel()
           .modelrange_manager.get_model_of_gid( e.get_sender_gid() )
           ->has_proxies() )
    {
      // events from central queue
      dest_buffer = kernel().event_delivery_manager.read_toggle();
    }
    else
    {
      // locally delivered events
      dest_buffer = kernel().event_delivery_manager.write_toggle();
    }

    for ( int i = 0; i < e.get_multiplicity(); ++i )
    {
      // We store the complete events
      Event* event = e.clone();
      B_.node_spikes_[ dest_buffer ][ event->get_rport() ].push_back( event );
    }
  }
}
