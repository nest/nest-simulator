/*
 *  iaf_psc_exp_ps_time_reversal.cpp
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
#include "iaf_psc_exp_ps_time_reversal.h"

#include "exceptions.h"
#include "dict.h"
#include "integerdatum.h"
#include "doubledatum.h"
#include "dictutils.h"
#include "universal_data_logger_impl.h"
#include "arraydatum.h"

#include <limits>

/* ---------------------------------------------------------------- 
 * Recordables map
 * ---------------------------------------------------------------- */

nest::RecordablesMap<nest::iaf_psc_exp_ps_time_reversal> nest::iaf_psc_exp_ps_time_reversal::recordablesMap_;

namespace nest
{
  // Override the create() method with one call to RecordablesMap::insert_() 
  // for each quantity to be recorded.
  template <>
  void RecordablesMap<iaf_psc_exp_ps_time_reversal>::create()
  {
    // use standard names whereever you can for consistency!
    insert_(names::V_m, & iaf_psc_exp_ps_time_reversal::get_V_m_);
    insert_(names::I_syn, & iaf_psc_exp_ps_time_reversal::get_I_syn_);
    insert_(names::y1_ex, & iaf_psc_exp_ps_time_reversal::get_y1_ex_);
    insert_(names::y1_in, &iaf_psc_exp_ps_time_reversal::get_y1_in_);
    insert_(names::y0, &iaf_psc_exp_ps_time_reversal::get_y0_);

  }
}

/* ---------------------------------------------------------------- 
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */
    
nest::iaf_psc_exp_ps_time_reversal::Parameters_::Parameters_()
  : tau_m_  ( 10.0     ),  // ms
    tau_ex_ (  2.0     ),  // ms
    tau_in_ (  2.0     ),  // ms
    c_m_    (250.0     ),  // pF
    t_ref_  (  2.0     ),  // ms
    E_L_    (  0.0   ),  // mV
    I_e_    (  0.0     ),  // pA
    U_th_   ( -55.0-E_L_),  // mV, rel to E_L_
    U_min_  (-std::numeric_limits<double_t>::infinity()),  // mV
    U_reset_( -70.0-E_L_)   // mV, rel to E_L_
{
  calc_const_spike_test_();
}

nest::iaf_psc_exp_ps_time_reversal::State_::State_()
  : y0_(0.0),
    y1_ex_(0.0),
    y1_in_(0.0),
    y2_(0.0),
    is_refractory_(false),
    last_spike_step_(-1),
    last_spike_offset_(0.0),
    dhaene_quick1(0),
    dhaene_quick2(0),
    dhaene_tmax_lt_t1(0),
    dhaene_max(0),
    dhaene_det_spikes(0),
    c0(0),
    c1a(0),
    c1b(0),
    c3a(0),
    c3b(0),
    c4(0),
    det_spikes(0)
{}

nest::iaf_psc_exp_ps_time_reversal::Buffers_::Buffers_(iaf_psc_exp_ps_time_reversal & n)
  : logger_(n)
{}

nest::iaf_psc_exp_ps_time_reversal::Buffers_::Buffers_(const Buffers_ &, iaf_psc_exp_ps_time_reversal & n)
  : logger_(n)
{}

/* ---------------------------------------------------------------- 
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */


//constants for time-reversal state space spike-detection algorithm

