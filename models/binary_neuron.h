/*
 *  binary_neuron.h
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

#ifndef BINARY_NEURON_H
#define BINARY_NEURON_H

// C++ includes:
#include <cmath>
#include <limits>

// Includes from libnestutil:
#include "dict_util.h"
#include "numerics.h"

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "event_delivery_manager_impl.h"
#include "exceptions.h"
#include "kernel_manager.h"
#include "nest_timeconverter.h"
#include "nest_types.h"
#include "random_generators.h"
#include "recordables_map.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"

namespace nest
{
/**
 * Binary stochastic neuron with linear or sigmoidal gain function.
 *
 * This class is a base class that needs to be instantiated with a gain
 * function.
 *
 * @note
 * This neuron has a special use for spike events to convey the
 * binary state of the neuron to the target. The neuron model
 * only sends a spike if a transition of its state occurs. If the
 * state makes an up-transition it sends a spike with multiplicity 2,
 * if a down-transition occurs, it sends a spike with multiplicity 1.
 * The decoding scheme relies on the feature that spikes with multiplicity
 * larger than 1 are delivered consecutively, also in a parallel setting.
 * The creation of double connections between binary neurons will
 * destroy the decoding scheme, as this effectively duplicates
 * every event. Using random connection routines it is therefore
 * advisable to set the property 'allow_multapses' to false.
 * The neuron accepts several sources of currents, e.g. from a
 * noise_generator.
 *
 * @see ginzburg_neuron, mccullogh_pitts_neuron, erfc_neuron
 */
template < class TGainfunction >
class binary_neuron : public ArchivingNode
{

public:
  binary_neuron();
  binary_neuron( const binary_neuron& );

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;
  using Node::receives_signal;
  using Node::sends_signal;

  port send_test_event( Node&, rport, synindex, bool ) override;

  void handle( SpikeEvent& ) override;
  void handle( CurrentEvent& ) override;
  void handle( DataLoggingRequest& ) override;

  port handles_test_event( SpikeEvent&, rport ) override;
  port handles_test_event( CurrentEvent&, rport ) override;
  port handles_test_event( DataLoggingRequest&, rport ) override;

  SignalType sends_signal() const override;
  SignalType receives_signal() const override;

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

  void calibrate_time( const TimeConverter& tc ) override;


private:
  void init_buffers_() override;
  void pre_run_hook() override;

  // gain function functor
  // must have an double operator(double) defined
  TGainfunction gain_;

  void update( Time const&, const long, const long ) override;

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< binary_neuron< TGainfunction > >;
  friend class UniversalDataLogger< binary_neuron< TGainfunction > >;

  // ----------------------------------------------------------------

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {
    //! mean inter-update interval in ms (acts like a membrane time constant).
    double tau_m_;

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const;             //!< Store current values in dictionary
    void set( const DictionaryDatum&, Node* node ); //!< Set values from dictionary
  };

  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   */
  struct State_
  {
    bool y_;                 //!< output of neuron in [0,1]
    double h_;               //!< total input current to neuron
    double last_in_node_id_; //!< node ID of the last spike being received
    Time t_next_;            //!< time point of next update
    Time t_last_in_spike_;   //!< time point of last input spike seen

    State_(); //!< Default initialization

    void get( DictionaryDatum&, const Parameters_& ) const;
    void set( const DictionaryDatum&, const Parameters_&, Node* );
  };

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( binary_neuron& );
    Buffers_( const Buffers_&, binary_neuron& );

    /** buffers and sums up incoming spikes/currents */
    RingBuffer spikes_;
    RingBuffer currents_;


    //! Logger for all analog data
    UniversalDataLogger< binary_neuron > logger_;
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {
    RngPtr rng_;                        //!< random number generator of my own thread
    exponential_distribution exp_dist_; //!< random deviate generator
  };

  // Access functions for UniversalDataLogger -------------------------------

  //! Read out the binary_neuron state of the neuron
  double
  get_output_state__() const
  {
    return S_.y_;
  }

  //! Read out the summed input of the neuron (= membrane potential)
  double
  get_input__() const
  {
    return S_.h_;
  }

  // ----------------------------------------------------------------

  /**
   * @defgroup iaf_psc_alpha_data
   * Instances of private data structures for the different types
   * of data pertaining to the model.
   * @note The order of definitions is important for speed.
   * @{
   */
  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;
  /** @} */

