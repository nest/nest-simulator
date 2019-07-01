/*
 *  nest_time.h
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

#ifndef NEST_TIME_H
#define NEST_TIME_H

// C++ includes:
#include <cassert>
#include <cfloat>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>

// Includes from libnestutil:
#include "numerics.h"

// Includes from nestkernel:
#include "nest_types.h"

class Token;

namespace nest
{
class Time;
}

std::ostream& operator<<( std::ostream&, const nest::Time& );

namespace nest
{
/**
   Class to handle simulation time and realtime.
   Main idea:

   All times given in multiples of "tics":
   A "tic" is a microsecond by default, but may be changed through
   the option -Dtics_per_ms to configure.

   User access to time only through accessor functions:
   - Times can be added, subtracted, and multiplied by ints
   - All real world time is given in ms as double
   - All computation is done based on tics

   Three time variables are kept:
   #- time in tics
   #- time in ms
   #- time in steps

   The largest representable time is available through Time::max().

   @NOTE
   - The time base (tics per millisecond) can only be set at
     compile time and by the Time::set_resolution().
   - Times in ms are rounded up to the next tic interval.
     This ensures that the time intervals (0, h] are open at the left
     point and closed at the right point. It also ensures compatibility with
     precise timing, namely that the offset u fulfills -h > u >= 0.
   - The resolution (tics per step) can only be set before the first
     node is created and before the simulation starts. The resolution
     can be changed after the network has been deleted and the time
     reset.
   - Implementers of models or methods containing persistent (member variable)
     Time objects, must ensure that these are recalibrated before the
     simulation starts. This is necessary to ensure that step values
     are updated after a change in resolution.
   - The default resolution can be changed using the --with-tics_per_step
     option to configure.


   @NOTE
   The step-time counter is NOT changed when the resolution is
   changed.  This is of no consequence, since changes in resolution
   are permitted at t=0 only.

   @NOTE
   - Neurons update themselves in min-delay intervals. During such a min-delay
     update step, time is in a sense undefined, since it is up to the model how
     it takes its dynamics across the interval. Any spikes emitted and voltage
     information returned must be fixed to time grid points.
   - One may later consider to introduce per-tread simulation time variables.

   @NOTE
   Delays must be added to current time, and moduloed each time a
   spike is inserted into a ring buffer.  That operation must be
   very fast, and there is no time for conversions.  Thus, delays
   must be stored in steps.  Given the large number of delays in a
   system, we cannot use class Time with its three member variables
   to store delays.  Delays must thus be stored explicitly as delay
   steps.

   Markus Diesmann,       2008-01-25
   Hans Ekkehard Plesser, 2004-01-25, 2006-12-18
   Marc-Oliver Gewaltig,  2004-01-27

*/

/////////////////////////////////////////////////////////////
// Function to use internally
/////////////////////////////////////////////////////////////

// This is just to map llabs => abs
template < class N >
inline N
time_abs( const N n )
{
  return std::abs( n );
}

template <>
inline long long
time_abs( long long n )
{
  return llabs( n );
}

/////////////////////////////////////////////////////////////
// Time class = tic_t
/////////////////////////////////////////////////////////////

class Time
{
  // tic_t: tics in  a step, signed long or long long
  // delay: steps, signed long
  // double: milliseconds (double!)

  friend class TimeConverter;

  /////////////////////////////////////////////////////////////
  // Range: Limits & conversion factors for different types
  /////////////////////////////////////////////////////////////

protected:
  struct Range
  {
    static tic_t TICS_PER_STEP;
    static double TICS_PER_STEP_INV;
    static tic_t TICS_PER_STEP_RND;

    static double TICS_PER_MS;
    static double MS_PER_TIC;
    static double STEPS_PER_MS;
    static double MS_PER_STEP;

    static const tic_t TICS_PER_STEP_DEFAULT;
    static const double TICS_PER_MS_DEFAULT;

    static const long INF_MARGIN = 8;
  };

public:
  static tic_t compute_max();

  /////////////////////////////////////////////////////////////
  // The data: longest integer for tics
  /////////////////////////////////////////////////////////////

protected:
  tic_t tics;

  /////////////////////////////////////////////////////////////
  // Friend declaration for units and binary operators
  /////////////////////////////////////////////////////////////

  friend struct step;
  friend struct tic;
  friend struct ms;
  friend struct ms_stamp;

  friend bool operator==( const Time& t1, const Time& t2 );
  friend bool operator!=( const Time& t1, const Time& t2 );
  friend bool operator<( const Time& t1, const Time& t2 );
  friend bool operator>( const Time& t1, const Time& t2 );
  friend bool operator<=( const Time& t1, const Time& t2 );
  friend bool operator>=( const Time& t1, const Time& t2 );
  friend Time operator+( const Time& t1, const Time& t2 );
  friend Time operator-( const Time& t1, const Time& t2 );
  friend Time operator*( const long factor, const Time& t );
  friend Time operator*( const Time& t, long factor );
  friend std::ostream&(::operator<<)( std::ostream&, const Time& );

