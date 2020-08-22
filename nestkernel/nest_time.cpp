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

/* Obtain time resolution information from configuration
   variables or use defaults.
*/

#ifndef CONFIG_TICS_PER_MS
#define CONFIG_TICS_PER_MS 1000.0
#endif

#ifndef CONFIG_TICS_PER_STEP
#define CONFIG_TICS_PER_STEP 100
#endif

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
const delay Time::LimitPosInf::steps;
const tic_t Time::LimitNegInf::tics;
const delay Time::LimitNegInf::steps;

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
  // results in corner cases --- and I don't think the
  // intended ones.
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

std::ostream& operator<<( std::ostream& strm, const Time& t )
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
