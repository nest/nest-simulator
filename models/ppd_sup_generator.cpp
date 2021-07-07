/*
 *  ppd_sup_generator.cpp
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

#include "ppd_sup_generator.h"

// C++ includes:
#include <algorithm>

// Includes from libnestutil:
#include "dict_util.h"
#include "numerics.h"

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "kernel_manager.h"

// Includes from sli:
#include "dict.h"
#include "doubledatum.h"


/* ----------------------------------------------------------------
 * Constructor of age distribution class
 * ---------------------------------------------------------------- */

nest::ppd_sup_generator::Age_distribution_::Age_distribution_( size_t num_age_bins,
  unsigned long ini_occ_ref,
  unsigned long ini_occ_act )
{
  occ_active_ = ini_occ_act;
  occ_refractory_.resize( num_age_bins, ini_occ_ref );
  activate_ = 0;
}

/* ----------------------------------------------------------------
 * Propagate age distribution one time step and generate spikes
 * ---------------------------------------------------------------- */

unsigned long
nest::ppd_sup_generator::Age_distribution_::update( double hazard_step, RngPtr rng )
{
  unsigned long n_spikes; // only set from poisson_dev, bino_dev or 0, thus >= 0
  if ( occ_active_ > 0 )
  {
    /*The binomial distribution converges towards the Poisson distribution as
    the number of trials goes to infinity while the product np remains fixed.
    Therefore the Poisson distribution with parameter \lambda = np can be used
    as an approximation to B(n, p) of the binomial distribution if n is
    sufficiently large and p is sufficiently small. According to two rules
    of thumb, this approximation is good if n >= 20 and p <= 0.05, or if
    n >= 100 and np <= 10. Source:
    http://en.wikipedia.org/wiki/Binomial_distribution#Poisson_approximation */
    if ( ( occ_active_ >= 100 && hazard_step <= 0.01 ) || ( occ_active_ >= 500 && hazard_step * occ_active_ <= 0.1 ) )
    {
      poisson_distribution::param_type param( hazard_step * occ_active_ );
      n_spikes = poisson_dist_( rng, param );
      if ( n_spikes > occ_active_ )
      {
        n_spikes = occ_active_;
      }
    }
    else
    {
      binomial_distribution::param_type param( occ_active_, hazard_step );
      n_spikes = bino_dist_( rng, param );
    }
  }
  else
  {
    n_spikes = 0;
  }

  if ( not occ_refractory_.empty() )
  {
    occ_active_ += occ_refractory_[ activate_ ] - n_spikes;
    occ_refractory_[ activate_ ] = n_spikes;
    activate_ = ( activate_ + 1 ) % occ_refractory_.size();
  }
  return n_spikes;
}


/* ----------------------------------------------------------------
 * Default constructors defining default parameter
 * ---------------------------------------------------------------- */

nest::ppd_sup_generator::Parameters_::Parameters_()
  : rate_( 0.0 )      // Hz
  , dead_time_( 0.0 ) // ms
  , n_proc_( 1 )
  , frequency_( 0.0 ) // Hz
  , amplitude_( 0.0 ) // percentage
  , num_targets_( 0 )
{
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::ppd_sup_generator::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::rate ] = rate_;
  ( *d )[ names::dead_time ] = dead_time_;
  ( *d )[ names::n_proc ] = n_proc_;
  ( *d )[ names::frequency ] = frequency_;
  ( *d )[ names::relative_amplitude ] = amplitude_;
}

