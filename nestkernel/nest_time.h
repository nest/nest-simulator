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
#include <string>
#include <iostream>
#include <cassert>
#include <limits>
#include <cmath>
#include "nest.h"

class Token;

namespace nest 
{


  /**
     Class to handle simulation time and realtime.
     Main idea:
     
     All times given in multiples of "tics":
     A "tic" is a microsecond by default, but may be changed through
     the option --with-tics_per_ms to configure.

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
       compile time and by the TimeModifier class.
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
     Is this entire setup still compatible with different step
     lengths in different subnets, or have we abandoned that idea?
     
     @NOTE 
     Delays must be added to current time, and moduloed each time a
     spike is inserted into a ring buffer.  That operation must be
     very fast, and there is no time for conversions.  Thus, delays
     must be stored in steps.  Given the large number of delays in a
     system, we cannot use class Time with its three member variables
     to store delays.  Delays must thus be stored explicitly as long_t
     steps.  Thus, every time the resolution changes, one needs to
     recalculate all delays, and resize all ring buffers. On each Time object,
     Time::calibrate() must be called.

     Markus Diesmann,       2008-01-25     
     Hans Ekkehard Plesser, 2004-01-25, 2006-12-18
     Marc-Oliver Gewaltig,  2004-01-27

  */

  class TimeModifier;

  class Time 
  {
   friend class TimeModifier;

  public:
    
    /**
     * Helper class to construct Time objects.
     */
    class step
    {
      friend class nest::Time;
    public:
      step(long_t s=0)
	  : steps_(s){}

    private:
      long_t steps_;
    };

    /**
     * Helper class to construct Time objects.
     */
    class tic
    {
      friend class nest::Time;
    public:

      tic(tic_t t= 0)
	  : tics_(t){}

    private:
      tic_t tics_;
    };

    /**
     * Helper class to construct Time objects.
     */
    class ms
    {
      friend class nest::Time;
    public:
      ms(double_t t=0.0)
	  : ms_(t){}
      ms(long_t t=0)
        : ms_(static_cast<double_t>(t)){}
      
      ms(Token const&);

    private:
      double_t ms_;
    };

 
    /**
     * Helper class to construct Time objects.
     * Time objects constructed from Time::ms_stamp objects will always
     * be rounded up to the earliest time step that is no earlier than t.
     * Thus, the Time object can be used directly as step part of a 
     * combined (step, offset) representation of a time. Since 0 < offset <= h
     * by definition, and T = step * h - offset, we must round up.
     */
    class ms_stamp
    {
      friend class nest::Time;
    public:
      ms_stamp(double_t t=0.0)
      	: ms_(t){}
      ms_stamp(long_t t=0)
        : ms_(static_cast<double_t>(t)){}
      
    private:
      double_t ms_;
    };

  // constructors, assignment
  private:
    
    explicit                 
    Time(const tic_t tics);  

  public:

    Time(); 
    Time(const Time& t);

    /**
     * Construct from figure in units of steps.
     * This constructor can, e.g. be used to convert a delay,
     * which is given in simulation steps to time units.
     */ 
    Time(Time::step);

    /**
     * Construct from figure in units of tics.
     * This constructor is identical to the intergral type constructors,
     * however, it makes explicit in the code which units are used.
     */ 
    Time(Time::tic);

    /**
     * Construct from figure in units of ms.
     *
     * The time given as double will be rounded to the nearest tic.
     *
     * @see Time(Time::ms_stamp)
     */ 
    Time(Time::ms);

    /**
     * Construct from figure in units of ms rounding up.
     *
     * Time objects constructed from Time::ms_stamp objects are 
     * set to the earliest time step no earlier than the time given
     * in ms. The Time object obtained in such a way can in particular
     * be used as the time stamp of a spike in stamp-offset representation:
     *
     *  stamp  = Time(Time::ms_stamp(spike_time));
     *  offset = spike_time - stamp.get_ms();
     *
     * @see Time(Time::ms)
     */
    Time(Time::ms_stamp);
    
    /**
     * Set resolution, ie, number of tics per step.
     * @note After a change of resolution, all existing Time objects must
     *       be made aware of the new resolution by calling calibrate(). It
     *       is the responsibility of the Time classe user to ensure this.
     * @param double_t  desired resolution in milliseconds
     */
    static void set_resolution(double_t);

    /**
     * Reset resolution to the compiled-in default.
     * @note After a change of resolution, all existing Time objects must
     *       be made aware of the new resolution by calling calibrate(). It
     *       is the responsibility of the Time classe user to ensure this.
     */
    static void reset_resolution();