  /////////////////////////////////////////////////////////////
  // Limits for time, including infinity definitions
  /////////////////////////////////////////////////////////////

protected:
  struct Limit
  {
    tic_t tics;
    delay steps;
    double ms;

    Limit( tic_t tics, delay steps, double ms )
      : tics( tics )
      , steps( steps )
      , ms( ms )
    {
    }
    Limit( const tic_t& );
  };
  static Limit LIM_MAX;
  static Limit LIM_MIN;
  Time::Limit limit( const tic_t& );

  // max is never larger than tics/INF_MARGIN, and we can use INF_MARGIN
  // to minimize range checks on +/- operations
  static struct LimitPosInf
  {
    static const tic_t tics = tic_t_max / Range::INF_MARGIN + 1;
    static const delay steps = delay_max;
#define LIM_POS_INF_ms DBL_MAX // because C++ bites
  } LIM_POS_INF;

  static struct LimitNegInf
  {
    static const tic_t tics = -tic_t_max / Range::INF_MARGIN - 1;
    static const delay steps = -delay_max;
#define LIM_NEG_INF_ms ( -DBL_MAX ) // c++ bites
  } LIM_NEG_INF;

  /////////////////////////////////////////////////////////////
  // Unit class for constructors
  /////////////////////////////////////////////////////////////

public:
  struct tic
  {
    tic_t t;
    explicit tic( tic_t t )
      : t( t ){};
  };

  struct step
  {
    delay t;
    explicit step( delay t )
      : t( t )
    {
    }
  };

  struct ms
  {
    double t;
    explicit ms( double t )
      : t( t )
    {
    }

    static double fromtoken( const Token& t );
    explicit ms( const Token& t )
      : t( fromtoken( t ) ){};
  };

  struct ms_stamp
  {
    double t;
    explicit ms_stamp( double t )
      : t( t )
    {
    }
  };

  /////////////////////////////////////////////////////////////
  // Constructors
  /////////////////////////////////////////////////////////////

protected:
  explicit Time( tic_t tics )
    : tics( tics )
  {
  } // This doesn't check ranges.
  // Ergo: LIM_MAX.tics >= tics >= LIM_MIN.tics or
  //       tics == LIM_POS_INF.tics or LIM_NEG_INF.tics

public:
  Time()
    : tics( 0 ){};

  // Default copy constructor: assumes legal time object
  // Defined by compiler.
  // Time(const Time& t);

  Time( tic t )
    : tics( ( time_abs( t.t ) < LIM_MAX.tics ) ? t.t : ( t.t < 0 ) ? LIM_NEG_INF.tics : LIM_POS_INF.tics )
  {
  }

  Time( step t )
    : tics( ( time_abs( t.t ) < LIM_MAX.steps ) ? t.t * Range::TICS_PER_STEP : ( t.t < 0 ) ? LIM_NEG_INF.tics
                                                                                           : LIM_POS_INF.tics )
  {
  }

  Time( ms t )
    : tics( ( time_abs( t.t ) < LIM_MAX.ms ) ? static_cast< tic_t >( t.t * Range::TICS_PER_MS + 0.5 )
                                             : ( t.t < 0 ) ? LIM_NEG_INF.tics : LIM_POS_INF.tics )
  {
  }

  static tic_t fromstamp( ms_stamp );
  Time( ms_stamp t )
    : tics( fromstamp( t ) )
  {
  }

  /////////////////////////////////////////////////////////////
  // Resolution: set tics per ms, steps per ms
  /////////////////////////////////////////////////////////////

  static void set_resolution( double tics_per_ms );
  static void set_resolution( double tics_per_ms, double ms_per_step );
  static void reset_resolution();
  static void reset_to_defaults();

  static Time
  get_resolution()
  {
    return Time( Range::TICS_PER_STEP );
  }

  static bool
  resolution_is_default()
  {
    return Range::TICS_PER_STEP == Range::TICS_PER_STEP_DEFAULT;
  }

  /////////////////////////////////////////////////////////////
  // Common zero-ary or unary operations
  /////////////////////////////////////////////////////////////

  void
  set_to_zero()
  {
    tics = 0;
  }

  void
  advance()
  {
    tics += Range::TICS_PER_STEP;
    range();
  }

  Time
  succ() const
  {
    return tic( tics + Range::TICS_PER_STEP );
  } // check range
  Time
  pred() const
  {
    return tic( tics - Range::TICS_PER_STEP );
  } // check range

