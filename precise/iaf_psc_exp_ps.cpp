/*
 *  iaf_psc_exp_ps.cpp
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

#include "iaf_psc_exp_ps.h"

#include "exceptions.h"
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

nest::RecordablesMap<nest::iaf_psc_exp_ps> nest::iaf_psc_exp_ps::recordablesMap_;

namespace nest
{
  // Override the create() method with one call to RecordablesMap::insert_() 
  // for each quantity to be recorded.
  template <>
  void RecordablesMap<iaf_psc_exp_ps>::create()
  {
    // use standard names whereever you can for consistency!
    insert_(names::V_m, & iaf_psc_exp_ps::get_V_m_);
  }
}

/* ---------------------------------------------------------------- 
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */
    
nest::iaf_psc_exp_ps::Parameters_::Parameters_()
  : tau_m_  ( 10.0     ),  // ms
    tau_ex_ (  2.0     ),  // ms
    tau_in_ (  2.0     ),  // ms
    c_m_    (250.0     ),  // pF
    t_ref_  (  2.0     ),  // ms
    E_L_    (-70.0     ),  // mV
    I_e_    (  0.0     ),  // pA
    U_th_   (-55.0-E_L_),  // mV, rel to E_L_
    U_min_  (-std::numeric_limits<double_t>::infinity()),  // mV
    U_reset_(-70.0-E_L_)   // mV, rel to E_L_
{}

nest::iaf_psc_exp_ps::State_::State_()
  : y0_(0.0),
    y1_ex_(0.0),
    y1_in_(0.0),
    y2_(0.0),
    is_refractory_(false),
    last_spike_step_(-1),
    last_spike_offset_(0.0)
{}

nest::iaf_psc_exp_ps::Buffers_::Buffers_(iaf_psc_exp_ps & n)
  : logger_(n)
{}

nest::iaf_psc_exp_ps::Buffers_::Buffers_(const Buffers_ &, iaf_psc_exp_ps & n)
  : logger_(n)
{}

/* ---------------------------------------------------------------- 
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void nest::iaf_psc_exp_ps::Parameters_::get(DictionaryDatum & d) const
{
  def<double>(d, names::E_L, E_L_);
  def<double>(d, names::I_e, I_e_);
  def<double>(d, names::V_th, U_th_+E_L_);
  def<double>(d, names::V_min, U_min_+E_L_);
  def<double>(d, names::V_reset, U_reset_+E_L_);
  def<double>(d, names::C_m, c_m_);
  def<double>(d, names::tau_m, tau_m_);
  def<double>(d, names::tau_syn_ex, tau_ex_);
  def<double>(d, names::tau_syn_in, tau_in_);
  def<double>(d, names::t_ref, t_ref_);
}

double nest::iaf_psc_exp_ps::Parameters_::set(const DictionaryDatum & d)
{
  // if E_L_ is changed, we need to adjust all variables defined relative to E_L_
  const double ELold = E_L_;
  updateValue<double>(d, names::E_L, E_L_);
  const double delta_EL = E_L_ - ELold;

  updateValue<double>(d, names::tau_m, tau_m_);
  updateValue<double>(d, names::tau_syn_ex, tau_ex_);
  updateValue<double>(d, names::tau_syn_in, tau_in_);
  updateValue<double>(d, names::C_m, c_m_);
  updateValue<double>(d, names::t_ref, t_ref_);
  updateValue<double>(d, names::I_e, I_e_);
  
  if ( updateValue<double>(d, names::V_th, U_th_) )
    U_th_ -= E_L_;
  else
    U_th_ -= delta_EL;
  
  if ( updateValue<double>(d, names::V_min, U_min_) )
    U_min_ -= E_L_;
  else
    U_min_ -= delta_EL;
  
  if ( updateValue<double>(d, names::V_reset, U_reset_) ) 
    U_reset_ -= E_L_;
  else
    U_reset_ -= delta_EL;
  
  if ( U_reset_ >= U_th_ )
    throw BadProperty("Reset potential must be smaller than threshold.");
  
  if ( U_reset_ < U_min_ )
    throw BadProperty("Reset potential must be greater equal minimum potential.");
  
  if ( c_m_ <= 0 )
    throw BadProperty("Capacitance must be strictly positive.");

  if ( Time(Time::ms(t_ref_)).get_steps() < 1 )
    throw BadProperty("Refractory time must be at least one time step.");

  if ( tau_m_ <= 0 || tau_ex_ <= 0 || tau_in_ <= 0 )
    throw BadProperty("All time constants must be strictly positive.");

  if ( tau_m_ == tau_ex_ || tau_m_ == tau_in_ )
    throw BadProperty("Membrane and synapse time constant(s) must differ."
		      "See note in documentation.");

  return delta_EL;
}

void nest::iaf_psc_exp_ps::State_::get(DictionaryDatum & d, 
                                       const Parameters_ & p) const
{
  def<double>(d, names::V_m, y2_ + p.E_L_);  // Membrane potential
  def<bool>(d, names::is_refractory, is_refractory_);
  def<double>(d, names::t_spike, Time(Time::step(last_spike_step_)).get_ms());
  def<double>(d, names::offset, last_spike_offset_);
}

void nest::iaf_psc_exp_ps::State_::set(const DictionaryDatum & d, const Parameters_ & p, double delta_EL)
{
  if ( updateValue<double>(d, names::V_m, y2_) )
    y2_ -= p.E_L_;
  else
    y2_ -= delta_EL;
}

/* ---------------------------------------------------------------- 
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::iaf_psc_exp_ps::iaf_psc_exp_ps()
  : Node(),
    P_(),
    S_(),
    B_(*this)
{
  recordablesMap_.create();
}

nest::iaf_psc_exp_ps::iaf_psc_exp_ps(const iaf_psc_exp_ps & n)
  : Node(n),
    P_(n.P_),
    S_(n.S_),
    B_(n.B_, *this)
{}

/* ---------------------------------------------------------------- 
 * Node initialization functions
 * ---------------------------------------------------------------- */

