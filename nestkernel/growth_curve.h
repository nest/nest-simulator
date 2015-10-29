/*
 *  growth_curve.h
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

#ifndef GROWTH_CURVE_H
#define GROWTH_CURVE_H

/**
 * \file growth_curve.h
 *
 * \author Mikael Naveau
 * \date July 2013
 */

#include "dictdatum.h"
#include "dictutils.h"
#include "histentry.h"
#include <cmath>

namespace nest
{

/**
 * \class GrowthCurve
 * Defines the way the number of synaptic elements changes through time
 * according to the calcium concentration of the neuron.
 */
class GrowthCurve
{
public:
  virtual ~GrowthCurve()
  {
  }
  virtual void get( DictionaryDatum& d ) const = 0;
  virtual void set( const DictionaryDatum& d ) = 0;
  virtual double_t update( double_t t,
    double_t t_minus,
    double_t Ca_minus,
    double_t z,
    double_t tau_Ca,
    double_t growth_rate ) const = 0;
  virtual bool
  is( Name n )
  {
    return n == name_;
  }
  Name
  get_name()
  {
    return name_;
  }

protected:
  GrowthCurve( const Name name )
    : name_( name )
  {
  }
  const Name name_;
};

/**
 * \class GrowthCurveLinear
 * Uses an exact integration method to update the number of synaptic elements:
 * dz/dt = nu (1 - (1/epsilon) * Ca(t)), where nu is the growth rate and
 * epsilon is the desired average calcium concentration.
 */
class GrowthCurveLinear : public GrowthCurve
{
public:
  GrowthCurveLinear();
  void get( DictionaryDatum& d ) const;
  void set( const DictionaryDatum& d );
  double_t update( double_t t,
    double_t t_minus,
    double_t Ca_minus,
    double_t z,
    double_t tau_Ca,
    double_t growth_rate ) const;

private:
  double_t eps_;
};

/**
 * \class GrowthCurveGaussian
 * Uses a forward Euler integration method to update the number of synaptic
 * elements:
 * dz/dt = nu (2 * e^(- ((Ca(t) - xi)/zeta)^2 ) - 1)
 * where xi = (eta  + epsilon)/2,
 * zeta = (epsilon - eta)/2 * sqrt(ln(2))),
 * eta is the minimum calcium concentration required for any synaptic element
 * to be created, epsilon is the target mean calcium concentration in the
 * neuron and nu is the growth rate.
 */
class GrowthCurveGaussian : public GrowthCurve
{
public:
  GrowthCurveGaussian();
  void get( DictionaryDatum& d ) const;
  void set( const DictionaryDatum& d );
  double_t update( double_t t,
    double_t t_minus,
    double_t Ca_minus,
    double_t z,
    double_t tau_Ca,
    double_t growth_rate ) const;

private:
  double_t eta_;
  double_t eps_;
};

} // of namespace

#endif
