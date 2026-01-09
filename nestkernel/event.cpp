/*
 *  event.cpp
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

#include "event.h"

// Includes from nestkernel:
#include "connection_manager.h"
#include "kernel_manager.h"
#include "node.h"

namespace nest
{
Event::Event()
  : sender_node_id_( 0 ) // initializing to 0 as this is an unsigned type
                         // node ID 0 is network, can never send an event, so
                         // this is safe
  , sender_spike_data_()
  , sender_( nullptr )
  , receiver_( nullptr )
  , p_( -1 )
  , rp_( 0 )
  , d_( 1 )
  , stamp_( Time::step( 0 ) )
  , stamp_steps_( 0 )
  , offset_( 0.0 )
  , w_( 0.0 )
{
}

size_t
Event::retrieve_sender_node_id_from_source_table() const
{
  if ( sender_node_id_ > 0 )
  {
    return sender_node_id_;
  }
  else
  {
    const size_t node_id = kernel::manager< ConnectionManager >.get_source_node_id(
      sender_spike_data_.get_tid(), sender_spike_data_.get_syn_id(), sender_spike_data_.get_lcid() );
    return node_id;
  }
}

size_t
Event::get_receiver_node_id() const
{
  return receiver_->get_node_id();
}

void
SpikeEvent::operator()()
{
  receiver_->handle( *this );
}

void
WeightRecorderEvent::operator()()
{
  receiver_->handle( *this );
}

void
DSSpikeEvent::operator()()
{
  sender_->event_hook( *this );
}

void
RateEvent::operator()()
{
  receiver_->handle( *this );
}

void
CurrentEvent::operator()()
{
  receiver_->handle( *this );
}

void
DSCurrentEvent::operator()()
{
  sender_->event_hook( *this );
}

void
ConductanceEvent::operator()()
{
  receiver_->handle( *this );
}

void
DoubleDataEvent::operator()()
{
  receiver_->handle( *this );
}

void
DataLoggingRequest::operator()()
{
  receiver_->handle( *this );
}

void
DataLoggingReply::operator()()
{
  receiver_->handle( *this );
}

void
GapJunctionEvent::operator()()
{
  receiver_->handle( *this );
}

void
InstantaneousRateConnectionEvent::operator()()
{
  receiver_->handle( *this );
}

void
DelayedRateConnectionEvent::operator()()
{
  receiver_->handle( *this );
}

void
DiffusionConnectionEvent::operator()()
{
  receiver_->handle( *this );
}

void
LearningSignalConnectionEvent::operator()()
{
  receiver_->handle( *this );
}

void
SICEvent::operator()()
{
  receiver_->handle( *this );
}

void
Event::set_rport( size_t rp )
{

  rp_ = rp;
}

void
Event::set_port( size_t p )
{

  p_ = p;
}

size_t
Event::get_rport() const
{

  return rp_;
}

size_t
Event::get_port() const
{

  return p_;
}

void
Event::set_offset( double t )
{

  offset_ = t;
}

double
Event::get_offset() const
{

  return offset_;
}

void
Event::set_delay_steps( long d )
{

  d_ = d;
}

long
Event::get_rel_delivery_steps( const Time& t ) const
{

  if ( stamp_steps_ == 0 )
  {
    stamp_steps_ = stamp_.get_steps();
  }
  return stamp_steps_ + d_ - 1 - t.get_steps();
}

long
Event::get_delay_steps() const
{

  return d_;
}

void
Event::set_stamp( Time const& s )
{

  stamp_ = s;
  stamp_steps_ = 0; // setting stamp_steps to zero indicates
                    // stamp_steps needs to be recalculated from
                    // stamp_ next time it is needed (e.g., in
                    // get_rel_delivery_steps)
}

Time const&
Event::get_stamp() const
{

  return stamp_;
}

void
Event::set_weight( double w )
{

  w_ = w;
}

double
Event::get_weight() const
{

  return w_;
}

size_t
Event::get_sender_node_id() const
{

  assert( sender_node_id_ > 0 );
  return sender_node_id_;
}

Node&
Event::get_sender() const
{

  assert( sender_ );
  return *sender_;
}

Node&
Event::get_receiver() const
{

  assert( receiver_ );
  return *receiver_;
}

void
Event::set_sender_node_id_info( const size_t tid, const synindex syn_id, const size_t lcid )
{

  // lag and offset of SpikeData are not used here
  sender_spike_data_.set( tid, syn_id, lcid, 0, 0.0 );
}

void
Event::set_sender_node_id( const size_t node_id )
{

  sender_node_id_ = node_id;
}

void
Event::set_sender( Node& s )
{

  sender_ = &s;
}

void
Event::set_receiver( Node& r )
{

  receiver_ = &r;
}

bool
Event::is_valid() const
{

  return ( sender_is_valid() and receiver_is_valid() and d_ > 0 );
}

bool
Event::receiver_is_valid() const
{

  return receiver_;
}

bool
Event::sender_is_valid() const
{

  return sender_;
}

DoubleDataEvent*
DoubleDataEvent::clone() const
{

  return new DoubleDataEvent( *this );
}


double
ConductanceEvent::get_conductance() const
{

  return g_;
}

void
ConductanceEvent::set_conductance( double g )
{

  g_ = g;
}

ConductanceEvent*
ConductanceEvent::clone() const
{

  return new ConductanceEvent( *this );
}


const std::vector< Name >&
DataLoggingRequest::record_from() const
{

  // During simulation, events are created without recordables
  // information. On these, record_from() must not be called.
  assert( record_from_ );

  return *record_from_;
}

const Time&
DataLoggingRequest::get_recording_offset() const
{

  assert( recording_offset_.is_finite() );
  return recording_offset_;
}

const Time&
DataLoggingRequest::get_recording_interval() const
{

  // During simulation, events are created without recording interval
  // information. On these, get_recording_interval() must not be called.
  assert( recording_interval_.is_finite() );

  return recording_interval_;
}

DataLoggingRequest*
DataLoggingRequest::clone() const
{

  return new DataLoggingRequest( *this );
}

DataLoggingRequest::DataLoggingRequest( const Time& rec_int, const Time& rec_offset, const std::vector< Name >& recs )
  : Event()
  , recording_interval_( rec_int )
  , recording_offset_( rec_offset )
  , record_from_( &recs )
{
}

DataLoggingRequest::DataLoggingRequest( const Time& rec_int, const std::vector< Name >& recs )
  : Event()
  , recording_interval_( rec_int )
  , record_from_( &recs )
{
}

DataLoggingRequest::DataLoggingRequest()
  : Event()
  , recording_interval_( Time::neg_inf() )
  , recording_offset_( Time::ms( 0. ) )
  , record_from_( nullptr )
{
}

double
CurrentEvent::get_current() const
{

  return c_;
}

void
CurrentEvent::set_current( double c )
{

  c_ = c;
}

CurrentEvent*
CurrentEvent::clone() const
{

  return new CurrentEvent( *this );
}

double
RateEvent::get_rate() const
{

  return r_;
}

void
RateEvent::set_rate( double r )
{

  r_ = r;
}

RateEvent*
RateEvent::clone() const
{

  return new RateEvent( *this );
}

size_t
WeightRecorderEvent::get_receiver_node_id() const
{

  return receiver_node_id_;
}

void
WeightRecorderEvent::set_receiver_node_id( size_t node_id )
{

  receiver_node_id_ = node_id;
}

WeightRecorderEvent*
WeightRecorderEvent::clone() const
{

  return new WeightRecorderEvent( *this );
}

WeightRecorderEvent::WeightRecorderEvent()
  : receiver_node_id_( 0 )
{
}

size_t
SpikeEvent::get_multiplicity() const
{

  return multiplicity_;
}

void
SpikeEvent::set_multiplicity( size_t multiplicity )
{

  multiplicity_ = multiplicity;
}

SpikeEvent*
SpikeEvent::clone() const
{

  return new SpikeEvent( *this );
}

SpikeEvent::SpikeEvent()
  : multiplicity_( 1 )
{
}

DataLoggingReply*
DataLoggingReply::clone() const
{

  assert( false );
  return nullptr;
}


const DataLoggingReply::Container&
DataLoggingReply::get_info() const
{

  return info_;
}


void
Event::set_drift_factor( double )
{
}

Event::~Event()
{
}

DataLoggingReply::DataLoggingReply( const DataLoggingReply::Container& info )
  : info_( info )
{
}

} // namespace nest
