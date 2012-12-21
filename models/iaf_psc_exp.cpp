/*
 *  iaf_psc_exp.cpp
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
#include "iaf_psc_exp.h"
#include "network.h"
#include "dict.h"
#include "integerdatum.h"
#include "doubledatum.h"
#include "dictutils.h"
#include "numerics.h"
#include "universal_data_logger_impl.h"

#include <limits>

/* ---------------------------------------------------------------- 
 * Recordables map
 * ---------------------------------------------------------------- */

nest::RecordablesMap<nest::iaf_psc_exp> nest::iaf_psc_exp::recordablesMap_;

namespace nest
{
  // Override the create() method with one call to RecordablesMap::insert_() 
  // for each quantity to be recorded.
  template <>
  void RecordablesMap<iaf_psc_exp>::create()
  {
    // use standard names whereever you can for consistency!
    insert_(names::V_m, &iaf_psc_exp::get_V_m_);
  }
}

/* ---------------------------------------------------------------- 
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */
    
nest::iaf_psc_exp::Parameters_::Parameters_()
  : Tau_     (  10.0       ),  // in ms
    C_       ( 250.0       ),  // in pF
    t_ref_   (   2.0       ),  // in ms
    U0_      ( -70.0       ),  // in mV
    I_e_     (   0.0       ),  // in pA
    Theta_   ( -55.0 - U0_ ),  // relative U0_
    V_reset_ ( -70.0 - U0_ ),  // in mV
    tau_ex_  (   2.0       ),  // in ms
    tau_in_  (   2.0       )   // in ms
{}

nest::iaf_psc_exp::State_::State_()
  : i_0_      ( 0.0 ),
    i_syn_ex_ ( 0.0 ),
    i_syn_in_ ( 0.0 ),
    V_m_      ( 0.0 ),
    r_ref_    ( 0   )
{}

/* ---------------------------------------------------------------- 
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void nest::iaf_psc_exp::Parameters_::get(DictionaryDatum &d) const
{
  def<double>(d, names::E_L,        U0_);         // resting potential
  def<double>(d, names::I_e,        I_e_);
  def<double>(d, names::V_th,       Theta_+U0_);  // threshold value
  def<double>(d, names::V_reset,    V_reset_+U0_);
  def<double>(d, names::C_m,        C_);
  def<double>(d, names::tau_m,      Tau_);
  def<double>(d, names::tau_syn_ex, tau_ex_);
  def<double>(d, names::tau_syn_in, tau_in_);
  def<double>(d, names::t_ref,      t_ref_);
}

double nest::iaf_psc_exp::Parameters_::set(const DictionaryDatum &d)
{
  // if U0_ is changed, we need to adjust all variables defined relative to U0_
  const double ELold = U0_;
  updateValue<double>(d, names::E_L, U0_);
  const double delta_EL = U0_ - ELold;

  if(updateValue<double>(d, names::V_reset, V_reset_))
    V_reset_ -= U0_;
  else
    V_reset_ -= delta_EL;

  if (updateValue<double>(d, names::V_th, Theta_)) 
    Theta_ -= U0_;
  else
    Theta_ -= delta_EL;

  updateValue<double>(d, names::I_e,        I_e_);
  updateValue<double>(d, names::C_m,        C_);
  updateValue<double>(d, names::tau_m,      Tau_);
  updateValue<double>(d, names::tau_syn_ex, tau_ex_);
  updateValue<double>(d, names::tau_syn_in, tau_in_);
  updateValue<double>(d, names::t_ref,      t_ref_);

  if ( V_reset_ >= Theta_ )
    throw BadProperty("Reset potential must be smaller than threshold.");
      
  if ( C_ <= 0 )
    throw BadProperty("Capacitance must be strictly positive.");
    
  if ( Tau_ <= 0 || tau_ex_ <= 0 || tau_in_ <= 0 || 
       t_ref_ <= 0 )
    throw BadProperty("All time constants must be strictly positive.");

  if ( Tau_ == tau_ex_ || Tau_ == tau_in_ )
    throw BadProperty("Membrane and synapse time constant(s) must differ."
		      "See note in documentation.");

  return delta_EL;
}

void nest::iaf_psc_exp::State_::get(DictionaryDatum &d, const Parameters_ &p) const
{
  def<double>(d, names::V_m, V_m_ + p.U0_); // Membrane potential
}

void nest::iaf_psc_exp::State_::set(const DictionaryDatum &d, 
				    const Parameters_ &p, double delta_EL)
{
  if ( updateValue<double>(d, names::V_m, V_m_) )
    V_m_ -= p.U0_;
  else
    V_m_ -= delta_EL;
}

nest::iaf_psc_exp::Buffers_::Buffers_(iaf_psc_exp &n)
  : logger_(n)
{}

nest::iaf_psc_exp::Buffers_::Buffers_(const Buffers_ &, iaf_psc_exp &n)
  : logger_(n)
{}

/* ---------------------------------------------------------------- 
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::iaf_psc_exp::iaf_psc_exp()
  : Archiving_Node(), 
    P_(), 
    S_(),
    B_(*this)
{
  recordablesMap_.create();
}

nest::iaf_psc_exp::iaf_psc_exp(const iaf_psc_exp &n)
  : Archiving_Node(n), 
    P_(n.P_), 
    S_(n.S_),
    B_(n.B_, *this)
{}

/* ---------------------------------------------------------------- 
 * Node initialization functions
 * ---------------------------------------------------------------- */

void nest::iaf_psc_exp::init_state_(const Node &proto)
{
  const iaf_psc_exp &pr = downcast<iaf_psc_exp>(proto);
  S_ = pr.S_;
}

