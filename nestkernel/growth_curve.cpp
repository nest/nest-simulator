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

// C++ includes:
#include <cmath>

// Includes from nestkernel:
#include "nest_names.h"
#include "nest_time.h"

// Includes from sli:
#include "dictutils.h"

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
  def< double >( d, names::eps, eps_ );
}

void
nest::GrowthCurveLinear::set( const DictionaryDatum& d )
{
  updateValue< double >( d, names::eps, eps_ );
}

double
nest::GrowthCurveLinear::update( double t,
  double t_minus,
  double Ca_minus,
  double z_minus,
  double tau_Ca,
  double growth_rate ) const
{
  const double Ca = Ca_minus * std::exp( ( t_minus - t ) / tau_Ca );
  const double z_value = growth_rate * tau_Ca * ( Ca - Ca_minus ) / eps_ + growth_rate * ( t - t_minus ) + z_minus;

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
  def< double >( d, names::eps, eps_ );
  def< double >( d, names::eta, eta_ );
}

void
nest::GrowthCurveGaussian::set( const DictionaryDatum& d )
{
  updateValue< double >( d, names::eps, eps_ );
  updateValue< double >( d, names::eta, eta_ );
}

double
nest::GrowthCurveGaussian::update( double t,
  double t_minus,
  double Ca_minus,
  double z_minus,
  double tau_Ca,
  double growth_rate ) const
{
  // Numerical integration from t_minus to t
  // use standard forward Euler numerics
  const double h = Time::get_resolution().get_ms();
  const double zeta = ( eta_ - eps_ ) / ( 2.0 * sqrt( log( 2.0 ) ) );
  const double xi = ( eta_ + eps_ ) / 2.0;

  double z_value = z_minus;
  double Ca = Ca_minus;

  for ( double lag = t_minus; lag < ( t - h / 2.0 ); lag += h )
  {
    Ca = Ca - ( ( Ca / tau_Ca ) * h );
    const double dz = h * growth_rate * ( 2.0 * exp( -pow( ( Ca - xi ) / zeta, 2 ) ) - 1.0 );
    z_value = z_value + dz;
  }

  return std::max( z_value, 0.0 );
}

/* ----------------------------------------------------------------
 * GrowthCurveSigmoid
 * ---------------------------------------------------------------- */

nest::GrowthCurveSigmoid::GrowthCurveSigmoid()
  : GrowthCurve( names::sigmoid )
  , eps_( 0.7 )
  , psi_( 0.1 )
{
}

void
nest::GrowthCurveSigmoid::get( DictionaryDatum& d ) const
{
  def< std::string >( d, names::growth_curve, name_.toString() );
  def< double >( d, names::eps, eps_ );
  def< double >( d, names::psi, psi_ );
}

void
nest::GrowthCurveSigmoid::set( const DictionaryDatum& d )
{
  updateValue< double >( d, names::eps, eps_ );
  updateValue< double >( d, names::psi, psi_ );

  // check that w is greater than 0
  if ( not( psi_ >= 0 ) )
  {
    throw BadProperty( "psi parameter must be greater than 0." );
  }
}

double
nest::GrowthCurveSigmoid::update( double t,
  double t_minus,
  double Ca_minus,
  double z_minus,
  double tau_Ca,
  double growth_rate ) const
{
  // Numerical integration from t_minus to t
  // use standard forward Euler numerics
  const double h = Time::get_resolution().get_ms();

  double z_value = z_minus;
  double Ca = Ca_minus;

  for ( double lag = t_minus; lag < ( t - h / 2.0 ); lag += h )
  {
    Ca = Ca - ( ( Ca / tau_Ca ) * h );
    const double dz = h * growth_rate * ( ( 2.0 / ( 1.0 + exp( ( Ca - eps_ ) / psi_ ) ) ) - 1.0 );
    z_value = z_value + dz;
  }

  return std::max( z_value, 0.0 );
}
