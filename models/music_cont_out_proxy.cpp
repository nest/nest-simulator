/*
 *  music_cont_out_proxy.cpp
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

#include "music_cont_out_proxy.h"

#ifdef HAVE_MUSIC

// C++ includes:
#include <numeric>
#include <string>

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "kernel_manager.h"
#include "nest_datums.h"

// Includes from libnestutil:
#include "compose.hpp"
#include "logging.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ----------------------------------------------------------------
 */

nest::music_cont_out_proxy::Parameters_::Parameters_()
  : interval_( Time::ms( 1.0 ) )
  , port_name_( "cont_out" )
  , record_from_()
  , targets_( new NodeCollectionPrimitive() )
{
}

nest::music_cont_out_proxy::Parameters_::Parameters_( const Parameters_& p )
  : interval_( p.interval_ )
  , port_name_( p.port_name_ )
  , record_from_( p.record_from_ )
  , targets_( p.targets_ )
{
  interval_.calibrate();
}

nest::music_cont_out_proxy::State_::State_()
  : published_( false )
  , port_width_( 0 )
{
}

nest::music_cont_out_proxy::State_::State_( const State_& s )
  : published_( s.published_ )
  , port_width_( s.port_width_ )
{
}

nest::music_cont_out_proxy::Buffers_::Buffers_()
  : has_targets_( false )
  , data_()
{
}

nest::music_cont_out_proxy::Buffers_::Buffers_( const Buffers_& b )
  : has_targets_( b.has_targets_ )
  , data_( b.data_ )
{
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::music_cont_out_proxy::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::port_name ] = port_name_;
  ( *d )[ names::interval ] = interval_.get_ms();

  ArrayDatum ad_record_from;

  for ( size_t j = 0; j < record_from_.size(); ++j )
  {
    ad_record_from.push_back( LiteralDatum( record_from_[ j ] ) );
  }

  ( *d )[ names::record_from ] = ad_record_from;
  ( *d )[ names::targets ] = new NodeCollectionDatum( targets_ );
}

void
nest::music_cont_out_proxy::Parameters_::set( const DictionaryDatum& d,
  const Node& self,
  const State_& state,
  const Buffers_& buffers )
{

  if ( state.published_ == false )
  {
    updateValue< string >( d, names::port_name, port_name_ );
  }

  if ( buffers.has_targets_ && ( d->known( names::interval ) || d->known( names::record_from ) ) )
  {
    throw BadProperty(
      "The recording interval and the list of properties to record "
      "cannot be changed after the index_map has been set." );
  }

  double v;
  if ( updateValue< double >( d, names::interval, v ) )
  {
    if ( Time( Time::ms( v ) ) < Time::get_resolution() )
    {
      throw BadProperty(
        "The sampling interval must be at least as long "
        "as the simulation resolution." );
    }

    // see if we can represent interval as multiple of step
    interval_ = Time::step( Time( Time::ms( v ) ).get_steps() );
    if ( std::abs( 1 - interval_.get_ms() / v ) > 10 * std::numeric_limits< double >::epsilon() )
    {
      throw BadProperty(
        "The sampling interval must be a multiple of "
        "the simulation resolution" );
    }
  }
  // extract data
  if ( d->known( names::record_from ) )
  {
    record_from_.clear();

    ArrayDatum ad = getValue< ArrayDatum >( d, names::record_from );
    for ( Token* t = ad.begin(); t != ad.end(); ++t )
    {
      record_from_.push_back( Name( getValue< std::string >( *t ) ) );
    }
  }

  if ( d->known( names::targets ) )
  {
    if ( record_from_.empty() )
    {
      throw BadProperty( "The property record_from must be set before passing targets." );
    }

    if ( state.published_ == false )
    {
      targets_ = getValue< NodeCollectionDatum >( d, names::targets );
    }
    else
    {
      throw MUSICPortAlreadyPublished( self.get_name(), port_name_ );
    }
  }
}

void
nest::music_cont_out_proxy::State_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::published ] = published_;
  ( *d )[ names::port_width ] = port_width_;
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::music_cont_out_proxy::music_cont_out_proxy()
  : DeviceNode()
  , P_()
  , S_()
  , B_()
{
}

nest::music_cont_out_proxy::music_cont_out_proxy( const music_cont_out_proxy& n )
  : DeviceNode( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_ )
{
}

void
nest::music_cont_out_proxy::init_buffers_()
{
  B_.data_.clear();
}

void
nest::music_cont_out_proxy::finalize()
{
}

nest::port
nest::music_cont_out_proxy::send_test_event( Node& target, rport receptor_type, synindex, bool )
{

  DataLoggingRequest e( P_.interval_, P_.record_from_ );
  e.set_sender( *this );
  port p = target.handles_test_event( e, receptor_type );
  if ( p != invalid_port_ and not is_model_prototype() )
  {
    B_.has_targets_ = true;
  }

  return p;
}