  //! Mapping of recordables names to access functions
  static RecordablesMap< binary_neuron< TGainfunction > > recordablesMap_;
};


template < class TGainfunction >
inline port
binary_neuron< TGainfunction >::send_test_event( Node& target, rport receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}

template < class TGainfunction >
inline port
binary_neuron< TGainfunction >::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

template < class TGainfunction >
inline port
binary_neuron< TGainfunction >::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

template < class TGainfunction >
inline port
binary_neuron< TGainfunction >::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}


template < class TGainfunction >
inline SignalType
binary_neuron< TGainfunction >::sends_signal() const
{
  return BINARY;
}

template < class TGainfunction >
inline SignalType
binary_neuron< TGainfunction >::receives_signal() const
{
  return BINARY;
}


template < class TGainfunction >
inline void
binary_neuron< TGainfunction >::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  ArchivingNode::get_status( d );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();

  gain_.get( d );
}

template < class TGainfunction >
inline void
binary_neuron< TGainfunction >::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;     // temporary copy in case of errors
  ptmp.set( d, this );       // throws if BadProperty
  State_ stmp = S_;          // temporary copy in case of errors
  stmp.set( d, ptmp, this ); // throws if BadProperty

  // We now know that (ptmp, stmp) are consistent. We do not
  // write them back to (P_, S_) before we are also sure that
  // the properties to be set in the parent class are internally
  // consistent.
  ArchivingNode::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;

  gain_.set( d, this );
}

template < typename TGainfunction >
RecordablesMap< nest::binary_neuron< TGainfunction > > nest::binary_neuron< TGainfunction >::recordablesMap_;

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

template < class TGainfunction >
binary_neuron< TGainfunction >::Parameters_::Parameters_()
  : tau_m_( 10.0 ) // ms
{
  recordablesMap_.create();
}

template < class TGainfunction >
binary_neuron< TGainfunction >::State_::State_()
  : y_( false )
  , h_( 0.0 )
  , last_in_node_id_( 0 )
  , t_next_( Time::neg_inf() )          // mark as not initialized
  , t_last_in_spike_( Time::neg_inf() ) // mark as not intialized
{
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

template < class TGainfunction >
void
binary_neuron< TGainfunction >::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::tau_m, tau_m_ );
}

template < class TGainfunction >
void
binary_neuron< TGainfunction >::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  updateValueParam< double >( d, names::tau_m, tau_m_, node );
  if ( tau_m_ <= 0 )
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }
}

template < class TGainfunction >
void
binary_neuron< TGainfunction >::State_::get( DictionaryDatum& d, const Parameters_& ) const
{
  def< double >( d, names::h, h_ ); // summed input
  def< double >( d, names::S, y_ ); // binary_neuron output state
}

template < class TGainfunction >
void
binary_neuron< TGainfunction >::State_::set( const DictionaryDatum&, const Parameters_&, Node* )
{
}

template < class TGainfunction >
binary_neuron< TGainfunction >::Buffers_::Buffers_( binary_neuron& n )
  : logger_( n )
{
}

