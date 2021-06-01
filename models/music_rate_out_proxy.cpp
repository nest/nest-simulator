/*
 *  music_rate_out_proxy.cpp
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

#include "music_rate_out_proxy.h"

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

nest::music_rate_out_proxy::Parameters_::Parameters_()
  : port_name_( "rate_out" )
{
}

nest::music_rate_out_proxy::State_::State_()
  : published_( false )
  , port_width_( -1 )
{
}

nest::music_rate_out_proxy::Buffers_::Buffers_()
  : data_()
{
}

nest::music_rate_out_proxy::Buffers_::Buffers_( const Buffers_& b )
  : data_( b.data_ )
{
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::music_rate_out_proxy::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::port_name ] = port_name_;
}

void
nest::music_rate_out_proxy::Parameters_::set( const DictionaryDatum& d, State_& s )
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
nest::music_rate_out_proxy::State_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::published ] = published_;
  ( *d )[ names::port_width ] = port_width_;
}

void
nest::music_rate_out_proxy::State_::set( const DictionaryDatum&, const Parameters_& )
{
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::music_rate_out_proxy::music_rate_out_proxy()
  : DeviceNode()
  , P_()
  , S_()
{
}

nest::music_rate_out_proxy::music_rate_out_proxy( const music_rate_out_proxy& n )
  : DeviceNode( n )
  , P_( n.P_ )
  , S_( n.S_ )
{
}

nest::music_rate_out_proxy::~music_rate_out_proxy()
{
  if ( S_.published_ )
  {
    delete V_.MP_;
  }
}

void
nest::music_rate_out_proxy::init_buffers_()
{
}

void
nest::music_rate_out_proxy::calibrate()
{
  // only publish the output port once,
  if ( not S_.published_ )
  {
    MUSIC::Setup* s = kernel().music_manager.get_music_setup();

    if ( s == 0 )
    {
      throw MUSICSimulationHasRun( "" );
    }

    V_.MP_ = s->publishContOutput( P_.port_name_ );

    if ( not V_.MP_->isConnected() )
    {
      throw MUSICPortUnconnected( "", P_.port_name_ );
    }

    if ( not V_.MP_->hasWidth() )
    {
      throw MUSICPortHasNoWidth( "", P_.port_name_ );
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

    // Allocate memory
    B_.data_.resize( S_.port_width_ );


    MUSIC::ArrayData* dmap =
      new MUSIC::ArrayData( static_cast< void* >( &( B_.data_.front() ) ), MPI::DOUBLE, 0, S_.port_width_ );


    // Setup an array map
    V_.MP_->map( dmap, 1 );

    S_.published_ = true;


    std::string msg = String::compose( "Mapping MUSIC output port '%1' with width=%2.", P_.port_name_, S_.port_width_ );
    LOG( M_INFO, "MusicRateHandler::publish_port()", msg.c_str() );
  }
}

void
nest::music_rate_out_proxy::get_status( DictionaryDatum& d ) const
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
nest::music_rate_out_proxy::set_status( const DictionaryDatum& d )
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
nest::music_rate_out_proxy::handle( InstantaneousRateConnectionEvent& e )
{

  // propagate last rate in min delay interval to MUSIC port; we can not use
  // e.end() - 1 since the iterator is defined in terms of unsigned ints, not
  // the event DataType; instead we forward iterate using e.get_coeffvalue and
  // overwrite the element in data_ for every dt step
  const long receiver_port = e.get_rport();

  std::vector< unsigned int >::iterator it = e.begin();
  // The call to get_coeffvalue( it ) in this loop also advances the iterator it
  while ( it != e.end() )
  {
    B_.data_[ receiver_port ] = e.get_coeffvalue( it );
  }
}

#endif
