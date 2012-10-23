/*
 *  ginzburg.cpp
 *
 *  This file is part of NEST
 *
 *  Copyright (C) 2004 by
 *  The NEST Initiative 
 *
 *  See the file AUTHORS for details.
 *
 *  Permission is granted to compile and modify
 *  this file for non-commercial use.
 *  See the file LICENSE for details.
 *
 */

/* ginzburg is a neuron where the potential jumps on each spike arrival. */

#include "exceptions.h"
#include "ginzburg_neuron.h"
#include "network.h"
#include "dict.h"
#include "integerdatum.h"  
#include "doubledatum.h"
#include "dictutils.h"
#include "numerics.h"
#include "universal_data_logger_impl.h"

#include <limits>

nest::RecordablesMap<nest::ginzburg> nest::ginzburg::recordablesMap_;

namespace nest
{

  /*
   * Override the create() method with one call to RecordablesMap::insert_() 
   * for each quantity to be recorded.
   */
  template <>
  void RecordablesMap<ginzburg>::create()
  {
    // use standard names whereever you can for consistency!
    insert_(names::S, &ginzburg::get_output_state__);
    insert_(names::h, &ginzburg::get_input__);
  }

/* ---------------------------------------------------------------- 
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

ginzburg::Parameters_::Parameters_()
  : tau_m_ ( 10.0 ),  // ms
    theta_ ( 0.0 ),   // mV
    c1_    ( 0.0 ),   // (mV)^-1
    c2_    ( 1.0 ),   // dimensionless
    c3_    ( 1.0 )    // (mV)^-1
{
  recordablesMap_.create();
}

ginzburg::State_::State_()
  : y_(false),
    h_   (0.0),
    last_in_gid_(0),
    t_next_(Time::neg_inf()),          // mark as not initialized
    t_last_in_spike_(Time::neg_inf())  // mark as not intialized
{}

/* ---------------------------------------------------------------- 
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void ginzburg::Parameters_::get(DictionaryDatum &d) const
{
  def<double>(d, names::tau_m, tau_m_);
  def<double>(d, names::theta, theta_);
  def<double>(d, names::c_1, c1_);
  def<double>(d, names::c_2, c2_);
  def<double>(d, names::c_3, c3_);
}

void ginzburg::Parameters_::set(const DictionaryDatum& d)
{
  updateValue<double>(d, names::tau_m, tau_m_);
  updateValue<double>(d, names::theta, theta_);
  updateValue<double>(d, names::c_1, c1_);
  updateValue<double>(d, names::c_2, c2_);
  updateValue<double>(d, names::c_3, c3_);
    
  if ( tau_m_ <= 0 )
    throw BadProperty("All time constants must be strictly positive.");
}

void ginzburg::State_::get(DictionaryDatum &d, const Parameters_&) const
{
  def<double>(d, names::h, h_); // summed input
  def<double>(d, names::S, y_); // binary output state
}

void ginzburg::State_::set(const DictionaryDatum&, const Parameters_&)
{
}

ginzburg::Buffers_::Buffers_(ginzburg& n)
  : logger_(n)
{}

ginzburg::Buffers_::Buffers_(const Buffers_&, ginzburg& n)
  : logger_(n)
{}


/* ---------------------------------------------------------------- 
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

ginzburg::ginzburg()
  : Archiving_Node(), 
    P_(), 
    S_(),
    B_(*this)
{}

ginzburg::ginzburg(const ginzburg& n)
  : Archiving_Node(n), 
    P_(n.P_), 
    S_(n.S_),
    B_(*this)
{}

/* ---------------------------------------------------------------- 
 * Node initialization functions
 * ---------------------------------------------------------------- */

void ginzburg::init_state_(const Node& proto)
{
  const ginzburg& pr = downcast<ginzburg>(proto);
  S_ = pr.S_;
}

void ginzburg::init_buffers_()
{
  B_.spikes_.clear();       // includes resize
  B_.logger_.reset();
  Archiving_Node::clear_history();
}

void ginzburg::calibrate()
{
  B_.logger_.init();  // ensures initialization in case mm connected after Simulate
  V_.rng_ = net_->get_rng(get_thread());

  // draw next time of update for the neuron from exponential distribution
  // only if not yet initialized
  if ( S_.t_next_.is_neg_inf() )
    S_.t_next_ = Time::ms ( V_.exp_dev_(V_.rng_) * P_.tau_m_ );
}


inline
double_t ginzburg::gain_(double_t h)
{
  return P_.c1_ * h + P_.c2_ * 0.5 * (1.0 + tanh( P_.c3_ * (h - P_.theta_) ));
}


/* ---------------------------------------------------------------- 
 * Update and spike handling functions
 */

void ginzburg::update(Time const & origin, 
				 const long_t from, const long_t to)
{
  assert(to >= 0 && (delay) from < Scheduler::get_min_delay());
  assert(from < to);

  for ( long_t lag = from ; lag < to ; ++lag )
  {
    // update the input current
    // the buffer for incoming spikes for every time step contains the difference
    // of the total input h with respect to the previous step, so sum them up
    S_.h_ += B_.spikes_.get_value(lag);

    // check, if the update needs to be done
    if ( Time::step(origin.get_steps()+lag) > S_.t_next_ )
    {
      // change the state of the neuron with probability given by
      // gain function
      // if the state has changed, the neuron produces an event sent to all its targets
      
      bool new_y = V_.rng_->drand() < gain_(S_.h_);

      if ( new_y != S_.y_ )
      {	
	SpikeEvent se;
	// use multiplicity 2 to signalize transition to 1 state
	// use multiplicity 1 to signalize transition to 0 state
	se.set_multiplicity(new_y ? 2 : 1);
	network()->send(*this, se, lag);
	S_.y_ = new_y;
      }
      
      // draw next update interval from exponential distribution
      S_.t_next_ += Time::ms ( V_.exp_dev_(V_.rng_) * P_.tau_m_ );

    } // of if (update now)

    // log state data
    B_.logger_.record_data(origin.get_steps() + lag);

  } // of for (lag ...

}                           
                     
void ginzburg::handle(SpikeEvent & e)
{
  assert(e.get_delay() > 0);

  // The following logic implements the encoding:
  // A single spike signals a transition to 0 state, two spikes in same time step
  // signal the transition to 1 state.
  //
  // Remember the global id of the sender of the last spike being received
  // this assumes that several spikes being sent by the same neuron in the same time step
  // are received consecutively or are conveyed by setting the multiplicity accordingly.
  
  long_t m = e.get_multiplicity();
  long_t gid = e.get_sender().get_gid();
  const Time &t_spike = e.get_stamp();

  if (m == 1)
  { //multiplicity == 1, either a single 1->0 event or the first or second of a pair of 0->1 events
    if (gid == S_.last_in_gid_ && t_spike == S_.t_last_in_spike_)
    {
      // received twice the same gid, so transition 0->1
      // take double weight to compensate for subtracting first event
      B_.spikes_.add_value(e.get_rel_delivery_steps(network()->get_slice_origin()),
			   2.0*e.get_weight());   
    }
    else
    {
      // count this event negatively, assuming it comes as single event
      // transition 1->0
      B_.spikes_.add_value(e.get_rel_delivery_steps(network()->get_slice_origin()),
                    -e.get_weight());
    }
  }
  else // multiplicity != 1
    if (m == 2)
    {
      // count this event positively, transition 0->1
      B_.spikes_.add_value(e.get_rel_delivery_steps(network()->get_slice_origin()),
                    e.get_weight());
    }

  S_.last_in_gid_ = gid;
  S_.t_last_in_spike_ = t_spike;
}


void ginzburg::handle(DataLoggingRequest& e)
{
  B_.logger_.handle(e);
}

 
} // namespace