template < class TGainfunction >
binary_neuron< TGainfunction >::Buffers_::Buffers_( const Buffers_&, binary_neuron& n )
  : logger_( n )
{
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

template < class TGainfunction >
binary_neuron< TGainfunction >::binary_neuron()
  : ArchivingNode()
  , P_()
  , S_()
  , B_( *this )
{
}

template < class TGainfunction >
binary_neuron< TGainfunction >::binary_neuron( const binary_neuron& n )
  : ArchivingNode( n )
  , gain_( n.gain_ )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( *this )
{
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

template < class TGainfunction >
void
binary_neuron< TGainfunction >::init_buffers_()
{
  B_.spikes_.clear();   // includes resize
  B_.currents_.clear(); // includes resize
  B_.logger_.reset();
  ArchivingNode::clear_history();
}

template < class TGainfunction >
void
binary_neuron< TGainfunction >::pre_run_hook()
{
  // ensures initialization in case mm connected after Simulate
  B_.logger_.init();
  V_.rng_ = get_vp_specific_rng( get_thread() );

  // draw next time of update for the neuron from exponential distribution
  // only if not yet initialized
  if ( S_.t_next_.is_neg_inf() )
  {
    S_.t_next_ = Time::ms( V_.exp_dist_( V_.rng_ ) * P_.tau_m_ );
  }
}


/* ----------------------------------------------------------------
 * Update and spike handling functions
 */

template < class TGainfunction >
void
binary_neuron< TGainfunction >::update( Time const& origin, const long from, const long to )
{
  assert( to >= 0 and ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  for ( long lag = from; lag < to; ++lag )
  {
    // update the input current
    // the buffer for incoming spikes for every time step contains the
    // difference
    // of the total input h with respect to the previous step, so sum them up
    S_.h_ += B_.spikes_.get_value( lag );

    double c = B_.currents_.get_value( lag );

    // check, if the update needs to be done
    if ( Time::step( origin.get_steps() + lag ) > S_.t_next_ )
    {
      // change the state of the neuron with probability given by
      // gain function
      // if the state has changed, the neuron produces an event sent to all its
      // targets

      bool new_y = gain_( V_.rng_, S_.h_ + c );

      if ( new_y != S_.y_ )
      {
        SpikeEvent se;
        // use multiplicity 2 to signal transition to 1 state
        // use multiplicity 1 to signal transition to 0 state
        se.set_multiplicity( new_y ? 2 : 1 );
        kernel().event_delivery_manager.send( *this, se, lag );

        // As multiplicity is used only to signal internal information
        // to other binary neurons, we only set spiketime once, independent
        // of multiplicity.
        set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );
        S_.y_ = new_y;
      }

      // draw next update interval from exponential distribution
      S_.t_next_ += Time::ms( V_.exp_dist_( V_.rng_ ) * P_.tau_m_ );

    } // of if (update now)

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );

  } // of for (lag ...
}

template < class TGainfunction >
void
binary_neuron< TGainfunction >::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  // The following logic implements the encoding:
  // A single spike signals a transition to 0 state, two spikes in same time
  // step signal the transition to 1 state.
  //
  // Remember the node ID of the sender of the last spike being received
  // this assumes that several spikes being sent by the same neuron in the same
  // time step are received consecutively or are conveyed by setting the
  // multiplicity accordingly.
  //
  // Since in collocate_buffers spike events with multiplicity > 1
  // will be converted into sequences of spikes with unit multiplicity,
  // we will count the arrival of the first spike of a doublet (not yet knowing
  // it's a doublet) with a weight -1. The second part of a doublet is then
  // counted with weight 2. Since both parts of a doublet are delivered before
  // update is called, the final value in the ring buffer is guaranteed to be
  // correct.


  const long m = e.get_multiplicity();
  const long node_id = e.retrieve_sender_node_id_from_source_table();
  const Time& t_spike = e.get_stamp();

  if ( m == 1 )
  { // multiplicity == 1, either a single 1->0 event or the first or second of a
    // pair of 0->1 events
    if ( node_id == S_.last_in_node_id_ and t_spike == S_.t_last_in_spike_ )
    {
      // received twice the same node ID, so transition 0->1
      // take double weight to compensate for subtracting first event
      B_.spikes_.add_value(
        e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), 2.0 * e.get_weight() );
    }
    else
    {
      // count this event negatively, assuming it comes as single event
      // transition 1->0
      B_.spikes_.add_value(
        e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), -e.get_weight() );
    }
  }
  else if ( m == 2 )
  {
    // count this event positively, transition 0->1
    B_.spikes_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() );
  }

  S_.last_in_node_id_ = node_id;
  S_.t_last_in_spike_ = t_spike;
}

template < class TGainfunction >
void
binary_neuron< TGainfunction >::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  // we use the spike buffer to receive the binary events
  // but also to handle the incoming current events added
  // both contributions are directly added to the variable h
  B_.currents_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), w * c );
}


template < class TGainfunction >
void
binary_neuron< TGainfunction >::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

template < class TGainfunction >
void
binary_neuron< TGainfunction >::calibrate_time( const TimeConverter& tc )
{
  S_.t_next_ = tc.from_old_tics( S_.t_next_.get_tics() );
  S_.t_last_in_spike_ = tc.from_old_tics( S_.t_last_in_spike_.get_tics() );
}


} // namespace

#endif /* #ifndef BINARY_NEURON_H */
