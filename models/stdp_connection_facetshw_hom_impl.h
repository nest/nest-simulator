/*
 *  stdp_connection_facetshw_hom_impl.h
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

#ifndef STDP_CONNECTION_FACETSHW_HOM_IMPL_H
#define STDP_CONNECTION_FACETSHW_HOM_IMPL_H

#include "stdp_connection_facetshw_hom.h"

// Includes from nestkernel:
#include "common_synapse_properties.h"
#include "connector_model.h"
#include "event.h"

// Includes from sli:
#include "dictdatum.h"

namespace nest
{
//
// Implementation of class STDPFACETSHWHomCommonProperties.
//

template < typename targetidentifierT >
STDPFACETSHWHomCommonProperties< targetidentifierT >::STDPFACETSHWHomCommonProperties()
  : CommonSynapseProperties()
  , tau_plus_( 20.0 )
  , tau_minus_( 20.0 )
  , Wmax_( 100.0 )
  , no_synapses_( 0 )
  , synapses_per_driver_( 50 )   // hardware efficiency of 50/256=20%,
                                 // which is comparable to Fieres et al. (2008)
  , driver_readout_time_( 15.0 ) // in ms; measured on hardware
{
  lookuptable_0_.resize( 16 );
  lookuptable_1_.resize( 16 );
  lookuptable_2_.resize( 16 );

  // intermediate Guetig (mu=0.4)
  // with r=4 bits and n=36 SSPs, see [3]_
  lookuptable_0_.at( 0 ) = 2;
  lookuptable_0_.at( 1 ) = 3;
  lookuptable_0_.at( 2 ) = 4;
  lookuptable_0_.at( 3 ) = 4;
  lookuptable_0_.at( 4 ) = 5;
  lookuptable_0_.at( 5 ) = 6;
  lookuptable_0_.at( 6 ) = 7;
  lookuptable_0_.at( 7 ) = 8;
  lookuptable_0_.at( 8 ) = 9;
  lookuptable_0_.at( 9 ) = 10;
  lookuptable_0_.at( 10 ) = 11;
  lookuptable_0_.at( 11 ) = 12;
  lookuptable_0_.at( 12 ) = 13;
  lookuptable_0_.at( 13 ) = 14;
  lookuptable_0_.at( 14 ) = 14;
  lookuptable_0_.at( 15 ) = 15;

  lookuptable_1_.at( 0 ) = 0;
  lookuptable_1_.at( 1 ) = 0;
  lookuptable_1_.at( 2 ) = 1;
  lookuptable_1_.at( 3 ) = 2;
  lookuptable_1_.at( 4 ) = 3;
  lookuptable_1_.at( 5 ) = 4;
  lookuptable_1_.at( 6 ) = 5;
  lookuptable_1_.at( 7 ) = 6;
  lookuptable_1_.at( 8 ) = 7;
  lookuptable_1_.at( 9 ) = 8;
  lookuptable_1_.at( 10 ) = 9;
  lookuptable_1_.at( 11 ) = 10;
  lookuptable_1_.at( 12 ) = 10;
  lookuptable_1_.at( 13 ) = 11;
  lookuptable_1_.at( 14 ) = 12;
  lookuptable_1_.at( 15 ) = 13;

  for ( size_t i = 0; i < lookuptable_0_.size(); ++i )
  {
    lookuptable_2_.at( i ) = i;
  }

  configbit_0_.resize( 4 );
  configbit_1_.resize( 4 );

  // see [4]_
  configbit_0_.at( 0 ) = 0;
  configbit_0_.at( 1 ) = 0;
  configbit_0_.at( 2 ) = 1;
  configbit_0_.at( 3 ) = 0;
  configbit_1_.at( 0 ) = 0;
  configbit_1_.at( 1 ) = 1;
  configbit_1_.at( 2 ) = 0;
  configbit_1_.at( 3 ) = 0;

  reset_pattern_.resize( 6 );
  for ( size_t i = 0; i < reset_pattern_.size(); ++i )
  {
    reset_pattern_.at( i ) = true;
  }

  weight_per_lut_entry_ = Wmax_ / ( lookuptable_0_.size() - 1 );
  calc_readout_cycle_duration_();
}

template < typename targetidentifierT >
void
STDPFACETSHWHomCommonProperties< targetidentifierT >::calc_readout_cycle_duration_()
{
  readout_cycle_duration_ = int( ( no_synapses_ - 1.0 ) / synapses_per_driver_ + 1.0 ) * driver_readout_time_;
  // std::cout << "stdp_connection_facetshw_hom::debug: readout cycle duration
  // changed to " <<
  // readout_cycle_duration_ << std::endl;
}

template < typename targetidentifierT >
void
STDPFACETSHWHomCommonProperties< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  CommonSynapseProperties::get_status( d );

  def< double >( d, names::tau_plus, tau_plus_ );
  def< double >( d, names::tau_minus_stdp, tau_minus_ );
  def< double >( d, names::Wmax, Wmax_ );
  def< double >( d, names::weight_per_lut_entry, weight_per_lut_entry_ );

  def< long >( d, names::no_synapses, no_synapses_ );
  def< long >( d, names::synapses_per_driver, synapses_per_driver_ );
  def< double >( d, names::driver_readout_time, driver_readout_time_ );
  def< double >( d, names::readout_cycle_duration, readout_cycle_duration_ );

  ( *d )[ names::lookuptable_0 ] = IntVectorDatum( new std::vector< long >( lookuptable_0_ ) );
  ( *d )[ names::lookuptable_1 ] = IntVectorDatum( new std::vector< long >( lookuptable_1_ ) );
  ( *d )[ names::lookuptable_2 ] = IntVectorDatum( new std::vector< long >( lookuptable_2_ ) );
  ( *d )[ names::configbit_0 ] = IntVectorDatum( new std::vector< long >( configbit_0_ ) );
  ( *d )[ names::configbit_1 ] = IntVectorDatum( new std::vector< long >( configbit_1_ ) );
  ( *d )[ names::reset_pattern ] = IntVectorDatum( new std::vector< long >( reset_pattern_ ) );
}

template < typename targetidentifierT >
void
STDPFACETSHWHomCommonProperties< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  CommonSynapseProperties::set_status( d, cm );

  updateValue< double >( d, names::tau_plus, tau_plus_ );
  updateValue< double >( d, names::tau_minus_stdp, tau_minus_ );
  if ( updateValue< double >( d, names::Wmax, Wmax_ ) )
  {
    weight_per_lut_entry_ = Wmax_ / ( lookuptable_0_.size() - 1 );
  }

  // TP: they should not be allowed to be changed! But needed for CopyModel ...
  updateValue< double >( d, names::weight_per_lut_entry, weight_per_lut_entry_ );
  updateValue< double >( d, names::readout_cycle_duration, readout_cycle_duration_ );
  if ( updateValue< long >( d, names::no_synapses, no_synapses_ ) )
  {
    calc_readout_cycle_duration_();
  }

  if ( updateValue< long >( d, names::synapses_per_driver, synapses_per_driver_ ) )
  {
    calc_readout_cycle_duration_();
  }
  if ( updateValue< double >( d, names::driver_readout_time, driver_readout_time_ ) )
  {
    calc_readout_cycle_duration_();
  }

  if ( d->known( names::lookuptable_0 ) )
  {
    updateValue< std::vector< long > >( d, names::lookuptable_0, lookuptable_0_ );

    // right size?
    if ( lookuptable_0_.size() != lookuptable_1_.size() )
    {
      throw BadProperty( "Look-up table has not 2^4 entries!" );
    }

    // are look-up table entries out of bounds?
    for ( size_t i = 0; i < size_t( lookuptable_0_.size() ); ++i )
    {
      if ( ( lookuptable_0_[ i ] < 0 ) || ( lookuptable_0_[ i ] > 15 ) )
      {
        throw BadProperty( "Look-up table entries must be integers in [0,15]" );
      }
    }
  }
  if ( d->known( names::lookuptable_1 ) )
  {
    updateValue< std::vector< long > >( d, names::lookuptable_1, lookuptable_1_ );

    // right size?
    if ( lookuptable_1_.size() != lookuptable_0_.size() )
    {
      throw BadProperty( "Look-up table has not 2^4 entries!" );
    }

    // are look-up table entries out of bounds?
    for ( size_t i = 0; i < size_t( lookuptable_1_.size() ); ++i )
    {
      if ( ( lookuptable_1_[ i ] < 0 ) || ( lookuptable_1_[ i ] > 15 ) )
      {
        throw BadProperty( "Look-up table entries must be integers in [0,15]" );
      }
    }
  }
  if ( d->known( names::lookuptable_2 ) )
  {
    updateValue< std::vector< long > >( d, names::lookuptable_2, lookuptable_2_ );

    // right size?
    if ( lookuptable_2_.size() != lookuptable_0_.size() )
    {
      throw BadProperty( "Look-up table has not 2^4 entries!" );
    }

    // are look-up table entries out of bounds?
    for ( size_t i = 0; i < size_t( lookuptable_2_.size() ); ++i )
    {
      if ( ( lookuptable_2_[ i ] < 0 ) || ( lookuptable_2_[ i ] > 15 ) )
      {
        throw BadProperty( "Look-up table entries must be integers in [0,15]" );
      }
    }
  }

  if ( d->known( names::configbit_0 ) )
  {
    updateValue< std::vector< long > >( d, names::configbit_0, configbit_0_ );

    // right size?
    if ( configbit_0_.size() != 4 )
    {
      throw BadProperty( "Wrong number of configuration bits (!=4)." );
    }
  }
  if ( d->known( names::configbit_1 ) )
  {
    updateValue< std::vector< long > >( d, names::configbit_1, configbit_1_ );

    // right size?
    if ( configbit_1_.size() != 4 )
    {
      throw BadProperty( "Wrong number of configuration bits (!=4)." );
    }
  }
  if ( d->known( names::reset_pattern ) )
  {
    updateValue< std::vector< long > >( d, names::reset_pattern, reset_pattern_ );

    // right size?
    if ( reset_pattern_.size() != 6 )
    {
      throw BadProperty( "Wrong number of reset bits (!=6)." );
    }
  }
}


//
// Implementation of class STDPFACETSHWConnectionHom.
//
template < typename targetidentifierT >
STDPFACETSHWConnectionHom< targetidentifierT >::STDPFACETSHWConnectionHom()
  : weight_( 1.0 )
  , a_causal_( 0.0 )
  , a_acausal_( 0.0 )
  , a_thresh_th_( 21.835 )
  , a_thresh_tl_( 21.835 ) // exp(-10ms/20ms) * 36SSPs
  , init_flag_( false )
  , synapse_id_( 0 )
  , next_readout_time_( 0.0 )
  , discrete_weight_( 0 )
  , t_lastspike_( 0.0 )
{
}

template < typename targetidentifierT >
void
STDPFACETSHWConnectionHom< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  // base class properties, different for individual synapse
  ConnectionBase::get_status( d );
  def< double >( d, names::weight, weight_ );

  // own properties, different for individual synapse
  def< double >( d, names::a_causal, a_causal_ );
  def< double >( d, names::a_acausal, a_acausal_ );
  def< double >( d, names::a_thresh_th, a_thresh_th_ );
  def< double >( d, names::a_thresh_tl, a_thresh_tl_ );

  def< bool >( d, names::init_flag, init_flag_ );
  def< long >( d, names::synapse_id, synapse_id_ );
  def< double >( d, names::next_readout_time, next_readout_time_ );
  // useful to get conversion before activity, but weight_per_lut_entry_ not
  // known here
  // def<unsigned int>(d, "discrete_weight",
  // entry_to_weight_(weight_to_entry_(weight_,
  // weight_per_lut_entry_), weight_per_lut_entry_));
}

template < typename targetidentifierT >
void
STDPFACETSHWConnectionHom< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  // base class properties
  ConnectionBase::set_status( d, cm );
  updateValue< double >( d, names::weight, weight_ );

  updateValue< double >( d, names::a_causal, a_causal_ );
  updateValue< double >( d, names::a_acausal, a_acausal_ );
  updateValue< double >( d, names::a_thresh_th, a_thresh_th_ );
  updateValue< double >( d, names::a_thresh_tl, a_thresh_tl_ );

  updateValue< long >( d, names::synapse_id, synapse_id_ );

  // TP: they should not be allowed to be changed! But needed for CopyModel ...
  updateValue< bool >( d, names::init_flag, init_flag_ );
  updateValue< double >( d, names::next_readout_time, next_readout_time_ );

  // setting discrete_weight_ does not make sense, is temporary variable
}

} // of namespace nest

#endif // #ifndef STDP_CONNECTION_FACETSHW_HOM_IMPL_H
