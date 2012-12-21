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

#include <string>
#include "nest_time.h"
#include "token.h"
#include "integerdatum.h"
#include "doubledatum.h"
#include "config.h"
#include "numerics.h"


/* Obtain time resolution information from configuration 
   variables or use defaults.
*/
#ifdef HAVE_TICS_PER_MS
const nest::double_t nest::Time::TICS_PER_MS_DEFAULT_ = CONFIG_TICS_PER_MS;
#else
const nest::double_t nest::Time::TICS_PER_MS_DEFAULT_ = 1000.0;
#endif

#ifdef HAVE_TICS_PER_STEP
const nest::tic_t    nest::Time::TICS_PER_STEP_DEFAULT_ = CONFIG_TICS_PER_STEP;
#else
const nest::tic_t    nest::Time::TICS_PER_STEP_DEFAULT_ = 100;
#endif

nest::double_t nest::Time::TICS_PER_MS_ = TICS_PER_MS_DEFAULT_;

nest::tic_t nest::Time::TICS_PER_STEP_ = TICS_PER_STEP_DEFAULT_;
nest::tic_t nest::Time::OLD_TICS_PER_STEP_ = TICS_PER_STEP_DEFAULT_;

nest::double_t nest::Time::MS_PER_TIC_ = 1.0 / nest::Time::TICS_PER_MS_;


// time limits
const nest::long_t nest::Time::INF_MARGIN_ = 8;
nest::Time nest::Time::T_MAX_     =  nest::Time::compute_t_max_();
nest::Time nest::Time::T_MIN_     = -nest::Time::compute_t_max_();
nest::Time nest::Time::T_POS_INF_ =  nest::Time::create_pos_inf_();
nest::Time nest::Time::T_NEG_INF_ =  nest::Time::create_neg_inf_();


void nest::Time::set_resolution(double_t ms_per_step)
{
  assert(ms_per_step > 0);

  OLD_TICS_PER_STEP_ = TICS_PER_STEP_;
  TICS_PER_STEP_ = static_cast<tic_t>(dround(TICS_PER_MS_ * ms_per_step));
  T_MAX_         =  compute_t_max_();
  T_MIN_         = -compute_t_max_();
  T_POS_INF_     =  create_pos_inf_();
  T_NEG_INF_     =  create_neg_inf_();
}

void nest::Time::reset_resolution()
{
  // When resetting the kernel, we have to reset OLD_TICS as well,
  // otherwise we get into trouble with regenerated synapse prototypes,
  // see ticket #164.
  OLD_TICS_PER_STEP_ = TICS_PER_STEP_DEFAULT_;
  TICS_PER_STEP_ = TICS_PER_STEP_DEFAULT_;
  T_MAX_         =  compute_t_max_();
  T_MIN_         = -compute_t_max_();
  T_POS_INF_     =  create_pos_inf_();
  T_NEG_INF_     =  create_neg_inf_();
}

nest::Time nest::Time::compute_t_max_()
{
  // Data types used for representing time in steps, delays and
  // for MPI communication of spike times are currently (r8662)
  // inconsistent. The most restrictive data type is the representation
  // of delays as uint_t and the MPI communication using uint_t.
  // Since times shall be signed, we need to restrict step representation
  // to int_t. See also #408.

  // basic consistency checks
  assert(std::numeric_limits<int_t>::max() 
	 <= std::numeric_limits<tic_t>::max());   
  assert(std::numeric_limits<int_t>::min() + std::numeric_limits<int_t>::max()
	 <= 0);
  assert(std::numeric_limits<tic_t>::min() + std::numeric_limits<tic_t>::max()
	 <= 0);

  Time tmax;

  // are tics or steps limiting the range?
  if ( std::numeric_limits<int_t>::max() < 
       std::numeric_limits<tic_t>::max() / TICS_PER_STEP_ )
  {
    // number of steps is limiting factor
    tmax.t_steps_  = std::numeric_limits<int_t>::max() / INF_MARGIN_;
    tmax.t_tics_ = TICS_PER_STEP_ * tmax.t_steps_;
  }
  else
  {
    // number of tics is limiting factor
    tmax.t_tics_ = std::numeric_limits<tic_t>::max() / INF_MARGIN_;
    tmax.update_steps_();  
  }

  tmax.update_ms_();
  
  return tmax;
}

