/*
 *  music_event_out_proxy.cpp
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

#include "music_event_out_proxy.h"

#ifdef HAVE_MUSIC

// C++ includes:
#include <numeric>

// Includes from sli:
#include "arraydatum.h"
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"

// Includes from libnestutil:
#include "compose.hpp"
#include "logging.h"

// Includes from nestkernel:
#include "kernel_manager.h"

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::music_event_out_proxy::Parameters_::Parameters_()
  : port_name_( "event_out" )
{
}

nest::music_event_out_proxy::State_::State_()
  : published_( false )
  , port_width_( -1 )
{
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::music_event_out_proxy::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::port_name ] = port_name_;
}

void
nest::music_event_out_proxy::Parameters_::set( const DictionaryDatum& d, State_& s )
{
  // TODO: This is not possible, as P_ does not know about get_name()
  //  if(d->known(names::port_name) && s.published_)
  //    throw MUSICPortAlreadyPublished(get_name(), P_.port_name_);

  if ( not s.published_ )
  {
    updateValue< string >( d, names::port_name, port_name_ );
  }
}

void
nest::music_event_out_proxy::State_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::published ] = published_;
  ( *d )[ names::port_width ] = port_width_;
}

void
nest::music_event_out_proxy::State_::set( const DictionaryDatum&, const Parameters_& )
{
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::music_event_out_proxy::music_event_out_proxy()
  : DeviceNode()
  , P_()
  , S_()
{
}

nest::music_event_out_proxy::music_event_out_proxy( const music_event_out_proxy& n )
  : DeviceNode( n )
  , P_( n.P_ )
  , S_( n.S_ )
{
}

nest::music_event_out_proxy::~music_event_out_proxy()
{
  if ( S_.published_ )
  {
    delete V_.MP_;
    delete V_.music_perm_ind_;
  }
}

void
nest::music_event_out_proxy::init_buffers_()
{
}

void
nest::music_event_out_proxy::calibrate()
{
  // only publish the output port once,
  if ( not S_.published_ )
  {
    MUSIC::Setup* s = kernel().music_manager.get_music_setup();
    if ( s == 0 )
    {
      throw MUSICSimulationHasRun( get_name() );
    }

    V_.MP_ = s->publishEventOutput( P_.port_name_ );

    if ( not V_.MP_->isConnected() )
    {
      throw MUSICPortUnconnected( get_name(), P_.port_name_ );
    }

    if ( not V_.MP_->hasWidth() )
    {
      throw MUSICPortHasNoWidth( get_name(), P_.port_name_ );
    }

    S_.port_width_ = V_.MP_->width();

    // check, if there are connections to receiver ports, which are
    // beyond the width of the port
    std::vector< MUSIC::GlobalIndex >::const_iterator it;
    for ( it = V_.index_map_.begin(); it != V_.index_map_.end(); ++it )
    {
      if ( *it > S_.port_width_ )
      {
        throw UnknownReceptorType( *it, get_name() );
      }
    }

    // The permutation index map, contains global_index[local_index]
    V_.music_perm_ind_ = new MUSIC::PermutationIndex( &V_.index_map_.front(), V_.index_map_.size() );

    // we identify channels by global indices within NEST
    V_.MP_->map( V_.music_perm_ind_, MUSIC::Index::GLOBAL );

    S_.published_ = true;

    std::string msg = String::compose( "Mapping MUSIC output port '%1' with width=%2.", P_.port_name_, S_.port_width_ );
    LOG( M_INFO, "MusicEventHandler::publish_port()", msg.c_str() );
  }
}

void
nest::music_event_out_proxy::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );

  ( *d )[ names::connection_count ] = V_.index_map_.size();

  // make a copy, since MUSIC uses int instead of long int
  std::vector< long >* pInd_map_long = new std::vector< long >( V_.index_map_.size() );
  std::copy< std::vector< MUSIC::GlobalIndex >::const_iterator, std::vector< long >::iterator >(
    V_.index_map_.begin(), V_.index_map_.end(), pInd_map_long->begin() );

  ( *d )[ names::index_map ] = IntVectorDatum( pInd_map_long );
}

void
nest::music_event_out_proxy::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d, S_ );     // throws if BadProperty

  State_ stmp = S_;
  stmp.set( d, P_ ); // throws if BadProperty

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

void
nest::music_event_out_proxy::handle( SpikeEvent& e )
{
  assert( e.get_multiplicity() > 0 );

  // propagate the spikes to MUSIC port
  double time = e.get_stamp().get_ms() * 1e-3; // event time in seconds
  long receiver_port = e.get_rport();

#ifdef _OPENMP
#pragma omp critical( insertevent )
  {
#endif
    for ( int i = 0; i < e.get_multiplicity(); ++i )
    {
      V_.MP_->insertEvent( time, MUSIC::GlobalIndex( receiver_port ) );
    }
#ifdef _OPENMP
  }
#endif
}

#endif
