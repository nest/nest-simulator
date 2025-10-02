/*
 *  nest_time.cpp
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

#include "nest_time.h"

// C++ includes:
#include <string>

// Generated includes:
#include "config.h"

// Includes from libnestutil:
#include "numerics.h"

// Includes from sli:
#include "doubledatum.h"
#include "integerdatum.h"
#include "token.h"

using namespace nest;

const double Time::Range::TICS_PER_MS_DEFAULT = CONFIG_TICS_PER_MS;
const tic_t Time::Range::TICS_PER_STEP_DEFAULT = CONFIG_TICS_PER_STEP;

tic_t Time::Range::TICS_PER_STEP = Time::Range::TICS_PER_STEP_DEFAULT;
double Time::Range::TICS_PER_STEP_INV = 1. / static_cast< double >( Time::Range::TICS_PER_STEP );
tic_t Time::Range::TICS_PER_STEP_RND = Time::Range::TICS_PER_STEP - 1;

double Time::Range::TICS_PER_MS = Time::Range::TICS_PER_MS_DEFAULT;
double Time::Range::MS_PER_TIC = 1 / Time::Range::TICS_PER_MS;

double Time::Range::MS_PER_STEP = TICS_PER_STEP / TICS_PER_MS;
double Time::Range::STEPS_PER_MS = 1 / Time::Range::MS_PER_STEP;

// define for unit -- const'ness is in the header
// should only be necessary when not folded away
// by the compiler as compile time consts
const tic_t Time::LimitPosInf::tics;
const long Time::LimitPosInf::steps;
const tic_t Time::LimitNegInf::tics;
const long Time::LimitNegInf::steps;

tic_t
Time::compute_max()
{
  const long lmax = std::numeric_limits< long >::max();
  const tic_t tmax = std::numeric_limits< tic_t >::max();

  tic_t tics;
  if ( lmax < tmax * Range::TICS_PER_STEP_INV ) // step size is limiting factor
  {
    tics = Range::TICS_PER_STEP * ( lmax / Range::INF_MARGIN );
  }
  else // tic size is limiting factor
  {
    tics = tmax / Range::INF_MARGIN;
  } // make sure that tics and steps match so that we can have simple range
  // checking when going back and forth, regardless of limiting factor
  return tics - ( tics % Range::TICS_PER_STEP );
}

Time::Limit::Limit( const tic_t& t )
  : tics( t )
  , steps( t * Range::TICS_PER_STEP_INV )
  , ms( steps * Range::MS_PER_STEP )
{
}

Time::Limit Time::LIM_MAX( +Time::compute_max() );
Time::Limit Time::LIM_MIN( -Time::compute_max() );

void
Time::set_resolution( double ms_per_step )
{
  assert( ms_per_step > 0 );

  Range::TICS_PER_STEP = static_cast< tic_t >( dround( Range::TICS_PER_MS * ms_per_step ) );
  Range::TICS_PER_STEP_INV = 1. / static_cast< double >( Range::TICS_PER_STEP );
  Range::TICS_PER_STEP_RND = Range::TICS_PER_STEP - 1;

  // Recalculate ms_per_step to be consistent with rounding above
  Range::MS_PER_STEP = Range::TICS_PER_STEP / Range::TICS_PER_MS;
  Range::STEPS_PER_MS = 1 / Range::MS_PER_STEP;

  const tic_t max = compute_max();
  LIM_MAX = +max;
  LIM_MIN = -max;
}

void
Time::set_resolution( double tics_per_ms, double ms_per_step )
{
  Range::TICS_PER_MS = tics_per_ms;
  Range::MS_PER_TIC = 1 / tics_per_ms;
  set_resolution( ms_per_step );
}

void
Time::reset_resolution()
{
  Range::TICS_PER_STEP = Range::TICS_PER_STEP_DEFAULT;
  Range::TICS_PER_STEP_INV = 1. / static_cast< double >( Range::TICS_PER_STEP );
  Range::TICS_PER_STEP_RND = Range::TICS_PER_STEP - 1;

  const tic_t max = compute_max();
  LIM_MAX = +max;
  LIM_MIN = -max;
}

double
Time::ms::fromtoken( const Token& t )
{
  IntegerDatum* idat = dynamic_cast< IntegerDatum* >( t.datum() );
  if ( idat )
  {
    return static_cast< double >( idat->get() );
  }

  DoubleDatum* ddat = dynamic_cast< DoubleDatum* >( t.datum() );
  if ( ddat )
  {
    return ddat->get();
  }

  throw TypeMismatch( IntegerDatum().gettypename().toString() + " or " + DoubleDatum().gettypename().toString(),
    t.datum()->gettypename().toString() );
}

tic_t
Time::fromstamp( Time::ms_stamp t )
{
  if ( t.t > LIM_MAX.ms )
  {
    return LIM_POS_INF.tics;
  }
  else if ( t.t < LIM_MIN.ms )
  {
    return LIM_NEG_INF.tics;
  }

  // why not just fmod STEPS_PER_MS? This gives different
  // results in corner cases --- and I don't think the intended ones.
  tic_t n = static_cast< tic_t >( t.t * Range::TICS_PER_MS );
  n -= ( n % Range::TICS_PER_STEP );
  const double ms = n * Range::TICS_PER_STEP_INV * Range::MS_PER_STEP;
  if ( ms < t.t )
  {
    n += Range::TICS_PER_STEP;
  }
  return n;
}

void
Time::reset_to_defaults()
{
  // reset the TICS_PER_MS to compiled in default values
  Range::TICS_PER_MS = Range::TICS_PER_MS_DEFAULT;
  Range::MS_PER_TIC = 1 / Range::TICS_PER_MS_DEFAULT;

  // reset TICS_PER_STEP to compiled in default values
  Range::TICS_PER_STEP = Range::TICS_PER_STEP_DEFAULT;
  Range::TICS_PER_STEP_INV = 1. / static_cast< double >( Range::TICS_PER_STEP );
  Range::TICS_PER_STEP_RND = Range::TICS_PER_STEP - 1;

  Range::MS_PER_STEP = Range::TICS_PER_STEP / Range::TICS_PER_MS;
  Range::STEPS_PER_MS = 1 / Range::MS_PER_STEP;
}

/////////////////////////////////////////////////////////////
// Resolution: set tics per ms, steps per ms
/////////////////////////////////////////////////////////////

Time
Time::get_resolution()
{
  return Time( Range::TICS_PER_STEP );
}

bool
Time::resolution_is_default()
{
  return Range::TICS_PER_STEP == Range::TICS_PER_STEP_DEFAULT;
}

/////////////////////////////////////////////////////////////
// Common zero-ary or unary operations
/////////////////////////////////////////////////////////////

void
Time::set_to_zero()
{
  tics = 0;
}

void
Time::advance()
{
  tics += Range::TICS_PER_STEP;
  range();
}

Time
Time::succ() const
{
  return tic( tics + Range::TICS_PER_STEP );
} // check range

Time
Time::pred() const
{
  return tic( tics - Range::TICS_PER_STEP );
} // check range

/////////////////////////////////////////////////////////////
// Subtypes of Time (bool tests)
/////////////////////////////////////////////////////////////


bool
Time::is_finite() const
{
  return tics != LIM_POS_INF.tics and tics != LIM_NEG_INF.tics;
}

bool
Time::is_neg_inf() const
{
  // Currently tics can never become smaller than LIM_NEG_INF.tics. However, if
  // LIM_NEG_INF.tics represent negative infinity, any smaller
  // value cannot be larger and thus must be infinity as well. to be on the safe side
  // we use less-or-equal instead of just equal.
  return tics <= LIM_NEG_INF.tics;
}

bool
Time::is_pos_inf() const
{
  return tics >= LIM_POS_INF.tics; // see comment for is_neg_inf()
}

bool
Time::is_grid_time() const
{
  return ( tics % Range::TICS_PER_STEP ) == 0;
}

bool
Time::is_step() const
{
  return tics > 0 and is_grid_time();
}

bool
Time::is_multiple_of( const Time& divisor ) const
{
  assert( divisor.tics > 0 );
  return ( tics % divisor.tics ) == 0;
}
/////////////////////////////////////////////////////////////
// Singleton'ish types
/////////////////////////////////////////////////////////////


Time
Time::max()
{
  return Time( LIM_MAX.tics );
}
Time
Time::min()
{
  return Time( LIM_MIN.tics );
}
double
Time::get_ms_per_tic()
{
  return Range::MS_PER_TIC;
}
Time
Time::neg_inf()
{
  return Time( LIM_NEG_INF.tics );
}
Time
Time::pos_inf()
{
  return Time( LIM_POS_INF.tics );
}

/////////////////////////////////////////////////////////////
// Overflow checks & recalibrate after resolution setting
/////////////////////////////////////////////////////////////


void
Time::range()
{
  if ( time_abs( tics ) < LIM_MAX.tics )
  {
    return;
  }
  tics = ( tics < 0 ) ? LIM_NEG_INF.tics : LIM_POS_INF.tics;
}

void
Time::calibrate()
{
  range();
}

/////////////////////////////////////////////////////////////
// Unary operators
/////////////////////////////////////////////////////////////
Time&
Time::operator+=( const Time& t )
{
  tics += t.tics;
  range();
  return *this;
}

/////////////////////////////////////////////////////////////
// Convert to external units
/////////////////////////////////////////////////////////////


tic_t
Time::get_tics() const
{
  return tics;
}
tic_t
Time::get_tics_per_step()
{
  return Range::TICS_PER_STEP;
}
double
Time::get_tics_per_ms()
{
  return Range::TICS_PER_MS;
}

double
Time::get_ms() const
{
  if ( is_pos_inf() )
  {
    return LIM_POS_INF_ms;
  }
  if ( is_neg_inf() )
  {
    return LIM_NEG_INF_ms;
  }
  return Range::MS_PER_TIC * tics;
}

long
Time::get_steps() const
{
  if ( is_pos_inf() )
  {
    return LIM_POS_INF.steps;
  }
  if ( is_neg_inf() )
  {
    return LIM_NEG_INF.steps;
  }

  // round tics up to nearest step
  // by adding TICS_PER_STEP-1 before division
  return ( tics + Range::TICS_PER_STEP_RND ) * Range::TICS_PER_STEP_INV;
}

/**
 * Convert between delays given in steps and milliseconds.
 *
 * This is not a reversible operation, since steps have a finite
 * rounding resolution. This is not a truncation, but rounding as per
 * ld_round, which is different from ms_stamp --> Time mapping, which rounds
 * up. See #903.
 */
