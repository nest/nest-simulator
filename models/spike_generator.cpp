/*
 *  spike_generator.cpp
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

#include "spike_generator.h"

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "exceptions.h"
#include "kernel_manager.h"

// Includes from libnestutil:
#include "dict_util.h"

// Includes from sli:
#include "arraydatum.h"
#include "booldatum.h"
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"


/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::spike_generator::Parameters_::Parameters_()
  : spike_stamps_()
  , spike_offsets_()
  , spike_weights_()
  , spike_multiplicities_()
  , precise_times_( false )
  , allow_offgrid_times_( false )
  , shift_now_spikes_( false )
{
}

nest::spike_generator::Parameters_::Parameters_( const Parameters_& op )
  : spike_stamps_( op.spike_stamps_ )
  , spike_offsets_( op.spike_offsets_ )
  , spike_weights_( op.spike_weights_ )
  , spike_multiplicities_( op.spike_multiplicities_ )
  , precise_times_( op.precise_times_ )
  , allow_offgrid_times_( op.allow_offgrid_times_ )
  , shift_now_spikes_( op.shift_now_spikes_ )
{
}

nest::spike_generator::State_::State_()
  : position_( 0 )
{
}


/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::spike_generator::Parameters_::get( DictionaryDatum& d ) const
{
  const size_t n_spikes = spike_stamps_.size();
  const size_t n_offsets = spike_offsets_.size();

  assert( ( precise_times_ && n_offsets == n_spikes ) || ( not precise_times_ && n_offsets == 0 ) );


  std::vector< double >* times_ms = new std::vector< double >();
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
  ( *d )[ names::spike_weights ] = DoubleVectorDatum( new std::vector< double >( spike_weights_ ) );
  ( *d )[ names::spike_multiplicities ] = IntVectorDatum( new std::vector< long >( spike_multiplicities_ ) );
  ( *d )[ names::precise_times ] = BoolDatum( precise_times_ );
  ( *d )[ names::allow_offgrid_times ] = BoolDatum( allow_offgrid_times_ );
  ( *d )[ names::shift_now_spikes ] = BoolDatum( shift_now_spikes_ );
}

void
nest::spike_generator::Parameters_::assert_valid_spike_time_and_insert_( double t, const Time& origin, const Time& now )
{
  if ( t == 0.0 && not shift_now_spikes_ )
  {
    throw BadProperty( "spike time cannot be set to 0." );
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
        msg << "spike_generator: Time point " << t << " is not representable in current resolution.";
        throw BadProperty( msg.str() );
      }
    }

    assert( t_spike.is_grid_time() );
    if ( origin + t_spike == now && shift_now_spikes_ )
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
      || ( std::fabs( offset ) < std::numeric_limits< double >::min() ) )
    {
      // if difference is smaller than scaled epsilon it is zero
      offset = 0.0;
    }
    assert( offset >= 0.0 );
    spike_offsets_.push_back( offset );
  }
}

void
nest::spike_generator::Parameters_::set( const DictionaryDatum& d,
  State_& s,
  const Time& origin,
  const Time& now,
  Node* node )
{
  bool flags_changed = updateValueParam< bool >( d, names::precise_times, precise_times_, node )
    or updateValueParam< bool >( d, names::shift_now_spikes, shift_now_spikes_, node )
    or updateValueParam< bool >( d, names::allow_offgrid_times, allow_offgrid_times_, node );
  if ( precise_times_ && ( allow_offgrid_times_ || shift_now_spikes_ ) )
  {
    throw BadProperty(
      "Option precise_times cannot be set to true when either "
      "allow_offgrid_times or shift_now_spikes is set to true." );
  }

  const bool updated_spike_times = d->known( names::spike_times );
  if ( flags_changed && not( updated_spike_times || spike_stamps_.empty() ) )
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
      std::vector< double >::const_iterator prev = d_times.begin();
      assert_valid_spike_time_and_insert_( *prev, origin, now );

      // handle all remaining spike times, compare to predecessor
      for ( std::vector< double >::const_iterator next = prev + 1; next != d_times.end(); ++next, ++prev )
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

  // spike_weights can be the same size as spike_times, or can be of size 0 to
  // only use the spike_times array
  bool updated_spike_weights = d->known( names::spike_weights );
  if ( updated_spike_weights )
  {
    std::vector< double > spike_weights = getValue< std::vector< double > >( d->lookup( names::spike_weights ) );

    if ( spike_weights.empty() )
    {
      spike_weights_.clear();
    }
    else
    {
      if ( spike_weights.size() != spike_stamps_.size() )
      {
        throw BadProperty(
          "spike_weights must have the same number of elements as spike_times,"
          " or 0 elements to clear the property." );
      }

      spike_weights_.swap( spike_weights );
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
  if ( updated_spike_times || updated_spike_weights || updated_spike_multiplicities || d->known( names::origin ) )
  {
    s.position_ = 0;
  }
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::spike_generator::spike_generator()
  : DeviceNode()
  , device_()
  , P_()
  , S_()
{
}

nest::spike_generator::spike_generator( const spike_generator& n )
  : DeviceNode( n )
  , device_( n.device_ )
  , P_( n.P_ )
  , S_( n.S_ )
{
}


/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::spike_generator::init_state_( const Node& proto )
{
  const spike_generator& pr = downcast< spike_generator >( proto );

  device_.init_state( pr.device_ );
  S_ = pr.S_;
}

void
nest::spike_generator::init_buffers_()
{
  device_.init_buffers();
}

void
nest::spike_generator::calibrate()
{
  device_.calibrate();
}


/* ----------------------------------------------------------------
 * Other functions
 * ---------------------------------------------------------------- */