void nest::iaf_psc_exp::init_buffers_()
{
  B_.spikes_ex_.clear();        // includes resize
  B_.spikes_in_.clear();        // includes resize
  B_.currents_.clear();         // includes resize
  B_.logger_.reset();
  Archiving_Node::clear_history();
}

void nest::iaf_psc_exp::calibrate()
{
  B_.logger_.init();  // ensures initialization in case mm connected after Simulate

  const double h = Time::get_resolution().get_ms(); 

  // numbering of state vaiables: i_0 = 0, i_syn_ = 1, V_m_ = 2

  // commented out propagators: forward Euler
  // needed to exactly reproduce Tsodyks network
 
  // these P are independent
  V_.P11ex_ = std::exp(-h/P_.tau_ex_);
  //P11ex_ = 1.0-h/tau_ex_;

  V_.P11in_ = std::exp(-h/P_.tau_in_);
  //P11in_ = 1.0-h/tau_in_;

  V_.P22_ = std::exp(-h/P_.Tau_);
  //P22_ = 1.0-h/Tau_;

  // these depend on the above. Please do not change the order.
  // TODO: use expm1 here to improve accuracy for small timesteps

  V_.P21ex_ = P_.Tau_/(P_.C_*(1.0-P_.Tau_/P_.tau_ex_)) * V_.P11ex_ * (1.0 - std::exp(h*(1.0/P_.tau_ex_-1.0/P_.Tau_)));
  //P21ex_ = h/C_;

  V_.P21in_ = P_.Tau_/(P_.C_*(1.0-P_.Tau_/P_.tau_in_)) * V_.P11in_ * (1.0 - std::exp(h*(1.0/P_.tau_in_-1.0/P_.Tau_)));
  //P21in_ = h/C_;

  V_.P20_ = P_.Tau_/P_.C_*(1.0 - V_.P22_);
  //P20_ = h/C_;

  // TauR specifies the length of the absolute refractory period as 
  // a double_t in ms. The grid based iaf_psc_exp can only handle refractory
  // periods that are integer multiples of the computation step size (h).
  // To ensure consistency with the overall simulation scheme such conversion
  // should be carried out via objects of class nest::Time. The conversion 
  // requires 2 steps:
  //     1. A time object r is constructed defining  representation of 
  //        TauR in tics. This representation is then converted to computation time
  //        steps again by a strategy defined by class nest::Time.
  //     2. The refractory time in units of steps is read out get_steps(), a member
  //        function of class nest::Time.
  //
  // Choosing a TauR that is not an integer multiple of the computation time 
  // step h will leed to accurate (up to the resolution h) and self-consistent
  // results. However, a neuron model capable of operating with real valued spike
  // time may exhibit a different effective refractory time.
  //
  V_.RefractoryCounts_ = Time(Time::ms(P_.t_ref_)).get_steps();

  if ( V_.RefractoryCounts_ < 1 )
    throw BadProperty("Absolute refractory time must be at least one time step.");
}

void nest::iaf_psc_exp::update(const Time &origin, const long_t from, const long_t to)
{
  assert(to >= 0 && (delay) from < Scheduler::get_min_delay());
  assert(from < to);

  // evolve from timestep 'from' to timestep 'to' with steps of h each
  for ( long_t lag = from; lag < to; ++lag )
  {	
    if ( S_.r_ref_ == 0 ) // neuron not refractory, so evolve V
      S_.V_m_ = S_.V_m_*V_.P22_ + S_.i_syn_ex_*V_.P21ex_ + S_.i_syn_in_*V_.P21in_ + (P_.I_e_+S_.i_0_)*V_.P20_; 
    else 
      --S_.r_ref_; // neuron is absolute refractory

    // exponential decaying PSCs
    S_.i_syn_ex_ *= V_.P11ex_;
    S_.i_syn_in_ *= V_.P11in_;

    // the spikes arriving at T+1 have an immediate effect on the state of the neuron
    S_.i_syn_ex_ += B_.spikes_ex_.get_value(lag);
    S_.i_syn_in_ += B_.spikes_in_.get_value(lag);
                                                       
    if ( S_.V_m_ >= P_.Theta_ )  // threshold crossing
    {
      S_.r_ref_ = V_.RefractoryCounts_;
      S_.V_m_ = P_.V_reset_;
        
      set_spiketime(Time::step(origin.get_steps()+lag+1));
	    
      SpikeEvent se;
      network()->send(*this, se, lag);
    }

    // set new input current
    S_.i_0_ = B_.currents_.get_value(lag);

    // log state data
    B_.logger_.record_data(origin.get_steps() + lag);

  }  
}                           
                     
void nest::iaf_psc_exp::handle(SpikeEvent &e)
{
  assert ( e.get_delay() > 0 );

  if ( e.get_weight() >= 0.0 )
    B_.spikes_ex_.add_value(e.get_rel_delivery_steps(network()->get_slice_origin()), 
			    e.get_weight() * e.get_multiplicity() );
  else
    B_.spikes_in_.add_value(e.get_rel_delivery_steps(network()->get_slice_origin()), 
			    e.get_weight() * e.get_multiplicity() );
}

void nest::iaf_psc_exp::handle(CurrentEvent &e)
{
  assert ( e.get_delay() > 0 );

  const double_t c=e.get_current();
  const double_t w=e.get_weight();

  // add weighted current; HEP 2002-10-04
  B_.currents_.add_value(e.get_rel_delivery_steps(network()->get_slice_origin()), w*c);
}

void nest::iaf_psc_exp::handle(DataLoggingRequest &e)
{
  B_.logger_.handle(e);
}
