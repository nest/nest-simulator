/*
 *  smp_generator.cpp
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

#include "exceptions.h"
#include "smp_generator.h"
#include "network.h"
#include "dict.h"
#include "integerdatum.h"
#include "doubledatum.h"
#include "arraydatum.h"
#include "dictutils.h"
#include "numerics.h"
#include "universal_data_logger_impl.h"

#include <cmath>
#include <limits>

namespace nest {
  RecordablesMap<smp_generator> smp_generator::recordablesMap_;

  template <>
  void RecordablesMap<smp_generator>::create()
  {
    insert_(Name("Rate"), &smp_generator::get_rate_);
  }
}

/* ---------------------------------------------------------------- 
 * Default constructors defining default parameter
 * ---------------------------------------------------------------- */
    
nest::smp_generator::Parameters_::Parameters_()
  : om_(0.0),    // radian/s
    phi_(0.0),   // radian
    dc_(0.0),    // spikes/s
    ac_(0.0)     // spikes/s
{}

nest::smp_generator::Parameters_::Parameters_(const Parameters_& p )
  : om_(p.om_),
    phi_(p.phi_),
    dc_(p.dc_),
    ac_(p.ac_)
{}

nest::smp_generator::Parameters_& 
nest::smp_generator::Parameters_::operator=(const Parameters_& p)
{
  if ( this == &p )
    return *this;

  dc_ = p.dc_;
  om_ = p.om_;
  phi_ = p.phi_;
  ac_ = p.ac_;

  return *this;
}

nest::smp_generator::State_::State_()
  : rate_(0),
    last_spike_(Time::step(-1))
{}


nest::smp_generator::Buffers_::Buffers_(smp_generator& n)
  : logger_(n)
{}

nest::smp_generator::Buffers_::Buffers_(const Buffers_&, smp_generator& n)
  : logger_(n)
{}


/* ---------------------------------------------------------------- 
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void nest::smp_generator::Parameters_::get(DictionaryDatum &d) const
{
  (*d)["dc"]  = dc_ * 1000.0; 
  (*d)["freq"]= om_ / ( 2.0 * numerics::pi / 1000.0);
  (*d)["phi"] = phi_;
  (*d)["ac"]  = ac_ * 1000.0;
} 

void nest::smp_generator::State_::get(DictionaryDatum &d) const
{
  (*d)["last_spike"] = last_spike_.get_ms();
}  

void nest::smp_generator::Parameters_::set(const DictionaryDatum& d)
{
  if ( updateValue<double_t>(d, "dc", dc_) )
    dc_ /= 1000.0;           // scale to ms^-1
  
  if ( updateValue<double_t>(d, "freq", om_) )
    om_ *= 2.0 * numerics::pi / 1000.0;

  updateValue<double_t>(d, "phi", phi_);

  if ( updateValue<double_t>(d, "ac", ac_) )
      ac_ /= 1000.0;
}

/* ---------------------------------------------------------------- 
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::smp_generator::smp_generator()
  : Node(),
    device_(),
    P_(),
    S_(),
    B_(*this)
{
    recordablesMap_.create();
}

nest::smp_generator::smp_generator(const smp_generator&n)
  : Node(n),
    device_(n.device_),
    P_(n.P_),
    S_(n.S_),
    B_(n.B_, *this)
{
}

/* ---------------------------------------------------------------- 
 * Node initialization functions
 * ---------------------------------------------------------------- */

void nest::smp_generator::init_state_(const Node& proto)
{ 
  const smp_generator& pr = downcast<smp_generator>(proto);
  
  device_.init_state(pr.device_);
  S_ = pr.S_;
}

void nest::smp_generator::init_buffers_()
{ 
  device_.init_buffers();
  B_.logger_.reset();
}

void nest::smp_generator::calibrate()
{
  B_.logger_.init();  // ensures initialization in case mm connected after Simulate

  device_.calibrate();

  // time resolution
  const double h = Time::get_resolution().get_ms(); 
  const double_t t = network()->get_time().get_ms();

  // initial state
  S_.y_0_ = P_.ac_ * std::cos(P_.om_ * t + P_.phi_);
  S_.y_1_ = P_.ac_ * std::sin(P_.om_ * t + P_.phi_);

  V_.sin_ = std::sin(h * P_.om_);         // block elements
  V_.cos_ = std::cos(h * P_.om_);

  return;
}

void nest::smp_generator::update(Time const& origin, 
			 const long_t from, const long_t to)
{
  assert(to >= 0 && (delay) from < Scheduler::get_min_delay());
  assert(from < to);

  const long_t start = origin.get_steps();

  // time resolution
  const double h = Time::get_resolution().get_ms(); 

  // random number generator
  librandom::RngPtr rng = net_->get_rng(get_thread());

  // We iterate the dynamics even when the device is turned off,
  // but do not issue spikes while it is off. In this way, the 
  // oscillators always have the right phase.  This is quite 
  // time-consuming, so it should be done only if the device is
  // on most of the time.

  for ( long_t lag = from ; lag < to ; ++lag )
  {
    // update oscillator blocks, accumulate rate as sum of DC and N_osc_ AC elements
    // rate is instantaneous sum of state
    S_.rate_ = P_.dc_;

    const double_t new_y_0 = V_.cos_ * S_.y_0_ - V_.sin_ * S_.y_1_;
    
    S_.y_1_ = V_.sin_ * S_.y_0_ + V_.cos_ * S_.y_1_;
    S_.y_0_ = new_y_0;
    S_.rate_ += S_.y_1_;
    
    if ( S_.rate_ < 0 )
      S_.rate_ = 0;
  
    // store rate in Hz
    B_.logger_.record_data(origin.get_steps()+lag);

    // create spikes
    if ( S_.rate_ > 0 && device_.is_active(Time::step(start+lag)) )
    {
      V_.poisson_dev_.set_lambda(S_.rate_ * h);
      ulong_t n_spikes = V_.poisson_dev_.uldev(rng);
      while ( n_spikes-- )
      {
      	SpikeEvent se;
        network()->send(*this, se, lag);
        S_.last_spike_ = Time::step(origin.get_steps()+lag+1);
      }
    }

  }
}                       

void nest::smp_generator::handle(DataLoggingRequest& e)
{
  B_.logger_.handle(e);
}