nest::Time nest::Time::create_pos_inf_()
{
  Time posinf;

  // ensure we are one step above max
  posinf.t_tics_ = T_MAX_.t_tics_ + TICS_PER_STEP_;
  posinf.t_steps_ = T_MAX_.t_steps_ + 1;
  // we cannot call tics2ms_(), since we violate overflow
  posinf.t_ms_   =  MS_PER_TIC_ * static_cast<double_t>(posinf.t_tics_); 

  return posinf;
}

nest::Time nest::Time::create_neg_inf_()
{
  Time neginf;

  // ensure we are one step below min
  neginf.t_tics_ = T_MIN_.t_tics_ - TICS_PER_STEP_;
  neginf.t_steps_  = T_MIN_.t_steps_ - 1;
  // we cannot call tics2ms_(), since we violate overflow
  neginf.t_ms_   =  MS_PER_TIC_ * static_cast<double_t>(neginf.t_tics_); 

  return neginf;
}

nest::double_t nest::Time::get_ms_per_tic() {
  return 1.0 / TICS_PER_MS_;
}
    
nest::double_t nest::Time::get_tics_per_ms() {
  return TICS_PER_MS_;
}

nest::tic_t nest::Time::get_tics_per_step() {
  return TICS_PER_STEP_;
}

nest::tic_t nest::Time::get_old_tics_per_step() {
  return OLD_TICS_PER_STEP_;
}

nest::tic_t nest::Time::get_tics_per_step_default() {
  return TICS_PER_STEP_DEFAULT_;
}

nest::Time::ms::ms(const Token &t)
{
  IntegerDatum *idat=dynamic_cast<IntegerDatum *>(t.datum());
  if(idat != NULL)
  {
    ms_=static_cast<double_t>(idat->get());
    return;
  }
  
  DoubleDatum *ddat=dynamic_cast<DoubleDatum *>(t.datum());
  if(ddat != NULL)
  {
    ms_=ddat->get();
    return;
  }

  IntegerDatum const d1;
  DoubleDatum const d2;
  throw TypeMismatch(d1.gettypename().toString() + " or " +
		     d2.gettypename().toString(), 
		     t.datum()->gettypename().toString());
}


nest::Time::Time()
    : 
    t_tics_(0),
    t_ms_(0.0),
    t_steps_(0)
{
}

nest::Time::Time(const nest::Time &t)
    : 
    t_tics_(t.t_tics_),
    t_ms_(t.t_ms_),
    t_steps_(t.t_steps_)
{
}

nest::Time::Time(const tic_t t)
    : 
    t_tics_(t),
    t_ms_(0.0),
    t_steps_(0)
{
  if ( t_tics_ > T_MAX_.t_tics_ )
    *this = T_POS_INF_;
  else if ( t_tics_ < T_MIN_.t_tics_ )
    *this = T_NEG_INF_;
  else
  {
      update_ms_();
      update_steps_();
  }
}

nest::Time::Time(Time::tic t)
    : 
    t_tics_(t.tics_),
    t_ms_(0.0),
    t_steps_(0)
{
  if ( t_tics_ > T_MAX_.t_tics_ )
    *this = T_POS_INF_;
  else if ( t_tics_ < T_MIN_.t_tics_ )
    *this = T_NEG_INF_;
  else
  {
    update_ms_();
    update_steps_();
  }
}

nest::Time::Time(Time::step s)
    : 
    t_tics_(0),
    t_ms_(0.0),
    t_steps_(s.steps_)
{
  if ( t_steps_ > T_MAX_.t_steps_ )
    *this = T_POS_INF_;
  else if ( t_steps_ < T_MIN_.t_steps_ )
    *this = T_NEG_INF_;
  else
  {
    t_tics_ = TICS_PER_STEP_ * t_steps_;
    update_ms_();
  }
}

nest::Time::Time(Time::ms t)
    : 
    t_tics_(0),
    t_ms_(0.0),
    t_steps_(0)
{
  if ( t.ms_ > T_MAX_.t_ms_ )
    *this = T_POS_INF_;
  else if ( t.ms_ < T_MIN_.t_ms_ )
    *this = T_NEG_INF_;
  else
  {
    // Do not change the order of the following statements
    t_tics_ = ms2tics_(t.ms_);
    update_steps_(); // This might round up ...
    update_ms_(); // ... so this has to come last
  }
}

