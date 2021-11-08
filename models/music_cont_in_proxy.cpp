/*
 *  music_cont_in_proxy.cpp
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

#include "music_cont_in_proxy.h"

#ifdef HAVE_MUSIC

// Includes from sli:
#include "arraydatum.h"
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"

// Includes from libnestutil:
#include "dict_util.h"
#include "compose.hpp"
#include "logging.h"

// Includes from nestkernel:
#include "kernel_manager.h"

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::music_cont_in_proxy::Parameters_::Parameters_()
  : port_name_( "cont_in" )
{
}

nest::music_cont_in_proxy::State_::State_()
  : published_( false )
  , port_width_( -1 )
{
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::music_cont_in_proxy::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::port_name ] = port_name_;
}

void
nest::music_cont_in_proxy::Parameters_::set( const DictionaryDatum& d, State_& s )
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
nest::music_cont_in_proxy::State_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::published ] = published_;
  ( *d )[ names::port_width ] = port_width_;
}

void
nest::music_cont_in_proxy::State_::set( const DictionaryDatum&, const Parameters_& )
{
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::music_cont_in_proxy::music_cont_in_proxy()
  : DeviceNode()
  , P_()
  , S_()
{
}

nest::music_cont_in_proxy::music_cont_in_proxy( const music_cont_in_proxy& n )
  : DeviceNode( n )
  , P_( n.P_ )
  , S_( n.S_ )
{
}


/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::music_cont_in_proxy::init_buffers_()
{
}

void
nest::music_cont_in_proxy::calibrate()
{
  // only publish the port once
  if ( not S_.published_ )
  {
    MUSIC::Setup* s = kernel().music_manager.get_music_setup();
    if ( s == 0 )
    {
      throw MUSICSimulationHasRun( get_name() );
    }

    V_.MP_ = s->publishContInput( P_.port_name_ );

    if ( not V_.MP_->isConnected() )
    {
      throw MUSICPortUnconnected( get_name(), P_.port_name_ );
    }

    if ( not V_.MP_->hasWidth() )
    {
      throw MUSICPortHasNoWidth( get_name(), P_.port_name_ );
    }

    S_.port_width_ = V_.MP_->width();

    B_.data_ = std::vector< double >( S_.port_width_ );
    MUSIC::ArrayData data_map( static_cast< void* >( &( B_.data_[ 0 ] ) ), MPI::DOUBLE, 0, S_.port_width_ );

    V_.MP_->map( &data_map );
    S_.published_ = true;

    std::string msg = String::compose( "Mapping MUSIC input port '%1' with width=%2.", P_.port_name_, S_.port_width_ );
    LOG( M_INFO, "music_cont_in_proxy::calibrate()", msg.c_str() );
  }
}

void
nest::music_cont_in_proxy::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );

  ( *d )[ names::data ] = DoubleVectorDatum( new std::vector< double >( B_.data_ ) );
}

void
nest::music_cont_in_proxy::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d, S_ );     // throws if BadProperty

  State_ stmp = S_;
  stmp.set( d, P_ ); // throws if BadProperty

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

#endif
