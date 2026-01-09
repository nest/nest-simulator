/*
 *  spike_train_injector.cpp
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

#include "spike_train_injector.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "genericmodel_impl.h"
#include "kernel_manager.h"
#include "nest_impl.h"
#include "universal_data_logger_impl.h"

// Includes from libnestutil:
#include "dict_util.h"
#include "event_delivery_manager_impl.h"

// Includes from sli:
#include "arraydatum.h"
#include "booldatum.h"
#include "dict.h"
#include "dictutils.h"

namespace nest
{
void
register_spike_train_injector( const std::string& name )
{
  register_node_model< spike_train_injector >( name );
}


/* ----------------------------------------------------------------
 * Default constructor defining default parameters
 * ---------------------------------------------------------------- */

spike_train_injector::Parameters_::Parameters_()
  : spike_stamps_()
  , spike_offsets_()
  , spike_multiplicities_()
  , precise_times_( false )
  , allow_offgrid_times_( false )
  , shift_now_spikes_( false )
{
}


/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
spike_train_injector::Parameters_::get( DictionaryDatum& d ) const
{
  const size_t n_spikes = spike_stamps_.size();
  auto* times_ms = new std::vector< double >();
  times_ms->reserve( n_spikes );

  for ( size_t n = 0; n < n_spikes; ++n )
  {
    times_ms->push_back( spike_stamps_[ n ].get_ms() );
    if ( precise_times_ )
    {
      ( *times_ms )[ n ] -= spike_offsets_[ n ];
    }
  }

  ( *d )[ names::spike_times ] = DoubleVectorDatum( times_ms );
  ( *d )[ names::spike_multiplicities ] = IntVectorDatum( new std::vector< long >( spike_multiplicities_ ) );
  ( *d )[ names::precise_times ] = BoolDatum( precise_times_ );
  ( *d )[ names::allow_offgrid_times ] = BoolDatum( allow_offgrid_times_ );
  ( *d )[ names::shift_now_spikes ] = BoolDatum( shift_now_spikes_ );
}

void
spike_train_injector::Parameters_::assert_valid_spike_time_and_insert_( double t, const Time& origin, const Time& now )
{
  if ( t == 0.0 and not shift_now_spikes_ )
  {
    throw BadProperty( "Spike time cannot be set to 0." );
  }

  Time t_spike;
  if ( precise_times_ )
  {
    t_spike = Time::ms_stamp( t );
  }
  else
  {
    // In this case, we need to force the spike time to the grid

    // First, convert the spike time to tics, may not be on grid
    t_spike = Time::ms( t );
    if ( not t_spike.is_grid_time() )
    {
      if ( allow_offgrid_times_ )
      {
        // In this case, we need to round to the end of the step
        // in which t lies, ms_stamp does that for us.
        t_spike = Time::ms_stamp( t );
      }
      else
      {
        std::stringstream msg;
        msg << "spike_train_injector: Time point " << t << " is not representable in current resolution.";
        throw BadProperty( msg.str() );
      }
    }

    assert( t_spike.is_grid_time() );
    if ( origin + t_spike == now and shift_now_spikes_ )
    {
      t_spike.advance();
    }
  }
  // t_spike is now the correct time stamp given the chosen options

  // when we get here, we know that the spike time is valid
  spike_stamps_.push_back( t_spike );
  if ( precise_times_ )
  {
    // t_spike is created with ms_stamp() that aligns the time to the next
    // resolution step, so the offset has to be greater or equal to t by
    // construction. Since subtraction of close-by floating point values is
    // not stable, we have to compare with a delta.
    double offset = t_spike.get_ms() - t;

    // The second part of the test handles subnormal values of offset.
    if ( ( std::fabs( offset ) < std::numeric_limits< double >::epsilon() * std::fabs( t_spike.get_ms() + t ) * 2.0 )
      or ( std::fabs( offset ) < std::numeric_limits< double >::min() ) )
    {
      // if difference is smaller than scaled epsilon it is zero
      offset = 0.0;
    }
    assert( offset >= 0.0 );
    spike_offsets_.push_back( offset );
  }
}