double
Time::delay_steps_to_ms( long steps )
{
  return steps * Range::MS_PER_STEP;
}

long
Time::delay_ms_to_steps( double ms )
{
  return ld_round( ms * Range::STEPS_PER_MS );
}

/////////////////////////////////////////////////////////////
// Binary operators
/////////////////////////////////////////////////////////////

namespace nest
{
bool
operator==( const Time& t1, const Time& t2 )
{
  return t1.tics == t2.tics;
}

bool
operator!=( const Time& t1, const Time& t2 )
{
  return t1.tics != t2.tics;
}

bool
operator<( const Time& t1, const Time& t2 )
{
  return t1.tics < t2.tics;
}

bool
operator>( const Time& t1, const Time& t2 )
{
  return t1.tics > t2.tics;
}

bool
operator<=( const Time& t1, const Time& t2 )
{
  return t1.tics <= t2.tics;
}

bool
operator>=( const Time& t1, const Time& t2 )
{
  return t1.tics >= t2.tics;
}

Time
operator+( const Time& t1, const Time& t2 )
{
  return Time::tic( t1.tics + t2.tics ); // check range
}

Time
operator-( const Time& t1, const Time& t2 )
{
  return Time::tic( t1.tics - t2.tics ); // check range
}

Time
operator*( const long factor, const Time& t )
{
  const tic_t n = factor * t.tics;
  // if no overflow:
  if ( t.tics == 0 or n / t.tics == factor )
  {
    return Time::tic( n ); // check range
  }
  if ( ( t.tics > 0 and factor > 0 ) or ( t.tics < 0 and factor < 0 ) )
  {
    return Time( Time::LIM_POS_INF.tics );
  }
  else
  {
    return Time( Time::LIM_NEG_INF.tics );
  }
}

Time
operator*( const Time& t, long factor )
{
  return factor * t;
}

}
std::ostream&
operator<<( std::ostream& strm, const Time& t )
{
  if ( t.is_neg_inf() )
  {
    strm << "-INF";
  }
  else if ( t.is_pos_inf() )
  {
    strm << "+INF";
  }
  else
  {
    strm << t.get_ms() << " ms (= " << t.get_tics() << " tics = " << t.get_steps()
         << ( t.get_steps() != 1 ? " steps)" : " step)" );
  }

  return strm;
}