    /**
     * Return time resolution in tics per step.
     */
    static 
    Time get_resolution(); 

    /**
     * Returns true if resolution is default resolution.
     */
    static
    bool resolution_is_default();

    static nest::double_t get_ms_per_tic();

    /**
     * Return number of tics per ms.
     * @note The value is returned as double, even though it logically is
     * of type tic_t. This is done to avoid the need for tic_t support in
     * SLI. Even though 64-bit long long has a value range up to 10^18, 
     * while double can only represent 16 digits, there is no conceivable
     * practical problem.
     */
    static nest::double_t get_tics_per_ms();
    static nest::tic_t    get_tics_per_step();
    static nest::tic_t    get_old_tics_per_step();
    static nest::tic_t    get_tics_per_step_default();
    
    /** 
     * Set time object to time zero.
     */
    void set_to_zero();

    /**
     * Recalculate steps from tics after a change in resolution.
     */
    void calibrate();

    /**
     * Add a time to the object.
     */
    Time operator+=(const Time& t);
    Time operator-=(const Time& t);
    Time operator*=(long_t t);

    Time const& operator=(Time const &);
    Time const& operator=(Time::ms const &);
    Time const& operator=(Time::step const &);
    Time const& operator=(Time::tic const &);

    /**
     * Return time in tics.
     */
    tic_t get_tics() const;

    /**
     * Return current time in milliseconds.
     */
    double_t get_ms() const;

    /**
     * Return string with current time in ms.
     */
    std::string get_timestring() const;

    /**
     * Return current time in steps (simulator cycles).
     */
    long_t get_steps() const;

    /** 
     * Increase value of Time object by one step.
     */
    void advance();

    /**
     * Return Time object with next higher step number.
     */
    Time succ() const;

    /**
     * Return Time object with next lower step number.
     */
    Time pred() const;


    /** 
     * Returns true if the Time is a multiple of the simulation step size.
     */
    inline
    bool is_grid_time() const
    {
      return t_tics_ % TICS_PER_STEP_ == 0;
    }

    /**
     * Return smallest possible Time object.
     */
    static Time min();

    /**
     * Return largest possible Time object.
     */ 
    static Time max();

    /**
     * Return Time object representing positive infinity.
     */
    static Time pos_inf();

    /**
     * Return Time object representing negative infinity.
     */ 
    static Time neg_inf();

    /**
     * True if time object is negative infinite.
     * Any time object t < Time::min() is considered negative infinte.
     */
    bool is_neg_inf() const;

    /**
     * True if time object is positive infinite.
     * Any time object t > Time::max() is considered positive infinte.
     */
    bool is_pos_inf() const;

    /**
     * True if time object is neither negative nor positive infinite.
     */
    bool is_finite() const;

    /**
     * True if time object is positive multiple of step size.
     */
    bool is_step() const;
    
    /**
     * True if object is multiple of argument, in tics.
     */
    bool is_multiple_of(const Time&) const;

private:

    friend Time operator-(Time const &t1);

    /** 
     * Basic time unit.
     * This number should be large enough so that the resolution of the 
     * simulation can be expressed in an integral number of tics.
     * The resolution is defined by Time::TICS_PER_STEP_.  
     */
 
    static double_t TICS_PER_MS_; 

    /** 
     * Inverse of basic time unit.
     * USed for fast conversion by multiplication.
     */
    static double_t MS_PER_TIC_; 

   
    /**
     * Resolution of the clock.
     * The value of Time::TICS_PER_STEP_ defines the resolution of the time
     * grid at which the simulation is run.
     * The unit of this variable is tics, thus, in order to obtain the
     * resolution in realtime, one has to divide this number by TICS_PER_MS_.
     */

    static tic_t TICS_PER_STEP_;

    /**
     * Previous resolution of the clock for conversion purposes.
     * @todo OLD_TICS_PER_STEP should be abolished asap, since this way
     *       of attempting to keep track of the resolution-change history
     *       is inherently unsafe. HEP 2008-02-07
     */
    static tic_t OLD_TICS_PER_STEP_;

    

    /**
     * Default value for TICS_PER_STEP_.
     * Contains the hard coded value or the one given by configure.
     */
    static const tic_t TICS_PER_STEP_DEFAULT_;

    /**
     * Default value for TICS_PER_STEP_.
     * Contains the hard coded value or the one given by configure.
     */
    static const double_t TICS_PER_MS_DEFAULT_;


    /**
     * Maximum values for time are limited to a factor of INF_MARGIN_
     * below the maximum values for the underlying datatype.
     */
    static const long_t INF_MARGIN_;

