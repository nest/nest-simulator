/*
 *  stimulating_device.h
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

#ifndef STIMULATING_DEVICE_H
#define STIMULATING_DEVICE_H

// Includes from nestkernel:
#include "node.h"
#include "device.h"
#include "device_node.h"
#include "stimulating_backend.h"
#include "nest_types.h"
#include "kernel_manager.h"

// Includes from sli:
#include "dictutils.h"

// Includes from libnestutil:
#include "compose.hpp"

#include <string>

class SpikeEvent;
class CurrentEvent;
class EmittedEvent;
class DoubleDataEvent;
class DelayedRateConnectionEvent;
//class IOManager;

namespace nest
{

/**
 * Base class for common properties of Stimulating Devices.
 *
 * Stimulating devices are all devices injecting currents, spike trains
 * or other signals into a network. They provide only output and do not
 * receive any input.
 *
 * Stimulating devices come in (at least) two varieties: those providing
 * analog signals (CurrentEvent) and thos providing spike trains (SpikeEvent).
 * Device activation needs to be handled differently in both cases. The general
 * principle is that of the left-open, right-closed interval (start, stop].
 * For devices emitting spikes, spikes with times in that interval will be
 * emitted.
 *
 * For analog stimuli, e.g., currents, a stimulus is present in the
 * interval (t, t+h], where h is the simulation resolution, if the effect
 * of the stimulus can be observed at t+h. Thus, if a stimulus is to be
 * active from time a, its effect will first be observable at a+h. This
 * requires that the Event communicating the stimulus must be delivered
 * at time a, i.e., by the deliver_events() call prior to the update for
 * (a, a+h].
 *
 * Since stimulating devices are connected to their targets with a delay of one
 * time step, this means that analog stimulating devices need to emit the event
 * during the update step for the interval (a-h, a]. Thus, the device needs
 * to be PRO-ACTIVE.
 *
 * Further, activity of stimulating devices is determined on the basis of
 * simulation time, not event time stamps. This means that the first simulation
 * time step during which the device must emit events is the step for which
 * the global clock has time a-h. If stimulation is to end by time b, this means
 * that the last event should be emitted during the time step for which the
 * global clock has time b-2h.
 *
 * @note Any stimulating devices transmitting analog signals must NOT HAVE
 * PROXIES.
 *
 * @note The distinction between analog and spike emitting devices is
 *       implemented by making StimulatingDevice a template class with the type
 *       of the Event sent as template parameter. Member is_active() is not
 *       implemented in general and is available only for those cases for which
 *       it is explicitly specialized.
 *
 * @note StimulatingDevice inherits protected from Device, so that
 *       implementations of is_active() can access t_min and t_max.
 *
 * @todo The timing of analog devices is correct only if they are transmitted
 *       using Network::send_local(), but we cannot enforce this currently.
 *
 * @ingroup Devices
 */
template < typename EmittedEvent >
class StimulatingDevice : public DeviceNode, public Device
{
public:
  StimulatingDevice();
  StimulatingDevice( StimulatingDevice< EmittedEvent > const& );
  virtual ~StimulatingDevice()
  {
  }

  /**
   * Determine whether device is active.
   * The argument is the value of the simulation time.
   * @see class comment for details.
   */
  bool is_active( const Time& ) const;
  void get_status( DictionaryDatum& d ) const;
  void set_status( const DictionaryDatum& ) const;
 
  using Device::init_state; 
  using Device::calibrate;
  using Device::init_buffers;
  using Node::calibrate;


  void calibrate( const std::vector< Name >&, const std::vector< Name >& );
  void calibrate();


  //! Throws IllegalConnection if synapse id differs from initial synapse id
  void enforce_single_syn_type( synindex );
  
  /**
   * Device type.
   */
  enum Type
  {
    STEP_CURRENT_GENERATOR,
    SPIKE_GENERATOR,
    UNSPECIFIED
  };

  Type get_type() const;
  const std::string& get_label() const;
  void update_from_backend( std::vector< double > input_spikes );
  void update( Time const&, const long, const long );
  void set_status( const DictionaryDatum& );

protected:
    void set_initialized_();
    
  struct Parameters_
  {
    std::string label_;  //!< A user-defined label for symbolic device names.
    bool time_in_steps_; //!< Flag indicating if time is recorded in steps or ms.
    Name input_from_;    //!< Array of input backends to use.

    Parameters_();
    Parameters_( const Parameters_& );
    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum& ) const;
  } P_;

  struct State_
  {
    size_t n_events_;

    State_();
    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum& ) const;
  } S_;