  /////////////////////////////////////////////////////////////
  // Subtypes of Time (bool tests)
  /////////////////////////////////////////////////////////////

  bool
  is_finite() const
  {
    return tics != LIM_POS_INF.tics and tics != LIM_NEG_INF.tics;
  }

  bool
  is_neg_inf() const
  {
    return tics <= LIM_NEG_INF.tics; // currently tics can never
                                     // become smaller than
                                     // LIM_NEG_INF.tics. however if
                                     // LIM_NEG_INF.tics represent
                                     // negative infinity, any smaller
                                     // value cannot be larger and
                                     // thus must be infinity as
                                     // well. to be on the safe side
                                     // we use less-or-equal instead
                                     // of just equal.
  }

  bool
  is_pos_inf() const
  {
    return tics >= LIM_POS_INF.tics; // see comment for is_neg_inf()
  }

  bool
  is_grid_time() const
  {
    return ( tics % Range::TICS_PER_STEP ) == 0;
  }
  bool
  is_step() const
  {
    return tics > 0 and is_grid_time();
  }

  bool
  is_multiple_of( const Time& divisor ) const
  {
    assert( divisor.tics > 0 );
    return ( tics % divisor.tics ) == 0;
  }

  /////////////////////////////////////////////////////////////
  // Singleton'ish types
  /////////////////////////////////////////////////////////////

  static Time
  max()
  {
    return Time( LIM_MAX.tics );
  }
  static Time
  min()
  {
    return Time( LIM_MIN.tics );
  }
  static double
  get_ms_per_tic()
  {
    return Range::MS_PER_TIC;
  }
  static Time
  neg_inf()
  {
    return Time( LIM_NEG_INF.tics );
  }
  static Time
  pos_inf()
  {
    return Time( LIM_POS_INF.tics );
  }

  /////////////////////////////////////////////////////////////
  // Overflow checks & recalibrate after resolution setting
  /////////////////////////////////////////////////////////////

  void
  range()
  {
    if ( time_abs( tics ) < LIM_MAX.tics )
    {
      return;
    }
    tics = ( tics < 0 ) ? LIM_NEG_INF.tics : LIM_POS_INF.tics;
  }

  void
  calibrate()
  {
    range();
  }

  /////////////////////////////////////////////////////////////
  // Unary operators
  /////////////////////////////////////////////////////////////

  Time& operator+=( const Time& t )
  {
    tics += t.tics;
    range();
    return *this;
  }

  /////////////////////////////////////////////////////////////
  // Convert to external units
  /////////////////////////////////////////////////////////////

  tic_t
  get_tics() const
  {
    return tics;
  }
  static tic_t
  get_tics_per_step()
  {
    return Range::TICS_PER_STEP;
  }
  static double
  get_tics_per_ms()
  {
    return Range::TICS_PER_MS;
  }

  double
  get_ms() const
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

  delay
  get_steps() const
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
   * This is not a reversible operation, since steps have a finite
   * rounding resolution. This is not a truncation, but rounding as per
   * ld_round, which is different from ms_stamp --> Time mapping, which rounds
   * up. See #903.
   */
  static double
  delay_steps_to_ms( delay steps )
  {
    return steps * Range::MS_PER_STEP;
  }

  static delay
  delay_ms_to_steps( double ms )
  {
    return ld_round( ms * Range::STEPS_PER_MS );
  }
};

/////////////////////////////////////////////////////////////
// Non-class definitions
/////////////////////////////////////////////////////////////

// Needs to be outside the class to get internal linkage to
// maybe make the zero visible for optimization.
const Time TimeZero;

/////////////////////////////////////////////////////////////
// Binary operators
/////////////////////////////////////////////////////////////

inline bool operator==( const Time& t1, const Time& t2 )
{
  return t1.tics == t2.tics;
}

inline bool operator!=( const Time& t1, const Time& t2 )
{
  return t1.tics != t2.tics;
}

inline bool operator<( const Time& t1, const Time& t2 )
{
  return t1.tics < t2.tics;
}

inline bool operator>( const Time& t1, const Time& t2 )
{
  return t1.tics > t2.tics;
}

inline bool operator<=( const Time& t1, const Time& t2 )
{
  return t1.tics <= t2.tics;
}

inline bool operator>=( const Time& t1, const Time& t2 )
{
  return t1.tics >= t2.tics;
}

inline Time operator+( const Time& t1, const Time& t2 )
{
  return Time::tic( t1.tics + t2.tics ); // check range
}

inline Time operator-( const Time& t1, const Time& t2 )
{
  return Time::tic( t1.tics - t2.tics ); // check range
}

inline Time operator*( const long factor, const Time& t )
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

inline Time operator*( const Time& t, long factor )
{
  return factor * t;
}
} // namespace

std::ostream& operator<<( std::ostream&, const nest::Time& );


#endif