void nest::iaf_psc_exp_ps_time_reversal::Parameters_::calc_const_spike_test_()
{
  //line corresponding to the final timestep i.e t_right: continuation
  // of the curved boundary: a + I*b
    a1_ =tau_m_ * tau_ex_;
    a2_ =tau_m_ * (tau_m_ - tau_ex_);
    a3_ =c_m_ * U_th_ * (tau_m_ - tau_ex_);
    a4_ =c_m_ * (tau_m_ - tau_ex_);

    //line joining endpoints of the envelope: \alpha I + \beta
    b1_ =-tau_m_ * tau_m_;
    b2_ =tau_m_ * tau_ex_;
    b3_ =tau_m_*(tau_m_ - tau_ex_)- tau_m_*tau_m_ + tau_m_* tau_ex_;
    b4_ =-tau_m_*tau_m_; //b1
    b5_ =tau_m_*c_m_*U_th_;
    b6_ =tau_m_*(tau_m_- tau_ex_);
    b7_ =-c_m_*(tau_m_ - tau_ex_);

    //envelope or curved boundary
    c1_ =tau_m_/c_m_;
    c2_ =(-tau_m_ * tau_ex_)/(c_m_*(tau_m_ - tau_ex_));
    c3_ =(tau_m_ * tau_m_)/ (c_m_ * (tau_m_ - tau_ex_));
    c4_ =tau_ex_/tau_m_;
    c5_ =(c_m_ * U_th_)/tau_m_;
    c6_ =1-(tau_ex_/tau_m_);

    //parallel line

    d1_ = tau_m_ *  c_m_ ;
    d2_ = tau_m_ * tau_ex_;
    d3_ = c_m_ * (tau_m_ - tau_ex_);
}

void nest::iaf_psc_exp_ps_time_reversal::Parameters_::get(DictionaryDatum & d) const
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

double nest::iaf_psc_exp_ps_time_reversal::Parameters_::set(const DictionaryDatum & d)
{

  updateValue<double>(d, names::tau_m, tau_m_);
  updateValue<double>(d, names::tau_syn_ex, tau_ex_);
  updateValue<double>(d, names::tau_syn_in, tau_in_);
  updateValue<double>(d, names::C_m, c_m_);
  updateValue<double>(d, names::t_ref, t_ref_);
  updateValue<double>(d, names::I_e, I_e_);


  // if U0_ is changed, we need to adjust all variables defined relative to U0_
  const double ELold = E_L_;
  updateValue<double>(d, names::E_L, E_L_);
  const double delta_EL = E_L_ - ELold;

  if(updateValue<double>(d, names::V_reset, U_reset_))
    U_reset_ -= E_L_;
  else
    U_reset_ -= delta_EL;

  if (updateValue<double>(d, names::V_th, U_th_))
    U_th_ -= E_L_;
  else
    U_th_ -= delta_EL;

  if (updateValue<double>(d, names::V_min, U_min_))
    U_min_ -= E_L_;
  else
    U_min_ -= delta_EL;
  
  if ( U_reset_ >= U_th_ )
    throw BadProperty("Reset potential must be smaller than threshold.");
  
  if ( U_reset_ < U_min_ )
    throw BadProperty("Reset potential must be greater equal minimum potential.");
  
  if ( c_m_ <= 0 )
    throw BadProperty("Capacitance must be strictly positive.");
  
  if ( t_ref_ < 0 )
    throw BadProperty("Refractory time must not be negative.");
  
  if ( tau_m_ <= 0 || tau_ex_ <= 0 || tau_in_ <= 0 )
    throw BadProperty("All time constants must be strictly positive.");

   if ( tau_m_ == tau_ex_ || tau_m_ == tau_in_ )
    throw BadProperty("Membrane and synapse time constant(s) must differ."
                      "See note in documentation.");

   calc_const_spike_test_();

   return delta_EL;
}