    /**
     * Largest non-infinite time object.
     */
    static Time T_MAX_;

    /**
     * Smallest non-infinite time object.
     */
    static Time T_MIN_;

    /**
     * Representation of positive inifinity.
     */
    static Time T_POS_INF_;

    /**
     * Representation of negative inifinity.
     */
    static Time T_NEG_INF_;

    /**
     * Compute maximum positive time value.
     * Maximum negative value and infinity values are based on this value.
     * @note This function is used to initialized static variables. The value
     * must not be recomputed at run time, since this would kill efficiency. 
     */
    static Time compute_t_max_();

    /**
     * Create positive infinity time object.
     */
    static Time create_pos_inf_();

    /**
     * Create negative infinity time object.
     */
    static Time create_neg_inf_();

    /**
     * Update ms value from tic value.
     */
    void update_ms_();

    /**
     * Update steps value from tic value.
     */
    void update_steps_();

    /**
     * Internal conversion from realtime (ms) to tics.
     *
     * Values are silently truncated to the nearest tic, such that any time
     * within [n*tic-tic/2, n*tic+tic/2) is rounded to n*tic, where tic is
     * the tic duration in ms.
     * 
     * @see ms2tics_floor_
     */
    static tic_t ms2tics_(double_t); 

    /**
     * Internal conversion from realtime (ms) to tics rounding down.
     *
     * Values are always truncated to the largest tic not larger than the
     * argument (floor).
     *
     * @see ms2tics_
     */
    static tic_t ms2tics_floor_(double_t); 

    /**
     * Absolute value.
     * Some older compilers do not overload std::abs() for all C++ types,
     * so we define our own.
     */
    template <typename Numeric>
    static Numeric abs_(Numeric);

    tic_t   t_tics_;      //<! Value in tics.
    double  t_ms_;        //<! Value in ms.
    long_t  t_steps_;     //<! Value in number of simulation steps.
  };


  inline 
  void Time::update_ms_()
  { 
    // Floating point multiplication is more efficient than division,
    // thus, we store the conversion factor such that we can multiply.
    t_ms_ = MS_PER_TIC_ * static_cast<double>(t_tics_); 
  }
  
  inline 
  void Time::update_steps_()
  {
    t_steps_ = t_tics_ / TICS_PER_STEP_;

    // check for truncation. This happens if the new resolution is
    // coarser that the old resolution or if the time object is not
    // representable in steps. The following lines round up to the
    // next compatible step.
    if ( t_tics_ % TICS_PER_STEP_ != 0 )
      t_steps_ += 1;
  }
  
  inline 
  tic_t Time::ms2tics_(double_t ms)
  { 
    // watch out for overflow from very large doubles
    if ( ms < T_MIN_.t_ms_ )
      return T_NEG_INF_.t_tics_;
    else if ( ms > T_MAX_.t_ms_ )
      return T_POS_INF_.t_tics_;
    else
      return static_cast<tic_t>(std::floor(ms*TICS_PER_MS_+0.5)); 
  }

  inline 
  tic_t Time::ms2tics_floor_(double_t ms)
  { 
    // watch out for overflow from very large doubles
    if ( ms < T_MIN_.t_ms_ )
      return T_NEG_INF_.t_tics_;
    else if ( ms > T_MAX_.t_ms_ )
      return T_POS_INF_.t_tics_;
    else
      return static_cast<tic_t>(std::floor(ms*TICS_PER_MS_)); 
  }

  
  inline 
  void Time::advance()
  {
    if ( t_tics_ < T_MAX_.t_tics_ )
    {
      ++t_steps_;
      t_tics_ += TICS_PER_STEP_;
      update_ms_();
    }
    else
      *this = T_POS_INF_;
  }

  inline 
  Time Time::succ() const
  {
    Time s; 

    if ( t_tics_ < T_MAX_.t_tics_ )
    {
      s.t_steps_  = t_steps_ + 1;
      s.t_tics_   = t_tics_ + TICS_PER_STEP_;
      s.update_ms_();
    }
    else
      s = T_POS_INF_;

    return s;
  }
 
  inline 
  Time Time::pred() const
  {
    Time p; 

    if ( t_tics_ > T_MIN_.t_tics_ )
    {
      p.t_steps_  = t_steps_ - 1;
      p.t_tics_   = t_tics_ - TICS_PER_STEP_;
      p.update_ms_();
    }
    else
      p = T_NEG_INF_;

    return p;
  }

  inline
  tic_t Time::get_tics() const
  {
    return t_tics_;
  }