void nest::iaf_psc_exp_ps::init_state_(const Node & proto)
{
  const iaf_psc_exp_ps & pr = downcast<iaf_psc_exp_ps>(proto);

  S_ = pr.S_;
}

void nest::iaf_psc_exp_ps::init_buffers_()
{
  B_.events_.resize();
  B_.events_.clear(); 
  B_.currents_.clear();  // includes resize
  B_.logger_.reset();
}

void nest::iaf_psc_exp_ps::calibrate()
{
  B_.logger_.init();  // ensures initialization in case mm connected after Simulate
  
  V_.h_ms_ = Time::get_resolution().get_ms();
  
  V_.expm1_tau_m_  = numerics::expm1(-V_.h_ms_/P_.tau_m_);
  V_.expm1_tau_ex_ = numerics::expm1(-V_.h_ms_/P_.tau_ex_);
  V_.expm1_tau_in_ = numerics::expm1(-V_.h_ms_/P_.tau_in_);
  V_.P20_          = -P_.tau_m_ / P_.c_m_ * V_.expm1_tau_m_;
  V_.P21_ex_       = -P_.tau_m_*P_.tau_ex_ / (P_.tau_m_-P_.tau_ex_) / P_.c_m_ * (V_.expm1_tau_ex_-V_.expm1_tau_m_);
  V_.P21_in_       = -P_.tau_m_*P_.tau_in_ / (P_.tau_m_-P_.tau_in_) / P_.c_m_ * (V_.expm1_tau_in_-V_.expm1_tau_m_);
  
  V_.refractory_steps_ = Time(Time::ms(P_.t_ref_)).get_steps();
  assert(V_.refractory_steps_ >= 1);  // since t_ref_ >= sim step size, this can only fail in error
}

/* ---------------------------------------------------------------- 
 * Update and spike handling functions
 * ---------------------------------------------------------------- */