void
nest::ppd_sup_generator::Parameters_::set( const DictionaryDatum& d, Node* node )
{

  updateValueParam< double >( d, names::dead_time, dead_time_, node );
  if ( dead_time_ < 0 )
  {
    throw BadProperty( "The dead time cannot be negative." );
  }

  updateValueParam< double >( d, names::rate, rate_, node );
  if ( 1000.0 / rate_ <= dead_time_ )
  {
    throw BadProperty( "The inverse rate has to be larger than the dead time." );
  }

  long n_proc_l = n_proc_;
  updateValueParam< long >( d, names::n_proc, n_proc_l, node );
  if ( n_proc_l < 1 )
  {
    throw BadProperty( "The number of component processes cannot be smaller than one" );
  }
  else
  {
    n_proc_ = static_cast< unsigned long >( n_proc_l );
  }

  updateValueParam< double >( d, names::frequency, frequency_, node );

  updateValueParam< double >( d, names::relative_amplitude, amplitude_, node );
  if ( amplitude_ > 1.0 or amplitude_ < 0.0 )
  {
    throw BadProperty( "The relative amplitude of the rate modulation must be in [0,1]." );
  }
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::ppd_sup_generator::ppd_sup_generator()
  : StimulationDevice()
  , P_()
{
}

nest::ppd_sup_generator::ppd_sup_generator( const ppd_sup_generator& n )
  : StimulationDevice( n )
  , P_( n.P_ )
{
}


/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::ppd_sup_generator::init_state_()
{
  StimulationDevice::init_state();
}

void
nest::ppd_sup_generator::init_buffers_()
{
  StimulationDevice::init_buffers();
}

void
nest::ppd_sup_generator::calibrate()
{
  StimulationDevice::calibrate();

  double h = Time::get_resolution().get_ms();

  // compute number of age bins that need to be kept track of
  unsigned long num_age_bins = static_cast< unsigned long >( P_.dead_time_ / h );

  // compute omega to evaluate modulation with, units [rad/ms]
  V_.omega_ = 2.0 * numerics::pi * P_.frequency_ / 1000.0;

  // hazard rate in units of the simulation time step.
  V_.hazard_step_ = 1.0 / ( 1000.0 / P_.rate_ - P_.dead_time_ ) * h;

  // equilibrium occupation of dead time bins (in case of constant rate)
  unsigned long ini_occ_0 = static_cast< unsigned long >( P_.rate_ / 1000.0 * P_.n_proc_ * h );

  // If new targets have been added during a simulation break, we
  // initialize the new elements in age_distributions with the initial dist. The
  // existing elements are unchanged.
  Age_distribution_ age_distribution0( num_age_bins, ini_occ_0, P_.n_proc_ - ini_occ_0 * num_age_bins );
  B_.age_distributions_.resize( P_.num_targets_, age_distribution0 );
}


/* ----------------------------------------------------------------
 * Update function and event hook
 * ---------------------------------------------------------------- */

void
nest::ppd_sup_generator::update( Time const& T, const long from, const long to )
{
  assert( to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  if ( P_.rate_ <= 0 || P_.num_targets_ == 0 )
  {
    return;
  }

  for ( long lag = from; lag < to; ++lag )
  {
    Time t = T + Time::step( lag );

    if ( not StimulationDevice::is_active( t ) )
    {
      continue; // no spike at this lag
    }

    // get current (time-dependent) hazard rate and store it.
    if ( P_.amplitude_ > 0.0 && ( P_.frequency_ > 0.0 || P_.frequency_ < 0.0 ) )
    {
      double t_ms = t.get_ms();
      V_.hazard_step_t_ = V_.hazard_step_ * ( 1.0 + P_.amplitude_ * std::sin( V_.omega_ * t_ms ) );
    }
    else
    {
      V_.hazard_step_t_ = V_.hazard_step_;
    }

    DSSpikeEvent se;
    kernel().event_delivery_manager.send( *this, se, lag );
  }
}


void
nest::ppd_sup_generator::event_hook( DSSpikeEvent& e )
{
  // get port number
  const port prt = e.get_port();

  // we handle only one port here, get reference to vector element
  assert( 0 <= prt && static_cast< size_t >( prt ) < B_.age_distributions_.size() );

  // age_distribution object propagates one time step and returns number of
  // spikes
  unsigned long n_spikes =
    B_.age_distributions_[ prt ].update( V_.hazard_step_t_, get_vp_specific_rng( get_thread() ) );

  if ( n_spikes > 0 ) // we must not send events with multiplicity 0
  {
    e.set_multiplicity( n_spikes );
    e.get_receiver().handle( e );
  }
}

/* ----------------------------------------------------------------
 * Other functions
 * ---------------------------------------------------------------- */

void
nest::ppd_sup_generator::set_data_from_stimulation_backend( std::vector< double >& input_param )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors

  // For the input backend
  if ( not input_param.empty() )
  {
    if ( input_param.size() != 5 )
    {
      throw BadParameterValue(
        "The size of the data for the ppd_sup_generator needs to be 5 "
        "[dead_time, rate, n_proc, frequency, relative_amplitude]." );
    }
    DictionaryDatum d = DictionaryDatum( new Dictionary );
    ( *d )[ names::dead_time ] = DoubleDatum( input_param[ 0 ] );
    ( *d )[ names::rate ] = DoubleDatum( input_param[ 1 ] );
    ( *d )[ names::n_proc ] = DoubleDatum( input_param[ 2 ] );
    ( *d )[ names::frequency ] = DoubleDatum( input_param[ 3 ] );
    ( *d )[ names::relative_amplitude ] = DoubleDatum( input_param[ 4 ] );
    ptmp.set( d, this );
  }

  // if we get here, temporary contains consistent set of properties
  P_ = ptmp;
}