  inline
  double_t Time::get_ms() const
  {
    if ( is_pos_inf() )
      return std::numeric_limits<double_t>::infinity();
    else if ( is_neg_inf() )
      return -std::numeric_limits<double_t>::infinity();
    else
      return t_ms_;
  }

  inline 
  long_t Time::get_steps() const
  {
    return t_steps_;
  }

  inline
  Time Time::get_resolution()
  {
    return Time(TICS_PER_STEP_);
  }

  inline
  bool Time::resolution_is_default()
  {
    return TICS_PER_STEP_ == TICS_PER_STEP_DEFAULT_;
  }

  inline
  Time Time::min()
  {
    return T_MIN_;
  }

  inline
  Time Time::max()
  {
    return T_MAX_;
  }

  inline
  Time Time::neg_inf()
  {
    return T_NEG_INF_;
  }

  inline
  Time Time::pos_inf()
  {
    return T_POS_INF_;
  }

  inline
  Time const& Time::operator=(const Time& t1)
  {
    t_tics_  = t1.t_tics_;
    t_steps_ = t1.t_steps_;
    t_ms_    = t1.t_ms_;
    return *this;
  }

  inline
  Time const& Time::operator=(const Time::step& s)
  {
    *this=Time(s);

    return *this;
  }

  inline
  Time const& Time::operator=(const Time::tic& s)
  {
    *this=Time(s);

    return *this;
  }

  inline
  Time const& Time::operator=(const Time::ms& s)
  {
    *this=Time(s);

    return *this;
  }

  inline
  bool Time::is_neg_inf() const
  {
    return t_tics_ < T_MIN_.t_tics_;
  }

  inline
  bool Time::is_pos_inf() const
  {
    return t_tics_ > T_MAX_.t_tics_;
  }

  inline
  bool Time::is_finite() const
  {
    return !( is_neg_inf() || is_pos_inf() );
  }
  
  inline 
  bool Time::is_step() const
  {
    return t_steps_ > 0 && t_steps_ * TICS_PER_STEP_ == t_tics_;
  }

  inline 
  bool Time::is_multiple_of(const Time& divisor) const
  {
    assert(divisor.t_tics_ > 0);
    return t_tics_ % divisor.t_tics_ == 0;
  }

  inline
  void Time::set_to_zero()
  {
    t_tics_  = 0;
    t_steps_ = 0;
    t_ms_    = 0.0;
  }

  inline
  void Time::calibrate()
  {
    /* We need to consider three cases:
       1. The time is within limits for both old and new resolution.
          In this case, only steps need to be recomputed.
       2. The time is outside the limits for the new resolution. 
          In this case, the time is set to the proper infinity.
       3. The time is within the limits of the new resolution, but
          was outside the limits of the old resolution. In this case,
          the actual time value is meaningless (old infinity representation).
          The time is thus set to the infinity with the correct sign.
     */
    if ( is_pos_inf() )
      *this = T_POS_INF_;
    else if ( is_neg_inf() )
      *this = T_NEG_INF_;
    else 
      update_steps_();
 
    return;
 }

  template <typename Numeric>
  inline
  Numeric Time::abs_(Numeric v)
  {
    return v >= 0 ? v : -v;
  }

  /**
   * Returns true if both objects have the same number of tics.
   */
  inline
  bool operator==(const Time& t1, const Time& t2)
  {
    return t1.get_tics() == t2.get_tics();
  }

  /**
   * Returns true if both objects have a different number of tics.
   */
  inline
  bool operator!=(const Time& t1, const Time& t2)
  {
    return ! ( t1 == t2 );
  }

  /**
   * Returns true if t1.tics() < t2.tics().
   */
  inline
  bool operator<(const Time& t1, const Time& t2)
  {
    return t1.get_tics() < t2.get_tics();
  }

  /**
   * Returns true if t1.tics() > t2.tics().
   */
  inline
  bool operator>(const Time& t1, const Time& t2)
  {
    return t1.get_tics() > t2.get_tics();
  }

  /**
   * Returns true if t1.tics() <= t2.tics().
   */
  inline
  bool operator<=(const Time& t1, const Time& t2)
  {
    return !(t1 > t2);
  }

  /**
   * Returns true if t1.tics() >= t2.tics().
   */
  inline
  bool operator>=(const Time& t1, const Time& t2)
  {
    return !(t1 < t2);
  }

  Time operator-(const Time& t1);
  Time operator+(const Time& t1, const Time& t2);
  Time operator-(const Time& t1, const Time& t2);
  Time operator*(const long_t factor, const Time& t);
  Time operator*(const Time& t, long_t factor);

  std::ostream& operator<<(std::ostream& strm, const Time& t);
  
} // Namespace

#endif