void nest::iaf_psc_exp_ps::update(const Time & origin, 
				  const long_t from, const long_t to)
{
  assert ( to >= 0 );
  assert ( static_cast<delay>(from) < Scheduler::get_min_delay() );
  assert ( from < to );
  
  // at start of slice, tell input queue to prepare for delivery
  if ( from == 0 )
    B_.events_.prepare_delivery();

  /* Neurons may have been initialized to superthreshold potentials.
     We need to check for this here and issue spikes at the beginning of
     the interval.
  */
  if ( S_.y2_ >= P_.U_th_ )
    emit_instant_spike_(origin, from, 
			V_.h_ms_*(1.0-std::numeric_limits<double_t>::epsilon()));

  for ( long_t lag = from; lag < to; ++lag )
  {
    // time at start of update step
    const long_t T = origin.get_steps() + lag;
    
    // if neuron returns from refractoriness during this step, place
    // pseudo-event in queue to mark end of refractory period
    if ( S_.is_refractory_ && ( T+1 - S_.last_spike_step_ == V_.refractory_steps_ ) )
      B_.events_.add_refractory(T, S_.last_spike_offset_);
    
    // save state at beginning of interval for spike-time approximation
    V_.y0_before_    = S_.y0_;
    V_.y1_ex_before_ = S_.y1_ex_;
    V_.y1_in_before_ = S_.y1_in_;
    V_.y2_before_    = S_.y2_;
    
    // get first event
    double_t ev_offset;
    double_t ev_weight;
    bool     end_of_refract;
    
    if ( !B_.events_.get_next_spike(T, ev_offset, ev_weight, end_of_refract) )
    {
      // No incoming spikes, handle with fixed propagator matrix.
      // Handling this case separately improves performance significantly
      // if there are many steps without input spikes.
      
      // update membrane potential
      if ( !S_.is_refractory_ )
      {
	S_.y2_ = V_.P20_*(P_.I_e_+S_.y0_) + V_.P21_ex_*S_.y1_ex_ +  V_.P21_in_*S_.y1_in_ + V_.expm1_tau_m_*S_.y2_ + S_.y2_;
	
	// lower bound of membrane potential
	S_.y2_ = ( S_.y2_ < P_.U_min_ ? P_.U_min_ : S_.y2_ ); 
      }
      
      // update synaptic currents
      S_.y1_ex_ = S_.y1_ex_*V_.expm1_tau_ex_ + S_.y1_ex_;
      S_.y1_in_ = S_.y1_in_*V_.expm1_tau_in_ + S_.y1_in_;
      
      /* The following must not be moved before the y1_, y2_ update,
	 since the spike-time interpolation within emit_spike_ depends 
	 on all state variables having their values at the end of the
	 interval. 
      */
      if ( S_.y2_ >= P_.U_th_ )
	emit_spike_(origin, lag, 0, V_.h_ms_);
    }
    else
    {
      // We only get here if there is at least on event, 
      // which has been read above.  We can therefore use 
      // a do-while loop.
      
      // Time within step is measured by offsets, which are h at the beginning
      // and 0 at the end of the step.
      double_t last_offset = V_.h_ms_;  // start of step
      
      do
      {
	// time is measured backward: inverse order in difference
	const double_t ministep = last_offset - ev_offset;
	
	propagate_(ministep);
	
	// check for threshold crossing during ministep
	// this must be done before adding the input, since
	// interpolation requires continuity
	if ( S_.y2_ >= P_.U_th_ ) 
	  emit_spike_(origin, lag, V_.h_ms_-last_offset, ministep);

	// handle event
	if ( end_of_refract )
	  S_.is_refractory_ = false;  // return from refractoriness
	else
	{
	  if ( ev_weight >= 0.0 )
	    S_.y1_ex_ += ev_weight;  // exc. spike input
	  else
	    S_.y1_in_ += ev_weight;  // inh. spike input
	}
	    
	// store state
	V_.y1_ex_before_ = S_.y1_ex_;
	V_.y1_in_before_ = S_.y1_in_;
	V_.y2_before_ = S_.y2_;
	last_offset = ev_offset;
      }
      while ( B_.events_.get_next_spike(T, ev_offset, ev_weight, 
					end_of_refract) );
      
      // no events remaining, plain update step across remainder 
      // of interval
      if ( last_offset > 0 )  // not at end of step, do remainder
      {
	propagate_(last_offset);
	if ( S_.y2_ >= P_.U_th_ ) 
	  emit_spike_(origin, lag, V_.h_ms_-last_offset, last_offset);
      }
    }  // else
    
    // Set new input current. The current change occurs at the
    // end of the interval and thus must come AFTER the threshold-
    // crossing approximation
    S_.y0_ = B_.currents_.get_value(lag);
    
    // log state data
    B_.logger_.record_data(origin.get_steps() + lag);
  }  // for
}

// function handles exact spike times
void nest::iaf_psc_exp_ps::handle(SpikeEvent & e)
{
  assert( e.get_delay() > 0 );

  /* We need to compute the absolute time stamp of the delivery time
     of the spike, since spikes might spend longer than min_delay_
     in the queue.  The time is computed according to Time Memo, Rule 3.
  */
  const long_t Tdeliver = e.get_stamp().get_steps() + e.get_delay() - 1;

  B_.events_.add_spike(e.get_rel_delivery_steps(network()->get_slice_origin()), 
		       Tdeliver, e.get_offset(), e.get_weight() * e.get_multiplicity());
}

void nest::iaf_psc_exp_ps::handle(CurrentEvent & e)
{
  assert( e.get_delay() > 0 );

  const double_t c = e.get_current();
  const double_t w = e.get_weight();

  // add weighted current; HEP 2002-10-04
  B_.currents_.add_value(e.get_rel_delivery_steps(network()->get_slice_origin()), 
			 w * c);
}