void nest::iaf_psc_exp_ps_time_reversal::State_::get(DictionaryDatum & d, 
                                       const Parameters_ & p) const
{
  def<double>(d, names::V_m, y2_ + p.E_L_);  // Membrane potential
  def<bool>(d, names::is_refractory, is_refractory_);
  def<double>(d, names::t_spike, Time(Time::step(last_spike_step_)).get_ms());
  def<double>(d, names::offset, last_spike_offset_);
  def<double>(d, names::y1_ex, y1_ex_);
  def<double>(d, names::y1_in, y1_in_); // y1 state
  def<double>(d, names::y2, y2_); // y2 state
  def<double>(d, names::I_syn, y1_ex_ + y1_in_); 


  // these entries will break ticket-459
  // because they chane depending on E_L (which is correct)
  // since they are only used for debugging we will comment them
  // out for the time being
  //def<long>(d, names::pot_spikes, c0);
  //def<double>(d, names::dhaene_quick1, (dhaene_quick1*100.0/c0));
  //def<double>(d, names::dhaene_quick2, (dhaene_quick2*100.0/c0));
  //def<double>(d, names::dhaene_tmax_lt_t1, (dhaene_tmax_lt_t1*100.0/c0));
  //def<double>(d, names::dhaene_max_geq_V_th, (dhaene_max*100.0/c0));
  //def<double>(d, names::dhaene_det_spikes, (dhaene_det_spikes*100.0/c0));

  //def<double>(d, names::eq7, (c1a*100.0/c0));
  //def<double>(d, names::eq9, (c1b*100.0/c0));
  //def<double>(d, names::eqs7and9, (c2*100.0/c0));
  //def<double>(d, names::lin_left_geq_V_th, (c3a*100.0/c0));
  //def<double>(d, names::lin_max_geq_V_th, (c3b*100.0/c0));
  //def<double>(d, names::eq13, (c4*100.0/c0));
  //def<double>(d, names::eq12, (det_spikes*100.0/c0));
}

void nest::iaf_psc_exp_ps_time_reversal::State_::set(const DictionaryDatum & d, const Parameters_ & p, double delta_EL)
{
  if ( updateValue<double>(d, names::V_m, y2_) )
    y2_ -= p.E_L_;
  else
    y2_ -= delta_EL;

  //double_t dv;
  //updateValue<double>(d, names::dhaene_quick1, dv);
  //updateValue<double>(d, names::dhaene_quick2, dv);
  //updateValue<double>(d, names::dhaene_tmax_lt_t1, dv);
  //updateValue<double>(d, names::dhaene_max_geq_V_th, dv);
  //updateValue<double>(d, names::dhaene_det_spikes, dv);
  //updateValue<double>(d, names::eq7, dv);
  //updateValue<double>(d, names::eq9, dv);
  //updateValue<double>(d, names::eqs7and9, dv);
  //updateValue<double>(d, names::lin_left_geq_V_th, dv);
  //updateValue<double>(d, names::lin_max_geq_V_th, dv);
  //updateValue<double>(d, names::eq13, dv);
  //updateValue<double>(d, names::eq12, dv);
  updateValue<double>(d, names::y1_ex, y1_ex_ );
  updateValue<double>(d, names::y1_in, y1_in_);
  updateValue<double>(d, names::y0, y0_);


}

/* ---------------------------------------------------------------- 
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::iaf_psc_exp_ps_time_reversal::iaf_psc_exp_ps_time_reversal()
  : Node(),
    P_(),
    S_(),
    B_(*this)
{
  recordablesMap_.create();
}

nest::iaf_psc_exp_ps_time_reversal::iaf_psc_exp_ps_time_reversal(const iaf_psc_exp_ps_time_reversal & n)
  : Node(n),
    P_(n.P_),
    S_(n.S_),
    B_(n.B_, *this)
{}

/* ---------------------------------------------------------------- 
 * Node initialization functions
 * ---------------------------------------------------------------- 

void nest::iaf_psc_exp_ps_time_reversal::init_node_(const Node & proto)
{
  const iaf_psc_exp_ps_time_reversal & pr = downcast<iaf_psc_exp_ps_time_reversal>(proto);

  P_ = pr.P_;
  S_ = pr.S_;
}*/

void nest::iaf_psc_exp_ps_time_reversal::init_state_(const Node & proto)
{
  const iaf_psc_exp_ps_time_reversal & pr = downcast<iaf_psc_exp_ps_time_reversal>(proto);

  S_ = pr.S_;
}

void nest::iaf_psc_exp_ps_time_reversal::init_buffers_()
{
  B_.events_.resize();
  B_.events_.clear(); 
  B_.currents_.clear();  // includes resize
  B_.logger_.reset();
}