private:
  /**
   * Synapse type of the first outgoing connection made by the Device.
   *
   * Used to check that devices connect using only a single synapse type,
   * see #481 and #737. Since this value must survive resets, it is
   * stored here, even though it is an implementation detail.
   */
  synindex first_syn_id_;
  
  DictionaryDatum backend_params_;
};
} // namespace nest

template < typename EmittedEvent >
nest::StimulatingDevice< EmittedEvent >::StimulatingDevice()
  : Device()
  , DeviceNode()
  , first_syn_id_( invalid_synindex )
  , backend_params_( new Dictionary )
{
}

template < typename EmittedEvent >
nest::StimulatingDevice< EmittedEvent >::StimulatingDevice( StimulatingDevice< EmittedEvent > const& sd )
  : Device( sd )
  , DeviceNode( sd )
  , first_syn_id_( invalid_synindex ) // a new instance can have no connections
  , backend_params_( new Dictionary )
{
}

template < typename EmittedEvent >
typename nest::StimulatingDevice< EmittedEvent >::Type
nest::StimulatingDevice< EmittedEvent >::get_type() const
{
    return StimulatingDevice< EmittedEvent >::Type::UNSPECIFIED;
}

template < typename EmittedEvent >
void
nest::StimulatingDevice< EmittedEvent >::update_from_backend( std::vector< double > input_spikes )
{
}

template < typename EmittedEvent >
void
nest::StimulatingDevice< EmittedEvent >::calibrate()
{
}

template < typename EmittedEvent >
void
nest::StimulatingDevice< EmittedEvent >::update( Time const&, const long, const long )
{
}

template < typename EmittedEvent >
void
nest::StimulatingDevice< EmittedEvent >::set_status( const DictionaryDatum& )
{
}

template < typename EmittedEvent >
void
nest::StimulatingDevice< EmittedEvent >::set_initialized_()
{
  kernel().io_manager.enroll_input( P_.input_from_, *this, backend_params_ );
}

template < typename EmittedEvent >
void
nest::StimulatingDevice< EmittedEvent >::calibrate( const std::vector< Name >& double_value_names,
  const std::vector< Name >& long_value_names )
{
  Device::calibrate();
  kernel().io_manager.set_input_value_names( P_.input_from_, *this, double_value_names, long_value_names );
}

template < typename EmittedEvent >
const std::string&
nest::StimulatingDevice< EmittedEvent >::get_label() const
{
  return P_.label_;
}

namespace nest{

// specializations must be declared inside namespace
template <>
inline bool
StimulatingDevice< nest::CurrentEvent >::is_active( const Time& T ) const
{
  /* We have t_min_ = origin_ + start_, t_max_ = origin_ + stop_ in steps.
     We need to check if
        t_min_ - 1 <= T.get_steps() <= t_max_ - 2
     This is equivalent to checking
        t_min_ <= T.get_steps() + 1 < t_max_
   */
  const long step = T.get_steps() + 1;
  return get_t_min_() <= step and step < get_t_max_();
}

template <>
inline bool
StimulatingDevice< nest::DelayedRateConnectionEvent >::is_active( const Time& T ) const
{
  // same as for the CurrentEvent
  const long step = T.get_steps() + 1;
  return get_t_min_() <= step and step < get_t_max_();
}

template <>
inline bool
StimulatingDevice< nest::DoubleDataEvent >::is_active( const Time& T ) const
{
  // same as for the CurrentEvent
  const long step = T.get_steps() + 1;
  return get_t_min_() <= step and step < get_t_max_();
}

template <>
inline bool
StimulatingDevice< nest::SpikeEvent >::is_active( const Time& T ) const
{
  /* Input is the time stamp of the spike to be emitted. */
  const long stamp = T.get_steps();
  return get_t_min_() < stamp and stamp <= get_t_max_();
}

}

template < typename EmittedEvent >
inline void
nest::StimulatingDevice< EmittedEvent >::enforce_single_syn_type( synindex syn_id )
{
  if ( first_syn_id_ == invalid_synindex )
  {
    first_syn_id_ = syn_id;
  }
  if ( syn_id != first_syn_id_ )
  {
    throw IllegalConnection(
      "All outgoing connections from a device must use the same synapse "
      "type." );
  }
}

template < typename EmittedEvent >
nest::StimulatingDevice< EmittedEvent >::Parameters_::Parameters_()
  : label_()
  , time_in_steps_( false )
  , input_from_( names::internal )
{
}

template < typename EmittedEvent >
nest::StimulatingDevice< EmittedEvent >::Parameters_::Parameters_( const Parameters_& p )
  : label_( p.label_ )
  , time_in_steps_( p.time_in_steps_ )
  , input_from_( p.input_from_ )
{
}

