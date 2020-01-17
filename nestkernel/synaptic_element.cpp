/*
 *  synaptic_element.cpp
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

/**
 * \file synaptic_element.cpp
 * Implementation of synaptic_element and growth_curve
 * \author Mikael Naveau
 * \date July 2013
 */

#include "synaptic_element.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"

// Includes from sli:
#include "dictutils.h"

/* ----------------------------------------------------------------
* SynapticElement
* Default constructors defining default parameters and state
* ---------------------------------------------------------------- */

nest::SynapticElement::SynapticElement()
  : z_( 0.0 )
  , z_t_( 0.0 )
  , z_connected_( 0 )
  , continuous_( true )
  , growth_rate_( 1.0 )
  , tau_vacant_( 0.1 )
  , growth_curve_( new GrowthCurveLinear )
{
}

nest::SynapticElement::SynapticElement( const SynapticElement& se )
  : z_( se.z_ )
  , z_t_( se.z_t_ )
  , z_connected_( se.z_connected_ )
  , continuous_( se.continuous_ )
  , growth_rate_( se.growth_rate_ )
  , tau_vacant_( se.tau_vacant_ )
{
  growth_curve_ = kernel().sp_manager.new_growth_curve( se.growth_curve_->get_name() );
  assert( growth_curve_ != 0 );
  DictionaryDatum nc_parameters = DictionaryDatum( new Dictionary );
  se.get( nc_parameters );
  growth_curve_->set( nc_parameters );
}

nest::SynapticElement& nest::SynapticElement::operator=( const SynapticElement& other )
{
  if ( this != &other )
  {
    // 1: allocate new memory and copy the elements
    GrowthCurve* new_nc = kernel().sp_manager.new_growth_curve( other.growth_curve_->get_name() );
    DictionaryDatum nc_parameters = DictionaryDatum( new Dictionary );

    other.get( nc_parameters );
    new_nc->set( nc_parameters );

    delete growth_curve_;
    growth_curve_ = new_nc;

    z_ = other.z_;
    z_t_ = other.z_t_;
    z_connected_ = other.z_connected_;
    continuous_ = other.continuous_;
    growth_rate_ = other.growth_rate_;
    tau_vacant_ = other.tau_vacant_;
  }
  return *this;
}

/* ----------------------------------------------------------------
* get function to store current values in dictionary
* ---------------------------------------------------------------- */
void
nest::SynapticElement::get( DictionaryDatum& d ) const
{
  // Store current values in the dictionary
  def< double >( d, names::growth_rate, growth_rate_ );
  def< double >( d, names::tau_vacant, tau_vacant_ );
  def< bool >( d, names::continuous, continuous_ );
  def< double >( d, names::z, z_ );
  def< int >( d, names::z_connected, z_connected_ );

  // Store growth curve
  growth_curve_->get( d );
}

/* ----------------------------------------------------------------
* set function to store dictionary values in the SynaticElement
* ---------------------------------------------------------------- */
void
nest::SynapticElement::set( const DictionaryDatum& d )
{
  double new_tau_vacant = tau_vacant_;

  // Store values
  updateValue< double >( d, names::growth_rate, growth_rate_ );
  updateValue< double >( d, names::tau_vacant, new_tau_vacant );
  updateValue< bool >( d, names::continuous, continuous_ );
  updateValue< double >( d, names::z, z_ );

  if ( d->known( names::growth_curve ) )
  {
    Name growth_curve_name( getValue< std::string >( d, names::growth_curve ) );
    if ( not growth_curve_->is( growth_curve_name ) )
    {
      growth_curve_ = kernel().sp_manager.new_growth_curve( growth_curve_name );
    }
  }
  growth_curve_->set( d );

  if ( new_tau_vacant <= 0.0 )
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }
  tau_vacant_ = new_tau_vacant;
}


/* ----------------------------------------------------------------
* Update the number of element at the time t (in ms)
* ---------------------------------------------------------------- */
void
nest::SynapticElement::update( double t, double t_minus, double Ca_minus, double tau_Ca )
{
  if ( z_t_ != t_minus )
  {
    throw KernelException(
      "Last update of the calcium concentration does not match the last update "
      "of the synaptic element" );
  }
  z_ = growth_curve_->update( t, t_minus, Ca_minus, z_, tau_Ca, growth_rate_ );
  z_t_ = t;
}
