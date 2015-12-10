/*
 *  growth_curve.cpp
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
 * \file growth_curve.cpp
 * Implementation of growth_curve
 * \author Mikael Naveau
 * \date July 2013
 */

#include "growth_curve.h"
#include "dictutils.h"
#include "exceptions.h"
#include "archiving_node.h"
#include "network.h"

/* ----------------------------------------------------------------
 * GrowthCurveLinear
 * ---------------------------------------------------------------- */

nest::GrowthCurveLinear::GrowthCurveLinear()
  : GrowthCurve( names::linear )
  , eps_( 0.7 )
{
}

void
nest::GrowthCurveLinear::get( DictionaryDatum& d ) const
{
  def< std::string >( d, names::growth_curve, name_.toString() );
  def< double_t >( d, names::eps, eps_ );
}

void
nest::GrowthCurveLinear::set( const DictionaryDatum& d )
{
  updateValue< double_t >( d, names::eps, eps_ );
}

nest::double_t
nest::GrowthCurveLinear::update( double_t t,
  double_t t_minus,
  double_t Ca_minus,
  double_t z_minus,
  double_t tau_Ca,
  double_t growth_rate ) const
{
  const double_t Ca = Ca_minus * std::exp( ( t_minus - t ) / tau_Ca );
  const double_t z_value =
    growth_rate * tau_Ca * ( Ca - Ca_minus ) / eps_ + growth_rate * ( t - t_minus ) + z_minus;

  return std::max( z_value, 0.0 );
}

/* ----------------------------------------------------------------
 * GrowthCurveGaussian
 * ---------------------------------------------------------------- */

nest::GrowthCurveGaussian::GrowthCurveGaussian()
  : GrowthCurve( names::gaussian )
  , eta_( 0.1 )
  , eps_( 0.7 )
{
}

void
nest::GrowthCurveGaussian::get( DictionaryDatum& d ) const
{
  def< std::string >( d, names::growth_curve, name_.toString() );
  def< double_t >( d, names::eps, eps_ );
  def< double_t >( d, names::eta, eta_ );
}

void
nest::GrowthCurveGaussian::set( const DictionaryDatum& d )
{
  updateValue< double_t >( d, names::eps, eps_ );
  updateValue< double_t >( d, names::eta, eta_ );
}

nest::double_t
nest::GrowthCurveGaussian::update( double_t t,
  double_t t_minus,
  double_t Ca_minus,
  double_t z_minus,
  double_t tau_Ca,
  double_t growth_rate ) const
{
  // Numerical integration from t_minus to t
  // use standard forward Euler numerics
  const double_t h = Time::get_resolution().get_ms();
  const double_t zeta = ( eta_ - eps_ ) / ( 2.0 * sqrt( log( 2.0 ) ) );
  const double_t xi = ( eta_ + eps_ ) / 2.0;

  double_t z_value = z_minus;
  double_t Ca = Ca_minus;

  for ( double_t lag = t_minus; lag < ( t - h / 2.0 ); lag += h )
  {
    Ca = Ca - ( ( Ca / tau_Ca ) * h );
    const double_t dz = h * growth_rate * ( 2.0 * exp( -pow( ( Ca - xi ) / zeta, 2 ) ) - 1.0 );
    z_value = z_value + dz;
  }

  return std::max( z_value, 0.0 );
}