template < typename EmittedEvent >
void
nest::StimulatingDevice< EmittedEvent >::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::label ] = label_;
  ( *d )[ names::time_in_steps ] = time_in_steps_;
  ( *d )[ names::input_from ] = LiteralDatum( input_from_ );
}

template < typename EmittedEvent >
void
nest::StimulatingDevice< EmittedEvent >::Parameters_::set( const DictionaryDatum& d ) const
{
  updateValue< std::string >( d, names::label, label_ );

  bool time_in_steps = time_in_steps_;
  updateValue< bool >( d, names::time_in_steps, time_in_steps );

  if ( time_in_steps != time_in_steps_ )
  {
    throw BadProperty(
      "Property /time_in_steps cannot be set if recordings exist. "
      "Please clear the events first by setting /n_events to 0." );
  }
  time_in_steps_ = time_in_steps;
  std::string input_from;
  if ( updateValue< std::string >( d, names::input_from, input_from ) )
  {
    
    if ( not kernel().io_manager.is_valid_input_backend( input_from ) )
    {
      std::string msg = String::compose( "Unknown input backend '%1'", input_from );
      throw BadProperty( msg );
    }
    input_from_ = input_from;
  }
}

template < typename EmittedEvent >
nest::StimulatingDevice< EmittedEvent >::State_::State_()
  : n_events_( 0 )
{
}

template < typename EmittedEvent >
void
nest::StimulatingDevice< EmittedEvent >::State_::get( DictionaryDatum& d ) const
{
  // if we already have the n_events entry, we add to it, otherwise we create it
  if ( d->known( names::n_events ) )
  {
    long n_events = getValue< long >( d, names::n_events );
    ( *d )[ names::n_events ] = n_events + n_events_;
  }
  else
  {
    ( *d )[ names::n_events ] = n_events_;
  }
}

template < typename EmittedEvent >
void
nest::StimulatingDevice< EmittedEvent >::State_::set( const DictionaryDatum& d ) const
{
  size_t n_events = n_events_;
  if ( updateValue< long >( d, names::n_events, n_events ) )
  {
    if ( n_events == 0 )
    {
      n_events_ = n_events;
    }
    else
    {
      throw BadProperty(
        "Property /n_events can only be set "
        "to 0 (which clears all stored events)." );
    }
  }
}

template < typename EmittedEvent >
void
nest::StimulatingDevice< EmittedEvent >::set_status( const DictionaryDatum& d ) const
{
  
  if ( kernel().simulation_manager.has_been_prepared() )
  {
    throw BadProperty( "Input parameters cannot be changed while inside a Prepare/Run/Cleanup context." );
  }

  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d );         // throws if BadProperty

  State_ stmp = S_; // temporary copy in case of errors
  stmp.set( d );    // throws if BadProperty

  Device::set_status( d );

  if ( get_node_id() == 0 ) // this is a model prototype, not an actual instance
  {
    DictionaryDatum backend_params = DictionaryDatum( new Dictionary );

    // copy all properties not previously accessed from d to backend_params
    for ( auto& kv_pair : *d )
    {
      if ( not kv_pair.second.accessed() )
      {
        ( *backend_params )[ kv_pair.first ] = kv_pair.second;
      }
    }

    kernel().io_manager.check_input_backend_device_status( ptmp.input_from_, backend_params );

    // cache all properties accessed by the backend in private member
    backend_params_->clear();
    for ( auto& kv_pair : *backend_params )
    {
      if ( kv_pair.second.accessed() )
      {
        ( *backend_params_ )[ kv_pair.first ] = kv_pair.second;
        d->lookup( kv_pair.first ).set_access_flag();
      }
    }
  }
  else
  {
    kernel().io_manager.enroll_input( ptmp.input_from_, *this, d );
  }

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}


template < typename EmittedEvent >
void
nest::StimulatingDevice< EmittedEvent >::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );

  Device::get_status( d );

  ( *d )[ names::element_type ] = LiteralDatum( names::stimulator );

  if ( get_node_id() == 0 ) // this is a model prototype, not an actual instance
  {
    // first get the defaults from the backend
    if ( not P_.input_from_.toString().compare(names::internal.toString()) == 0 ){
        kernel().io_manager.get_stimulating_backend_device_defaults( P_.input_from_, d );
    }

    // then overwrite with cached parameters
    for ( auto& kv_pair : *backend_params_ )
    {
      ( *d )[ kv_pair.first ] = kv_pair.second;
    }
  }
  else
  {
    if ( not P_.input_from_.toString().compare(names::internal.toString()) == 0 ){
        kernel().io_manager.get_stimulating_backend_device_status( P_.input_from_, *this, d );
    }
  }
}


#endif