nest::Time::Time(Time::ms_stamp t)
    : 
    t_tics_(0),
    t_ms_(0.0),
    t_steps_(0)
{
  if ( t.ms_ > T_MAX_.t_ms_ )
    *this = T_POS_INF_;
  else if ( t.ms_ < T_MIN_.t_ms_ )
    *this = T_NEG_INF_;
  else
  {
    // convert to steps by first flooring double to tics,
    // then tics to steps
    t_steps_ = ms2tics_floor_(t.ms_) / TICS_PER_STEP_;

    // compute tics and ms value for number of steps
    t_tics_ = TICS_PER_STEP_ * t_steps_;
    update_ms_();
    
    /* We now need to add one step, unless the value in
       ms could be represented exactly in steps. We cannot
       add that step by default above and then check if it
       was superfluous since that would make stamps one step
       too large if the ms value can be represented exactly.
     */
    if ( t_ms_ < t.ms_ )
    {
      ++t_steps_;
      t_tics_ += TICS_PER_STEP_;
      update_ms_();
    }
  }
  
  assert(t.ms_ <= t_ms_);
}

nest::Time nest::Time::operator+=(const Time &t)
{
  const tic_t new_tics = t_tics_ + t.t_tics_;

  // check for over-/underflow
  if ( new_tics > T_MAX_.t_tics_ )
    *this = T_POS_INF_;
  else if ( new_tics  < T_MIN_.t_tics_ )
    *this = T_NEG_INF_;
  else
  {
    t_steps_ += t.t_steps_;  // faster than recomputing
    t_tics_= new_tics;
    update_ms_();
  }

  return *this;
}

nest::Time nest::Time::operator-=(const Time &t)
{
  const tic_t new_tics = t_tics_ - t.t_tics_;

  // check for over-/underflow
  if ( new_tics > T_MAX_.t_tics_ )
    *this = T_POS_INF_;
  else if ( new_tics < T_MIN_.t_tics_ )
    *this = T_NEG_INF_;
  else
  {
    t_steps_ -= t.t_steps_;
    t_tics_   = new_tics;
    update_ms_();
  }

  return *this;
}

nest::Time nest::Time::operator*=(long n)
{
  // we must check against overflow using division
  // since ranges are symmetric by definition, we 
  // can perform check using abs()
  if ( n == 0 ) 
  {
    t_steps_ = 0;
    t_tics_ = 0;
    t_ms_ = 0;
  }
  else if ( abs_(t_tics_) <= abs_(T_MAX_.t_tics_ / n) )
  { 
    t_steps_ *= n;
    t_tics_  *= n;
    update_ms_();
  }
  else if ( ( t_tics_ < 0 && n < 0 ) || ( t_tics_ > 0 && n > 0 ) )
    *this = T_POS_INF_;
  else
    *this = T_NEG_INF_;

  return *this;
}

nest::Time nest::operator-(Time const &t1)
{
  Time result;

  if ( t1.is_neg_inf() )
    result = Time::pos_inf();  // turn neg to pos inf 
  else if ( t1.is_pos_inf() )
    result = Time::neg_inf();  // turn pos to neg inf 
  else
  {
    result.t_tics_ = -t1.t_tics_;
    result.t_steps_= -t1.t_steps_;
    result.t_ms_   = -t1.t_ms_;
  }

  return result;
}

nest::Time nest::operator+(Time const &t1,Time const &t2)
{
  Time result(t1);
  result += t2;
  return result;
}

nest::Time nest::operator-(Time const &t1,Time const &t2)
{
  Time result(t1);
  result -= t2;
  return result;
}

nest::Time nest::operator*(Time const &t1, long_t t2)
{
  Time result(t1);
  result *= t2;
  return result;
}

nest::Time nest::operator*(const long_t t1, Time const &t2)
{
  Time result(t2);
  result *= t1;
  return result;
}

std::ostream& nest::operator<<(std::ostream& strm, const nest::Time& t)
{
  if ( t.is_neg_inf() )
    strm << "-INF";
  else if ( t.is_pos_inf() )
    strm << "+INF";
  else
    strm << t.get_ms() << " ms (= " << t.get_tics() << " tics = " 
    << t.get_steps() << ( t.get_steps() != 1 ? " steps)" : " step)");
  return strm;
}
