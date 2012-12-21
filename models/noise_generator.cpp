/*
 *  noise_generator.cpp
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

#include "noise_generator.h"
#include "network.h"
#include "dict.h"
#include "integerdatum.h"
#include "doubledatum.h"
#include "dictutils.h"

/* ---------------------------------------------------------------- 
 * Default constructors defining default parameter
 * ---------------------------------------------------------------- */
    
nest::noise_generator::Parameters_::Parameters_()
  : mean_(0.0),  // pA
    std_(0.0),   // pA / sqrt(s)
    dt_(Time::ms(1.0)),
    num_targets_(0)
{}

nest::noise_generator::Parameters_::Parameters_(const Parameters_& p)
  : mean_(p.mean_),
    std_(p.std_),
    dt_(p.dt_),
    num_targets_(0)  // we do not copy connections
{
  // do not check validity of dt_ here, otherwise we cannot copy
  // to temporary in set(); see node copy c'tor
  dt_.calibrate();
}


/* ---------------------------------------------------------------- 
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void nest::noise_generator::Parameters_::get(DictionaryDatum &d) const
{
  (*d)[names::mean] = mean_;
  (*d)[names::std ] = std_;
  (*d)[names::dt]   = dt_.get_ms();
}  

void nest::noise_generator::Parameters_::set(const DictionaryDatum& d,
                                             const noise_generator& n)
{
  updateValue<double_t>(d, names::mean, mean_);
  updateValue<double_t>(d, names::std , std_);
  double_t dt;
  if ( updateValue<double_t>(d, names::dt, dt) )
    dt_ = Time::ms(dt);
  
  if ( std_ < 0 )
    throw BadProperty("The standard deviation cannot be negative.");
    
  if ( !dt_.is_step() )
    throw StepMultipleRequired(n.get_name(), names::dt, dt_);
}


/* ---------------------------------------------------------------- 
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::noise_generator::noise_generator()
  : Node(),
    device_(), 
    P_()
{
  if ( !P_.dt_.is_step() )
    throw InvalidDefaultResolution(get_name(), names::dt, P_.dt_);
}

nest::noise_generator::noise_generator(const noise_generator& n)
  : Node(n), 
    device_(n.device_),
    P_(n.P_)
{
  if ( !P_.dt_.is_step() )
    throw InvalidTimeInModel(get_name(), names::dt, P_.dt_);
}


/* ---------------------------------------------------------------- 
 * Node initialization functions
 * ---------------------------------------------------------------- */

void nest::noise_generator::init_state_(const Node& proto)
{ 
  const noise_generator& pr = downcast<noise_generator>(proto);

  device_.init_state(pr.device_);
}

void nest::noise_generator::init_buffers_()
{ 
  device_.init_buffers();
  
  B_.next_step_ = 0;
  B_.amps_.clear();
  B_.amps_.resize(P_.num_targets_, 0.0);
}

void nest::noise_generator::calibrate()
{
  device_.calibrate();
  if ( P_.num_targets_ != B_.amps_.size() )
  {
    network()->message(SLIInterpreter::M_INFO, "noise_generator::calibrate()",
      "The number of targets has changed, drawing new amplitudes.");
    init_buffers_();
  }

  V_.dt_steps_ = P_.dt_.get_steps();
}


/* ---------------------------------------------------------------- 
 * Update function and event hook
 * ---------------------------------------------------------------- */


nest::port nest::noise_generator::check_connection(Connection& c, port receptor_type)
{
    DSCurrentEvent e;
    e.set_sender(*this);
    c.check_event(e);
    const port receptor = c.get_target()->connect_sender(e, receptor_type);
    ++P_.num_targets_;
    return receptor;
}

//
// Time Evolution Operator
//
void nest::noise_generator::update(Time const &origin, const long_t from, const long_t to)
{
  const long_t start = origin.get_steps();

  for ( long_t offs = from ; offs < to ; ++offs )
  {
    const long_t now = start + offs;
    
    if ( !device_.is_active(Time::step(now)) )
      continue;
    
    // >= in case we woke from inactivity  
    if( now >= B_.next_step_ )
    {
      // compute new currents
      for ( AmpVec_::iterator it = B_.amps_.begin() ;
            it != B_.amps_.end() ; ++it )
        *it = P_.mean_ + P_.std_ * V_.normal_dev_(net_->get_rng(get_thread()));

      // use now as reference, in case we woke up from inactive period
      B_.next_step_ = now + V_.dt_steps_;
    }
    
    DSCurrentEvent ce;
    network()->send(*this, ce, offs);
  }
}

void nest::noise_generator::event_hook(DSCurrentEvent& e)
{
  // get port number
  const port prt = e.get_port();

  // we handle only one port here, get reference to vector elem
  assert(0 <= prt && static_cast<size_t>(prt) < B_.amps_.size());

  e.set_current(B_.amps_[prt]);
  e.get_receiver().handle(e);
}