void
nest::spike_generator::update( Time const& sliceT0, const long from, const long to )
{
  if ( P_.spike_stamps_.empty() )
  {
    return;
  }

  assert( not P_.precise_times_ || P_.spike_stamps_.size() == P_.spike_offsets_.size() );
  assert( P_.spike_weights_.empty() || P_.spike_stamps_.size() == P_.spike_weights_.size() );
  assert( P_.spike_multiplicities_.empty() || P_.spike_stamps_.size() == P_.spike_multiplicities_.size() );

  const Time tstart = sliceT0 + Time::step( from );
  const Time tstop = sliceT0 + Time::step( to );
  const Time& origin = device_.get_origin();

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

    if ( device_.is_active( tnext_stamp ) )
    {
      SpikeEvent* se;

      // if we have to deliver weighted spikes, we need to get the
      // event back to set its weight according to the entry in
      // spike_weights_, so we use a DSSpike event and event_hook()
      if ( not P_.spike_weights_.empty() )
      {
        se = new DSSpikeEvent;
      }
      else
      {
        se = new SpikeEvent;
      }

      if ( P_.precise_times_ )
      {
        se->set_offset( P_.spike_offsets_[ S_.position_ ] );
      }

      if ( not P_.spike_multiplicities_.empty() )
      {
        se->set_multiplicity( P_.spike_multiplicities_[ S_.position_ ] );
      }

      // we need to subtract one from stamp which is added again in send()
      long lag = Time( tnext_stamp - sliceT0 ).get_steps() - 1;

      // all spikes are sent locally, so offset information is always preserved
      kernel().event_delivery_manager.send( *this, *se, lag );
      delete se;
    }

    ++S_.position_;
  }
}

void
nest::spike_generator::event_hook( DSSpikeEvent& e )
{
  e.set_weight( P_.spike_weights_[ S_.position_ ] * e.get_weight() );
  e.get_receiver().handle( e );
}

// inline
void
nest::spike_generator::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors

  // To detect "now" spikes and shift them, we need the origin. In case
  // it is set in this call, we need to extract it explicitly here.
  Time origin;
  double v;
  if ( updateValue< double >( d, names::origin, v ) )
  {
    origin = Time::ms( v );
  }
  else
  {
    origin = device_.get_origin();
  }

  // throws if BadProperty
  ptmp.set( d, S_, origin, kernel().simulation_manager.get_time(), this );

  // We now know that ptmp is consistent. We do not write it back
  // to P_ before we are also sure that the properties to be set
  // in the parent class are internally consistent.
  device_.set_status( d );

  // if we get here, temporary contains consistent set of properties
  P_ = ptmp;
}
