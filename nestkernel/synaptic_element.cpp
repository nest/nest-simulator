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
#include "dictutils.h"
#include "exceptions.h"
#include "archiving_node.h"
#include "network.h"

/* ----------------------------------------------------------------
* GrowthCurveLinear
* ---------------------------------------------------------------- */
nest::GrowthCurveLinear::GrowthCurveLinear()
  : eps( 0.7 )
{
  name = "linear";
}

void
nest::GrowthCurveLinear::get( DictionaryDatum& d ) const
{
  def< std::string >( d, "growth_curve", name );
  def< double_t >( d, "eps", eps );
}

void
nest::GrowthCurveLinear::set( const DictionaryDatum& d )
{
  updateValue< double_t >( d, "eps", eps );
}

nest::double_t
nest::GrowthCurveLinear::update( double_t t,
  double_t t_minus,
  double_t Ca_minus,
  double_t z_minus,
  double_t tau_Ca,
  double_t growth_rate ) const
{
  double_t Ca, z_value;
  Ca = Ca_minus * std::exp( ( t_minus - t ) / tau_Ca );
  z_value =
    growth_rate * tau_Ca * ( Ca - Ca_minus ) / eps + growth_rate * ( t - t_minus ) + z_minus;

  if ( z_value > 0 )
    return z_value;
  else
    return 0.0;
}

/* ----------------------------------------------------------------
* GrowthCurveGaussian
* ---------------------------------------------------------------- */
nest::GrowthCurveGaussian::GrowthCurveGaussian()
  : eta( 0.1 )
  , eps( 0.7 )
{
  name = "gaussian";
}

void
nest::GrowthCurveGaussian::get( DictionaryDatum& d ) const
{
  def< std::string >( d, "growth_curve", name );
  def< double_t >( d, "eps", eps );
  def< double_t >( d, "eta", eta );
}

void
nest::GrowthCurveGaussian::set( const DictionaryDatum& d )
{
  updateValue< double_t >( d, "eps", eps );
  updateValue< double_t >( d, "eta", eta );
}

nest::double_t
nest::GrowthCurveGaussian::update( double_t t,
  double_t t_minus,
  double_t Ca_minus,
  double_t z_minus,
  double_t tau_Ca,
  double_t growth_rate ) const
{
  double_t lag, dz, zeta, xi, Ca, z_value;
  const double_t h = Time::get_resolution().get_ms();

  // Numerical integration from t_minus to t
  // use standard forward Euler numerics
  zeta = ( eta - eps ) / ( 2.0 * sqrt( log( 2.0 ) ) );
  xi = ( eta + eps ) / 2.0;

  z_value = z_minus;
  Ca = Ca_minus;

  for ( lag = t_minus; lag < ( t - h / 2.0 ); lag += h )
  {
    Ca = Ca - ( ( Ca / tau_Ca ) * h );
    dz = h * growth_rate * ( 2.0 * exp( -pow( ( Ca - xi ) / zeta, 2 ) ) - 1.0 );
    z_value = z_value + dz;
  }

  //  return z_value;
  if ( z_value > 0 )
    return z_value;
  else
    return 0.0;
}


/* ----------------------------------------------------------------
* SynapticElement
* Default constructors defining default parameters and state
* ---------------------------------------------------------------- */

nest::SynapticElement::SynapticElement()
  : z_minus_( 0.0 )
  , z_minus_t_( 0.0 )
  , z_connected_( 0 )
  , continuous_( true )
  , growth_rate_( 1.0 )
  , tau_vacant_( 10.0 )
  , growth_curve_( new GrowthCurveLinear )
{
}

nest::SynapticElement::SynapticElement( const SynapticElement& se )
  : z_minus_( se.z_minus_ )
  , z_minus_t_( se.z_minus_t_ )
  , z_connected_( se.z_connected_ )
  , continuous_( se.continuous_ )
  , growth_rate_( se.growth_rate_ )
  , tau_vacant_( se.tau_vacant_ )
{
  growth_curve_ = new_growth_curve( se.growth_curve_->get_name() );
  DictionaryDatum gc_parameters = DictionaryDatum( new Dictionary );
  se.get( gc_parameters );
  growth_curve_->set( gc_parameters );
}

nest::SynapticElement& nest::SynapticElement::operator=( const SynapticElement& other )
{
  if ( this != &other )
  {
    // 1: allocate new memory and copy the elements
    GrowthCurve* new_gc = new_growth_curve( other.growth_curve_->get_name() );
    DictionaryDatum gc_parameters = DictionaryDatum( new Dictionary );

    other.get( gc_parameters );
    new_gc->set( gc_parameters );

    delete growth_curve_;
    growth_curve_ = new_gc;

    z_minus_ = other.z_minus_;
    z_minus_t_ = other.z_minus_t_;
    z_connected_ = other.z_connected_;
    continuous_ = other.continuous_;
    growth_rate_ = other.growth_rate_;
    tau_vacant_ = other.tau_vacant_;
  }
  return *this;
}

nest::GrowthCurve*
nest::SynapticElement::new_growth_curve( std::string name )
{
  if ( !name.compare( "gaussian" ) )
  {
    return new GrowthCurveGaussian;
  }
  if ( !name.compare( "linear" ) )
  {
    return new GrowthCurveLinear;
  }
  throw BadProperty( "Unknown growth curve. Available growth curve are: linear, gaussian." );
}

/* ----------------------------------------------------------------
* get function to store current values in dictionary
* ---------------------------------------------------------------- */
void
nest::SynapticElement::get( DictionaryDatum& d ) const
{
  // Store current values in the dictionary
  def< double_t >( d, "growth_rate", growth_rate_ );
  def< double_t >( d, "tau_vacant", tau_vacant_ );
  def< bool >( d, "continuous", continuous_ );
  def< double_t >( d, "z", z_minus_ );
  def< int_t >( d, "z_connected", z_connected_ );

  // Store growth curve
  growth_curve_->get( d );
}

/* ----------------------------------------------------------------
* set function to store dictionary values in the SynaticElement
* ---------------------------------------------------------------- */
void
nest::SynapticElement::set( const DictionaryDatum& d )
{
  double_t new_tau_vacant = tau_vacant_;

  // Store values
  updateValue< double_t >( d, "growth_rate", growth_rate_ );
  updateValue< double_t >( d, "tau_vacant", new_tau_vacant );
  updateValue< bool >( d, "continuous", continuous_ );
  updateValue< double_t >( d, "z", z_minus_ );

  if ( d->known( "growth_curve" ) )
  {
    std::string growth_curve_name = getValue< std::string >( d, "growth_curve" );
    if ( !growth_curve_->is( growth_curve_name ) )
    {
      growth_curve_ = new_growth_curve( growth_curve_name );
    }
  }
  growth_curve_->set( d );

  if ( new_tau_vacant <= 0.0 )
    throw BadProperty( "All time constants must be strictly positive." );
  tau_vacant_ = new_tau_vacant;
}


/* ----------------------------------------------------------------
* Update the number of element at the time t (in ms)
* ---------------------------------------------------------------- */
void
nest::SynapticElement::update( double_t t, double_t t_minus, double_t Ca_minus, double_t tau_Ca )
{
  if ( z_minus_t_ != t_minus )
  {
    throw KernelException(
      "Last update of the calcium concentration does not match the last update of the synaptic "
      "element" );
  }
  z_minus_ = growth_curve_->update( t, t_minus, Ca_minus, z_minus_, tau_Ca, growth_rate_ );
  z_minus_t_ = t;
}