void nest::iaf_psc_exp_ps_time_reversal::calibrate()
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
  assert( V_.refractory_steps_ >= 0 );  // since t_ref_ >= 0, this can only fail in error
}

/* ---------------------------------------------------------------- 
 * Update and spike handling functions
 * ---------------------------------------------------------------- */

void nest::iaf_psc_exp_ps_time_reversal::update(const Time & origin, 
					 const long from, const long to)
{
  assert ( to >= 0 );
  assert( static_cast< delay >( from )
    < kernel().connection_manager.get_min_delay() );
  assert( from < to );

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

  for ( long lag = from; lag < to; ++lag )
  {
    // time at start of update step
    const long T = origin.get_steps() + lag;
    
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
      //  the state space test takes argument dt and
      //  returns true, spike: if (V(t_{right}) > V_(\theta));
      //  returns false: ( (V(t_{right} < V_(\theta) or initial conditions in no-spike region);
      //  returns true, spike: missed spike excursion, compute t_{max} = dt and find point of 
      // threshold crossing t_{\theta} using emit_spike_.
      V_.bisection_step = V_.h_ms_;

      if (is_spike_(V_.h_ms_))
        emit_spike_(origin, lag, 0, V_.bisection_step);
	
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

  V_.bisection_step = ministep;

 
	if (is_spike_(ministep))
	{
	  emit_spike_(origin, lag, V_.h_ms_-last_offset, V_.bisection_step);
	}


    
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


        V_.bisection_step = last_offset;
	      propagate_(last_offset);

	      if (is_spike_(last_offset))	 
          	emit_spike_(origin, lag, V_.h_ms_-last_offset, V_.bisection_step);


	
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
void nest::iaf_psc_exp_ps_time_reversal::handle(SpikeEvent & e)
{
  assert( e.get_delay() > 0 );

  /* We need to compute the absolute time stamp of the delivery time
     of the spike, since spikes might spend longer than min_delay_
     in the queue.  The time is computed according to Time Memo, Rule 3.
  */
  const long Tdeliver = e.get_stamp().get_steps() + e.get_delay() - 1;

  B_.events_.add_spike(e.get_rel_delivery_steps(nest::kernel().simulation_manager.get_slice_origin()), 
		       Tdeliver, e.get_offset(), e.get_weight() * e.get_multiplicity());
}

void nest::iaf_psc_exp_ps_time_reversal::handle(CurrentEvent & e)
{
  assert( e.get_delay() > 0 );

  const double_t c = e.get_current();
  const double_t w = e.get_weight();

  // add weighted current; HEP 2002-10-04
  B_.currents_.add_value(e.get_rel_delivery_steps(nest::kernel().simulation_manager.get_slice_origin() ), w * c);
}

void nest::iaf_psc_exp_ps_time_reversal::handle(DataLoggingRequest &e)
{
  B_.logger_.handle(e);
}

// auxiliary functions ---------------------------------------------

inline 
void nest::iaf_psc_exp_ps_time_reversal::set_spiketime(const Time & now)
{
  S_.last_spike_step_ = now.get_steps();
}

void nest::iaf_psc_exp_ps_time_reversal::propagate_(const double_t dt)
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

void nest::iaf_psc_exp_ps_time_reversal::emit_spike_(const Time & origin, const long lag, const double_t t0,  const double_t dt)
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
  kernel().event_delivery_manager.send(*this, se, lag);
}

void nest::iaf_psc_exp_ps_time_reversal::emit_instant_spike_(const Time & origin, const long lag,
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
  kernel().event_delivery_manager.send(*this, se, lag);
}

inline double nest::iaf_psc_exp_ps_time_reversal::bisectioning_(const double dt) const
{
 
  double_t root = 0.0;

  double_t y2_root = V_.y2_before_;

  double_t div = 2.0;
  while ( fabs(P_.U_th_-y2_root) > 1e-14 and (dt/div > 0.0) )
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

void nest::iaf_psc_exp_ps_time_reversal::spike_test_count_(const double_t t1)
{
  S_.c0++; // V(t1) < V_th

  // we assume that P_.tau_ex_=P_.tau_in_
  double_t const I_0   = V_.y1_ex_before_ + V_.y1_in_before_;
  double_t const V_0   = V_.y2_before_;
  double_t const I_t1  = S_.y1_ex_ + S_.y1_in_;
  double_t const V_t1  = S_.y2_;
  double_t const tau   = P_.tau_ex_;
  double_t const tau_m = P_.tau_m_;
  double_t const I_x   = P_.I_e_;
  double_t const C_m   = P_.c_m_;
  double_t const V_th  = P_.U_th_;

  double_t const tauC_m = tau_m/C_m;

  double_t const Vdot_0  = -V_0/tau_m + (I_0+I_x)/C_m;
  double_t const Vdot_t1 = -V_t1/tau_m + (I_t1+I_x)/C_m;

  // iaflossless tests
  if ( Vdot_t1 < 0.0 )
    S_.c1b++;
  if ( Vdot_0 > 0.0 )
  {
    S_.c1a++;
    if ( Vdot_t1 < 0.0 )
    {
      S_.c2++;

      if ( Vdot_0*t1 + V_0 >= V_th )
	S_.c3a++;
      if ( V_0 + Vdot_0 * (V_0 - V_t1 + Vdot_t1*t1) / (Vdot_t1-Vdot_0) >= V_th )
	S_.c3b++;

      double_t const expm1_tau_syn = numerics::expm1(t1/tau);   // positive exponent!
      double_t const expm1_tau_m   = numerics::expm1(t1/tau_m); // positive exponent!

      double_t const V_0_bar     = V_0 - tauC_m*I_x;
      //double_t const V_t1_bar    = V_t1 - tauC_m*I_x;
      double_t const V_th_bar    = V_th - tauC_m*I_x;
      double_t const V_right_bar = (tau_m*expm1_tau_m - tau*expm1_tau_syn) * V_th_bar / (tau_m-tau);
      double_t const I_left      = V_th_bar/tauC_m;
      double_t const m           = (V_right_bar - V_th_bar) / (expm1_tau_syn*I_left); // V_left_bar = V_th_bar

      if ( V_0_bar >= m * (I_0 - I_left) + V_th )
      {
	S_.c4++;
	  
	double_t const y = V_th_bar/tauC_m / I_0;

	if ( V_0 >= tau_m/(tau_m-tau) * (-tau/C_m * I_0 + V_th_bar * pow(y, (-tau/tau_m))) )
	  S_.det_spikes++;
      }
    }
  }

  // D'Haene tests
  double_t const minus_taus = -tau_m*tau / (tau_m-tau);
  double_t const V_syn      = minus_taus / C_m * I_0;
  double_t const V_m        = V_0 - tauC_m*I_x - V_syn;

  if ( V_m > 0.0 && V_syn < 0.0 )
  {
    S_.dhaene_quick1++;

    double_t const quot = -tau*V_m / (tau_m*V_syn);

    if ( quot <= 1.0 )
    {
      S_.dhaene_quick2++;

      double_t const t_max = minus_taus * log(quot);

      if ( t_max < t1 )
	S_.dhaene_tmax_lt_t1++;
      
      double_t const expm1_tau_syn = numerics::expm1(-t_max/tau);
      double_t const expm1_tau_m   = numerics::expm1(-t_max/tau_m);
      
      double_t const P20 = -tau_m*expm1_tau_m / C_m;
      double_t const P21 = minus_taus / C_m * (expm1_tau_syn-expm1_tau_m);
      
      if ( (P20*I_x + P21*I_0 + expm1_tau_m*V_0 + V_0) >= V_th )
      {
	S_.dhaene_max++;
        if ( t_max <= t1 )
	  S_.dhaene_det_spikes++;
      }
    }
  }
}