void nest::iaf_psc_exp_ps::handle(DataLoggingRequest &e)
{
  B_.logger_.handle(e);
}

// auxiliary functions ---------------------------------------------

inline 
void nest::iaf_psc_exp_ps::set_spiketime(const Time & now)
{
  S_.last_spike_step_ = now.get_steps();
}

void nest::iaf_psc_exp_ps::propagate_(const double_t dt)
{
  const double_t expm1_tau_ex = numerics::expm1(-dt/P_.tau_ex_);
  const double_t expm1_tau_in = numerics::expm1(-dt/P_.tau_in_);

  if ( !S_.is_refractory_ )
  {
    const double_t expm1_tau_m  = numerics::expm1(-dt/P_.tau_m_);
    
    const double_t P20    = -P_.tau_m_ / P_.c_m_ * expm1_tau_m;
    const double_t P21_ex = -P_.tau_m_*P_.tau_ex_ / (P_.tau_m_-P_.tau_ex_) / P_.c_m_ * (expm1_tau_ex-expm1_tau_m);
    const double_t P21_in = -P_.tau_m_*P_.tau_in_ / (P_.tau_m_-P_.tau_in_) / P_.c_m_ * (expm1_tau_in-expm1_tau_m);
    
    S_.y2_  = P20*(P_.I_e_+S_.y0_) + P21_ex*S_.y1_ex_ + P21_in*S_.y1_in_ + expm1_tau_m*S_.y2_ + S_.y2_;
  }
  S_.y1_ex_ = S_.y1_ex_*expm1_tau_ex + S_.y1_ex_;
  S_.y1_in_ = S_.y1_in_*expm1_tau_in + S_.y1_in_;
}

void nest::iaf_psc_exp_ps::emit_spike_(const Time & origin, const long_t lag, 
				       const double_t t0,  const double_t dt)
{
  // we know that the potential is subthreshold at t0, super at t0+dt
  
  // compute spike time relative to beginning of step
  const double_t spike_offset = V_.h_ms_ - (t0 + bisectioning_(dt));
  
  set_spiketime(Time::step(origin.get_steps() + lag + 1));
  S_.last_spike_offset_ = spike_offset;  
  
  // reset neuron and make it refractory
  S_.y2_ = P_.U_reset_;
  S_.is_refractory_ = true;

  // send spike
  SpikeEvent se;
  
  se.set_offset(spike_offset);
  network()->send(*this, se, lag);
}

void nest::iaf_psc_exp_ps::emit_instant_spike_(const Time & origin, const long_t lag,
					       const double_t spike_offs) 
{
  assert( S_.y2_ >= P_.U_th_ );  // ensure we are superthreshold
  
  // set stamp and offset for spike
  set_spiketime(Time::step(origin.get_steps() + lag + 1));
  S_.last_spike_offset_ = spike_offs;
  
  // reset neuron and make it refractory
  S_.y2_ = P_.U_reset_;
  S_.is_refractory_ = true;

  // send spike
  SpikeEvent se;
  
  se.set_offset(S_.last_spike_offset_);
  network()->send(*this, se, lag);
}

nest::double_t nest::iaf_psc_exp_ps::bisectioning_(const double_t dt) const
{
  double_t root = 0.0;

  double_t y2_root = V_.y2_before_;

  double_t div = 2.0;

  while ( fabs(P_.U_th_-y2_root) > 1e-14 )
  {
    if ( y2_root > P_.U_th_ )
      root -= dt/div;
    else
      root += dt/div;
    
    div *= 2.0;
    
    const double_t expm1_tau_ex = numerics::expm1(-root/P_.tau_ex_);
    const double_t expm1_tau_in = numerics::expm1(-root/P_.tau_in_);
    const double_t expm1_tau_m  = numerics::expm1(-root/P_.tau_m_);
    
    const double_t P20    = -P_.tau_m_ / P_.c_m_ * expm1_tau_m;
    const double_t P21_ex = -P_.tau_m_*P_.tau_ex_ / (P_.tau_m_-P_.tau_ex_) / P_.c_m_ * (expm1_tau_ex-expm1_tau_m);
    const double_t P21_in = -P_.tau_m_*P_.tau_in_ / (P_.tau_m_-P_.tau_in_) / P_.c_m_ * (expm1_tau_in-expm1_tau_m);
    
    y2_root = P20*(P_.I_e_+V_.y0_before_) + P21_ex*V_.y1_ex_before_ + P21_in*V_.y1_in_before_ + expm1_tau_m*V_.y2_before_ + V_.y2_before_;
  }
  return root;
}
