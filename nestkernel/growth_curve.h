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

// Includes from nestkernel:
#include "nest_types.h"
#include "exceptions.h"

// Includes from sli:
#include "dictdatum.h"

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
  virtual double
  update( double t, double t_minus, double Ca_minus, double z, double tau_Ca, double growth_rate ) const = 0;
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

/** @BeginDocumentation

  Name: growth_curve_linear - Linear version of a growth curve

  Description:
   This class represents a linear growth rule for the number of synaptic
   elements inside a neuron. The creation and deletion of synaptic elements
   when structural plasticity is enabled, allows the dynamic rewiring of the
   network during the simulation.
   This type of growth curve uses an exact integration method to update the
   number of synaptic elements: dz/dt = nu (1 - (1/eps) * Ca(t)), where nu is
   the growth rate [elements/ms] and eps is the desired average calcium
   concentration. The growth rate nu is defined in the SynapticElement class.

  Parameters:
   eps          double -  The target calcium concentration that
                          the neuron should look to achieve by creating or
                          deleting synaptic elements. It should always be a
                          positive value.  It is important to note that the
                          calcium concentration is linearly proportional to the
                          firing rate. This is because dCa/dt = - Ca(t)/tau_Ca
                          + beta_Ca if the neuron fires and dCa/dt = -
                          Ca(t)/tau_Ca otherwise, where tau_Ca is the calcium
                          concentration decay constant and beta_Ca is the
                          calcium intake constant (see SynapticElement class).
                          This means that eps also defines the desired firing
                          rate that the neuron should achieve.  For example, an
                          eps = 0.05 [Ca2+] with tau_Ca = 10000.0 and beta_Ca =
                          0.001 for a synaptic element means a desired firing
                          rate of 5Hz.

  References:
   [1] Butz, Markus, Florentin Wörgötter, and Arjen van Ooyen.
   "Activity-dependent structural plasticity." Brain research reviews 60.2
   (2009): 287-305.

   [2] Butz, Markus, and Arjen van Ooyen. "A simple rule for dendritic spine
   and axonal bouton formation can account for cortical reorganization after
   focal retinal lesions." PLoS Comput Biol 9.10 (2013): e1003259.

  FirstVersion: July 2013

  Author: Mikael Naveau

  SeeAlso: SynapticElement, SPManager, SPBuilder, GrowthCurveLinear,
           GrowthCurveGaussian
*/
/**
 * \class GrowthCurveLinear
 * Uses an exact integration method to update the number of synaptic elements:
 * dz/dt = nu (1 - (1/eps) * Ca(t)), where nu is the growth rate and
 * eps is the desired average calcium concentration.
 */
class GrowthCurveLinear : public GrowthCurve
{
public:
  GrowthCurveLinear();
  void get( DictionaryDatum& d ) const;
  void set( const DictionaryDatum& d );
  double update( double t, double t_minus, double Ca_minus, double z, double tau_Ca, double growth_rate ) const;

private:
  double eps_;
};

/** @BeginDocumentation
  Name: growth_curve_gaussian - Gaussian version of a growth curve

  Description:
   This class represents a Gaussian growth rule for the number of synaptic
   elements inside a neuron. The creation and deletion of synaptic elements
   when structural plasticity is enabled, allows the dynamic rewiring of the
   network during the simulation.
   This type of growth curve  uses a forward Euler integration method to update
   the number of synaptic elements:
   dz/dt = nu (2 * e^(- ((Ca(t) - xi)/z)^2 ) - 1)
   where xi = (eta  + eps)/2,
   zeta = (eps - eta)/2 * sqrt(ln(2))),
   eta is the minimum calcium concentration required for any synaptic element
   to be created, eps is the target mean calcium concentration in the neuron
   and nu is the growth rate in elements/ms. The growth rate nu is defined in
   the SynapticElement class.

  Parameters:
   eta          double -  Minimum amount of calcium concentration that the
                          neuron needs to start creating synaptic elements.
                          eta can have a negative value, making the growth
                          curve move its maximum to the left. For example, if
                          eta=-0.5 and eps=0.5 [Ca2+], the maximum growth rate
                          (elements/ms) will be achieved at 0.0 [Ca2+]. If
                          eta=0.0 [Ca2+] and eps=0.5 [Ca2+] the maximum growth
                          rate will be achieved at 0.25 [Ca2+] while at 0.0
                          [Ca+2] no new elements will be created.

   eps          double -  The target calcium concentration that
                          the neuron should look to achieve by creating or
                          deleting synaptic elements. It should always be a
                          positive value.  It is important to note that the
                          calcium concentration is linearly proportional to the
                          firing rate. This is because dCa/dt = - Ca(t)/tau_Ca
                          + beta_Ca if the neuron fires and dCa/dt = -
                          Ca(t)/tau_Ca otherwise, where tau_Ca is the calcium
                          concentration decay constant and beta_Ca is the
                          calcium intake constant (see SynapticElement class).
                          This means that eps can also be seen as the desired
                          firing rate that the neuron should achieve.  For
                          example, an eps = 0.05 [Ca2+] with tau_Ca = 10000.0
                          and beta_Ca = 0.001 for a synaptic element means a
                          desired firing rate of 5Hz.

   nu           double -  Growth rate in elements/ms. The growth rate nu is
                          defined in the SynapticElement class. Can be negative.

  References:
   [1] Butz, Markus, Florentin Wörgötter, and Arjen van Ooyen.
   "Activity-dependent structural plasticity." Brain research reviews 60.2
   (2009): 287-305.

   [2] Butz, Markus, and Arjen van Ooyen. "A simple rule for dendritic spine
   and axonal bouton formation can account for cortical reorganization after
   focal retinal lesions." PLoS Comput Biol 9.10 (2013): e1003259.

  FirstVersion: July 2013

  Author: Mikael Naveau

  SeeAlso: SynapticElement, SPManager, SPBuilder, GrowthCurveLinear,
           GrowthCurveGaussian
*/
/**
 * \class GrowthCurveGaussian
 * Uses a forward Euler integration method to update the number of synaptic
 * elements:
 * dz/dt = nu (2 * e^(- ((Ca(t) - xi)/zeta)^2 ) - 1)
 * where xi = (eta  + eps)/2,
 * zeta = (eps - eta)/2 * sqrt(ln(2))),
 * eta is the minimum calcium concentration required for any synaptic element
 * to be created, eps is the target mean calcium concentration in the
 * neuron and nu is the growth rate.
 */
