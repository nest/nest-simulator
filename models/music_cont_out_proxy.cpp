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

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"

// Includes from libnestutil:
#include "compose.hpp"
#include "logging.h"

// Includes from C++:
#include <numeric>
#include <string>

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "kernel_manager.h"
#include "sibling_container.h"

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ----------------------------------------------------------------
 */

nest::music_cont_out_proxy::Parameters_::Parameters_()
  : port_name_( "cont_out" )
  , interval_( Time::ms( 1.0 ) )
  , record_from_()
{
}

nest::music_cont_out_proxy::Parameters_::Parameters_( const Parameters_& p )
  : port_name_( p.port_name_ )
  , interval_( p.interval_ )
  , record_from_( p.record_from_ )
{
  interval_.calibrate();
}

nest::music_cont_out_proxy::State_::State_()
  : published_( false )
  , port_width_( -1 )
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

nest::music_cont_out_proxy::Variables_::Variables_()
  : index_map_()
{
}

nest::music_cont_out_proxy::Variables_::Variables_( const Variables_& v )
  : index_map_( v.index_map_ )
{
}
/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::music_cont_out_proxy::Parameters_::get( DictionaryDatum& d,
  const Variables_& vars ) const
{
  ( *d )[ names::port_name ] = port_name_;
  ( *d )[ names::interval ] = interval_.get_ms();

  ArrayDatum ad_record_from;

  for ( size_t j = 0; j < record_from_.size(); ++j )
  {
    ad_record_from.push_back( LiteralDatum( record_from_[ j ] ) );
  }

  ( *d )[ names::record_from ] = ad_record_from;

  std::vector< long_t >* pInd_map_long =
    new std::vector< long_t >( vars.index_map_.size() );

  std::copy< std::vector< MUSIC::GlobalIndex >::const_iterator,
    std::vector< long_t >::iterator >(
    vars.index_map_.begin(), vars.index_map_.end(), pInd_map_long->begin() );

  ( *d )[ names::index_map ] = IntVectorDatum( pInd_map_long );
}

void
nest::music_cont_out_proxy::Parameters_::set( const DictionaryDatum& d,
  const State_& states,
  const Buffers_& buffs )
{

  if ( states.published_ == false )
  {
    updateValue< string >( d, names::port_name, port_name_ );
  }

  if ( buffs.has_targets_
    && ( d->known( names::interval ) || d->known( names::record_from ) ) )
  {
    throw BadProperty(
      "The recording interval and the list of properties to record "
      "cannot be changed after the index_map has been set." );
  }

  double_t v;
  if ( updateValue< double_t >( d, names::interval, v ) )
  {
    if ( Time( Time::ms( v ) ) < Time::get_resolution() )
    {
      throw BadProperty(
        "The sampling interval must be at least as long "
        "as the simulation resolution." );
    }

    // see if we can represent interval as multiple of step
    interval_ = Time::step( Time( Time::ms( v ) ).get_steps() );
    if ( std::abs( 1 - interval_.get_ms() / v ) > 10
        * std::numeric_limits< double >::epsilon() )
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
}

void
nest::music_cont_out_proxy::State_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::published ] = published_;
  ( *d )[ names::port_width ] = port_width_;
}