void
spike_train_injector::Parameters_::set( const DictionaryDatum& d,
  State_& s,
  const Time& origin,
  const Time& now,
  Node* node )
{
  bool precise_times_changed = updateValueParam< bool >( d, names::precise_times, precise_times_, node );
  bool shift_now_spikes_changed = updateValueParam< bool >( d, names::shift_now_spikes, shift_now_spikes_, node );
  bool allow_offgrid_times_changed =
    updateValueParam< bool >( d, names::allow_offgrid_times, allow_offgrid_times_, node );
  bool flags_changed = precise_times_changed or shift_now_spikes_changed or allow_offgrid_times_changed;
  if ( precise_times_ and ( allow_offgrid_times_ or shift_now_spikes_ ) )
  {
    throw BadProperty(
      "Option precise_times cannot be set to true when either "
      "allow_offgrid_times or shift_now_spikes is set to true." );
  }

  const bool updated_spike_times = d->known( names::spike_times );
  if ( flags_changed and not( updated_spike_times or spike_stamps_.empty() ) )
  {
    throw BadProperty(
      "Options can only be set together with spike times or if no "
      "spike times have been set." );
  }

  if ( updated_spike_times )
  {
    const std::vector< double > d_times = getValue< std::vector< double > >( d->lookup( names::spike_times ) );
    const size_t n_spikes = d_times.size();
    spike_stamps_.clear();
    spike_stamps_.reserve( n_spikes );
    spike_offsets_.clear();
    if ( precise_times_ )
    {
      spike_offsets_.reserve( n_spikes );
    }

    // Check spike times for ordering and grid compatibility and insert them
    if ( not d_times.empty() )
    {
      // handle first spike time, no predecessor to compare with
      auto prev = d_times.begin();
      assert_valid_spike_time_and_insert_( *prev, origin, now );

      // handle all remaining spike times, compare to predecessor
      for ( auto next = prev + 1; next != d_times.end(); ++next, ++prev )
      {
        if ( *prev > *next )
        {
          throw BadProperty( "Spike times must be sorted in non-descending order." );
        }
        else
        {
          assert_valid_spike_time_and_insert_( *next, origin, now );
        }
      }
    }
  }

  // spike_multiplicities can be the same size as spike_times,
  // or can be of size 0 to only use the spike_times array
  bool updated_spike_multiplicities = d->known( names::spike_multiplicities );
  if ( updated_spike_multiplicities )
  {
    std::vector< long > spike_multiplicities =
      getValue< std::vector< long > >( d->lookup( names::spike_multiplicities ) );

    if ( spike_multiplicities.empty() )
    {
      spike_multiplicities_.clear();
    }
    else
    {
      if ( spike_multiplicities.size() != spike_stamps_.size() )
      {
        throw BadProperty(
          "spike_multiplicities must have the same number of elements as "
          "spike_times or 0 elements to clear the property." );
      }

      spike_multiplicities_.swap( spike_multiplicities );
    }
  }

  // Set position to start if something changed
  if ( updated_spike_times or updated_spike_multiplicities or d->known( names::origin ) )
  {
    s.position_ = 0;
  }
}


/* ----------------------------------------------------------------
 * Default constructor defining default state
 * ---------------------------------------------------------------- */

spike_train_injector::State_::State_()
  : position_( 0 )
{
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

spike_train_injector::spike_train_injector()
  : Node()
  , Device()
  , S_()
  , P_()
{
}

spike_train_injector::spike_train_injector( const spike_train_injector& n )
  : Node( n )
  , Device( n )
  , S_( n.S_ )
  , P_( n.P_ )
{
}


/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
spike_train_injector::init_state_()
{
  Node::init_state_();
}

void
spike_train_injector::init_buffers_()
{
}

void
spike_train_injector::pre_run_hook()
{
  // We do not need to recalibrate time objects, since they are
  // recalibrated on instance construction and resolution cannot
  // change after a single node instance has been created.

  // Off-grid communication needs to be activated here since this model
  // is not an exclusive precise spiking model
  if ( is_off_grid() )
  {
    kernel::manager< EventDeliveryManager >.set_off_grid_communication( true );
    LOG( M_INFO,
      "spike_train_injector::pre_run_hook",
      "Spike train injector has been configured to emit precisely timed "
      "spikes: the kernel property off_grid_spiking has been set to true.\n\n"
      "NOTE: Mixing precise-spiking and normal neuron models may "
      "lead to inconsistent results." );
  }

  Device::pre_run_hook();
}


/* ----------------------------------------------------------------
 * Other functions
 * ---------------------------------------------------------------- */

bool
spike_train_injector::is_active( const Time& T ) const
{
  long step = T.get_steps();
  return get_t_min_() < step and step <= get_t_max_();
}

void
spike_train_injector::update( Time const& sliceT0, const long from, const long to )
{
  if ( P_.spike_stamps_.empty() )
  {
    return;
  }

  assert( not P_.precise_times_ or P_.spike_stamps_.size() == P_.spike_offsets_.size() );
  assert( P_.spike_multiplicities_.empty() or P_.spike_stamps_.size() == P_.spike_multiplicities_.size() );

  const Time tstart = sliceT0 + Time::step( from );
  const Time tstop = sliceT0 + Time::step( to );
  const Time& origin = Device::get_origin();

  // We fire all spikes with time stamps up to including sliceT0 + to
  while ( S_.position_ < P_.spike_stamps_.size() )
  {
    const Time tnext_stamp = origin + P_.spike_stamps_[ S_.position_ ];

    // this might happen due to wrong usage of the generator
    if ( tnext_stamp <= tstart )
    {
      ++S_.position_;
      continue;
    }
    if ( tnext_stamp > tstop )
    {
      break;
    }

    long step = tnext_stamp.get_steps();

    if ( get_t_min_() < step and step <= get_t_max_() )
    {
      SpikeEvent se;

      if ( P_.precise_times_ )
      {
        se.set_offset( P_.spike_offsets_[ S_.position_ ] );
      }

      if ( not P_.spike_multiplicities_.empty() )
      {
        se.set_multiplicity( P_.spike_multiplicities_[ S_.position_ ] );
      }

      // we need to subtract one from stamp which is added again in send()
      long lag = Time( tnext_stamp - sliceT0 ).get_steps() - 1;

      kernel::manager< EventDeliveryManager >.send( *this, se, lag );
    }

    ++S_.position_;
  }
}

} // namespace