void
nest::music_cont_out_proxy::calibrate()
{
  // only publish the output port once,
  if ( S_.published_ == false )
  {
    const Token synmodel = kernel().model_manager.get_synapsedict()->lookup( "static_synapse" );
    assert( synmodel.empty() == false && "synapse 'static_synapse' not available" );

    const index synmodel_id = static_cast< index >( synmodel );
    std::vector< MUSIC::GlobalIndex > music_index_map;

    DictionaryDatum dummy_params = new Dictionary();
    for ( size_t i = 0; i < P_.targets_->size(); ++i )
    {
      const index tnode_id = ( *P_.targets_ )[ i ];
      if ( kernel().node_manager.is_local_node_id( tnode_id ) )
      {
        kernel().connection_manager.connect( get_node_id(), tnode_id, dummy_params, synmodel_id );

        for ( size_t j = 0; j < P_.record_from_.size(); ++j )
        {
          music_index_map.push_back( P_.record_from_.size() * i + j );
        }
      }
    }

    MUSIC::Setup* s = kernel().music_manager.get_music_setup();
    if ( s == 0 )
    {
      throw MUSICSimulationHasRun( get_name() );
    }

    MUSIC::ContOutputPort* MP = s->publishContOutput( P_.port_name_ );

    if ( MP->isConnected() == false )
    {
      throw MUSICPortUnconnected( get_name(), P_.port_name_ );
    }

    if ( MP->hasWidth() == false )
    {
      throw MUSICPortHasNoWidth( get_name(), P_.port_name_ );
    }

    S_.port_width_ = MP->width();
    const size_t per_port_width = P_.record_from_.size();

    // Allocate memory
    B_.data_.resize( per_port_width * S_.port_width_ );

    // Check if any port is out of bounds
    if ( P_.targets_->size() > S_.port_width_ )
    {
      throw MUSICChannelUnknown( get_name(), P_.port_name_, S_.port_width_ + 1 );
    }

    // The permutation index map, contains global_index[local_index]
    MUSIC::PermutationIndex* music_perm_ind =
      new MUSIC::PermutationIndex( &music_index_map.front(), music_index_map.size() );

    MUSIC::ArrayData* dmap =
      new MUSIC::ArrayData( static_cast< void* >( &( B_.data_.front() ) ), MPI::DOUBLE, music_perm_ind );

    // Setup an array map
    MP->map( dmap );

    S_.published_ = true;

    std::string msg =
      String::compose( "Mapping MUSIC continuous output port '%1' with width=%2.", P_.port_name_, S_.port_width_ );
    LOG( M_INFO, "music_cont_out_proxy::calibrate()", msg.c_str() );
  }
}

void
nest::music_cont_out_proxy::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );

  if ( is_model_prototype() )
  {
    return; // no data to collect
  }

  // if we are the device on thread 0, also get the data from the
  // siblings on other threads
  if ( get_thread() == 0 )
  {
    const std::vector< Node* > siblings = kernel().node_manager.get_thread_siblings( get_node_id() );
    std::vector< Node* >::const_iterator s;
    for ( s = siblings.begin() + 1; s != siblings.end(); ++s )
    {
      ( *s )->get_status( d );
    }
  }
}

void
nest::music_cont_out_proxy::set_status( const DictionaryDatum& d )
{
  P_.set( d, *this, S_, B_ ); // throws if BadProperty
}

void
nest::music_cont_out_proxy::update( Time const& origin, const long from, const long )
{
  /* There is nothing to request during the first time slice. For
     each subsequent slice, we collect all data generated during
     the previous slice if we are called at the beginning of the
     slice. Otherwise, we do nothing.
   */
  if ( origin.get_steps() == 0 || from != 0 )
  {
    return;
  }

  // We send a request to each of our targets.
  // The target then immediately returns a DataLoggingReply event,
  // which is caught by music_cont_out_proxy::handle(), which in turn
  // ensures that the event is recorded.
  // handle() has access to request_, so it knows what we asked for.
  //
  // Note that not all nodes receiving the request will necessarily answer.
  DataLoggingRequest req;
  kernel().event_delivery_manager.send( *this, req );
}

void
nest::music_cont_out_proxy::handle( DataLoggingReply& reply )
{
  // easy access to relevant information
  DataLoggingReply::Container const& info = reply.get_info();

  const index port = reply.get_port();
  const size_t record_width = P_.record_from_.size();
  const size_t offset = port * record_width;
  const DataLoggingReply::DataItem item = info[ info.size() - 1 ].data;
  if ( info[ info.size() - 1 ].timestamp.is_finite() )
  {
    for ( size_t i = 0; i < item.size(); i++ )
    {
      B_.data_[ offset + i ] = item[ i ];
    }
  }
}

#endif