void
nest::music_cont_out_proxy::State_::set( const DictionaryDatum& d,
  const Parameters_& p )
{
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::music_cont_out_proxy::music_cont_out_proxy()
  : Node()
  , P_()
  , S_()
  , V_()
  , B_()
{
}

nest::music_cont_out_proxy::music_cont_out_proxy(
  const music_cont_out_proxy& n )
  : Node( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , V_( n.V_ )
  , B_( n.B_ )
{
}


void
nest::music_cont_out_proxy::init_state_( const Node& /* np */ )
{
  // const Multimeter& asd = dynamic_cast< const Multimeter& >( np );
  // device_.init_state( asd.device_ );
  B_.data_.clear();
}

void
nest::music_cont_out_proxy::init_buffers_()
{
}

void
nest::music_cont_out_proxy::finalize()
{
}

nest::port
nest::music_cont_out_proxy::send_test_event( Node& target,
  rport receptor_type,
  synindex,
  bool )
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
    std::vector< MUSIC::GlobalIndex >::const_iterator it;
    for ( it = V_.index_map_.begin(); it != V_.index_map_.end(); ++it )
    {
      if ( *it > S_.port_width_ )
      {
        throw MUSICChannelUnknown( get_name(), P_.port_name_, *it );
      }
    }

    // The permutation index map, contains global_index[local_index]
    MUSIC::PermutationIndex* music_perm_ind = new MUSIC::PermutationIndex(
      &V_.index_map_.front(), V_.index_map_.size() );

    MUSIC::ArrayData* dmap = NULL;
    if ( per_port_width > 1 )
    {
      // New MPI datatype a compound of multiple double values.
      // The receiver applications must support the MPI
      // contiguous type as well
      MPI_Datatype n_double_tuple;
      MPI_Type_contiguous( per_port_width, MPI::DOUBLE, &n_double_tuple );
      dmap =
        new MUSIC::ArrayData( static_cast< void* >( &( B_.data_.front() ) ),
          n_double_tuple,
          music_perm_ind );
    }
    else
    {
      dmap =
        new MUSIC::ArrayData( static_cast< void* >( &( B_.data_.front() ) ),
          MPI::DOUBLE,
          music_perm_ind );
    }

    // Setup an array map
    MP->map( dmap );

    S_.published_ = true;

    std::string msg =
      String::compose( "Mapping MUSIC output port '%1' with width=%2.",
        P_.port_name_,
        S_.port_width_ );
    LOG( M_INFO, "music_cont_out_proxy::calibrate()", msg.c_str() );
  }
}

void
nest::music_cont_out_proxy::get_status( DictionaryDatum& d ) const
{
  // if we are the device on thread 0, also get the data from the
  // siblings on other threads
  if ( get_thread() == 0 )
  {
    const SiblingContainer* siblings =
      kernel().node_manager.get_thread_siblings( get_gid() );
    std::vector< Node* >::const_iterator sibling;
    for ( sibling = siblings->begin() + 1; sibling != siblings->end();
          ++sibling )
    {
      ( *sibling )->get_status( d );
    }
  }

  P_.get( d, V_ );
  S_.get( d );
}

void
nest::music_cont_out_proxy::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d, S_, B_ ); // throws if BadProperty

  State_ stmp = S_;
  stmp.set( d, P_ ); // throws if BadProperty

  if ( d->known( names::index_map ) )
  {
    if ( S_.published_ == false )
    {
      const Token synmodel =
        kernel().model_manager.get_synapsedict()->lookup( "static_synapse" );
      const index synmodel_id = static_cast< index >( synmodel );
      DictionaryDatum syn_defaults =
        kernel().model_manager.get_connector_defaults( synmodel_id );
      ArrayDatum mca = getValue< ArrayDatum >( d, names::index_map );
      size_t music_index = 0;

      for ( Token* t = mca.begin(); t != mca.end(); ++t, ++music_index )
      {

        const long target_node_id = getValue< long_t >( *t );
        if ( kernel().node_manager.is_local_gid( target_node_id ) )
        {
          // std::distance( mca.begin(), t )
          V_.index_map_.push_back( static_cast< int_t >( music_index ) );
          Node* const target_node =
            kernel().node_manager.get_node( target_node_id );
          const thread target_thread = target_node->get_thread();
          kernel().connection_manager.connect(
            this->get_gid(), target_node, target_thread, synmodel_id );
        }
      }
    }
    else
    {
      throw MUSICPortAlreadyPublished( get_name(), P_.port_name_ );
    }
  }

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

void
nest::music_cont_out_proxy::update( Time const& origin,
  const long_t from,
  const long_t )
{
  /* There is nothing to request during the first time slice.
     For each subsequent slice, we collect all data generated during the
     previous
     slice if we are called at the beginning of the slice. Otherwise, we do
     nothing.
   */
  if ( origin.get_steps() == 0 || from != 0 )
  {
    return;
  }

  // We send a request to each of our targets.
  // The target then immediately returns a DataLoggingReply event,
  // which is caught by multimeter::handle(), which in turn
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