class GrowthCurveGaussian : public GrowthCurve
{
public:
  GrowthCurveGaussian();
  void get( DictionaryDatum& d ) const;
  void set( const DictionaryDatum& d );
  double update( double t, double t_minus, double Ca_minus, double z, double tau_Ca, double growth_rate ) const;

private:
  double eta_;
  double eps_;
};

/** @BeginDocumentation
  Name: growth_curve_sigmoid - Sigmoid version of a growth curve

  Description:
   This class represents a Sigmoid growth rule for the number of synaptic
   elements inside a neuron. The creation and deletion of synaptic elements
   when structural plasticity is enabled, allows the dynamic rewiring of the
   network during the simulation.
   This type of growth curve  uses a forward Euler integration method to update
   the number of synaptic elements:
   dz/dt = nu ((2 / (1 + e^((Ca(t) - eps)/psi))) - 1)
   eps is the target mean calcium concentration in the
   neuron, psi controls the width of the sigmoid and nu is the growth rate in
   elements/ms. The growth rate nu is defined in the SynapticElement class.

  Parameters:
   eps          double -  The target calcium concentration that
                          the neuron should look to achieve by creating or
                          deleting synaptic elements. It should always be a
                          positive value.  It is important to note that the
                          calcium concentration is linearly proportional to the
                          firing rate. This is because dCa/dt = - Ca(t)/tau_Ca
                          + beta_Ca if the neuron fires and dCa/dt = -
                          Ca(t)/tau_Ca otherwise, where tau_Ca is the calcium
                          concentration decay constant and beta_Ca is the
                          calcium intake constant (see SynapticElement class).
                          This means that eps can also be seen as the desired
                          firing rate that the neuron should achieve.  For
                          example, an eps = 0.05 [Ca2+] with tau_Ca = 10000.0
                          and beta_Ca = 0.001 for a synaptic element means a
                          desired firing rate of 5Hz.

   nu           double -  Growth rate in elements/ms. The growth rate nu is
                          defined in the SynapticElement class. Can be negative.

   psi          double -  Parameter that controls the width of the curve.
                          Must be greater than 0

  References:
   [1] Butz, Markus, Steenbuck, Ines D., and Arjen van Ooyen.
   "Homeostatic structural plasticity increases the efficiency of small-world
   networks." Frontiers in Synaptic Neuroscience 6 (2014): 7.

  FirstVersion: September 2016

  Author: Ankur Sinha

  SeeAlso: SynapticElement, SPManager, SPBuilder, GrowthCurveLinear,
           GrowthCurveSigmoid
*/
/**
 * \class GrowthCurveSigmoid
 * Uses a forward Euler integration method to update the number of synaptic
 * elements:
 * dz/dt = nu ((2 / (1 + e^((Ca(t) - eps)/psi))) - 1)
 * eps is the target mean calcium concentration in the
 * neuron, psi controls the width of the sigmoid
 * and nu is the growth rate.
 */
class GrowthCurveSigmoid : public GrowthCurve
{
public:
  GrowthCurveSigmoid();
  void get( DictionaryDatum& d ) const;
  void set( const DictionaryDatum& d );
  double update( double t, double t_minus, double Ca_minus, double z, double tau_Ca, double growth_rate ) const;

private:
  double eps_;
  double psi_;
};

} // of namespace

#endif
