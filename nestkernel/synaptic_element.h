/*
 *  synaptic_element.h
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
 * \file synaptic_element.h
 * Definition of SynapticElement which is capable of
 * managing a synaptic element of a node.
 * \author Mikael Naveau
 * \date July 2013
 */

#ifndef SYNAPTIC_ELEMENT_H
#define SYNAPTIC_ELEMENT_H

#include "dictdatum.h"
#include "dictutils.h"
#include "histentry.h"

namespace nest
{

class SynapticElement;
class Archiving_Node;

/**
 * \class GrowthCurve
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
    double_t z_minus,
    double_t tau_Ca,
    double_t growth_rate ) const = 0;
  virtual bool
  is( std::string n )
  {
    return !n.compare( name );
  }
  std::string
  get_name()
  {
    return name;
  }

protected:
  std::string name;
};

/**
 * \class GrowthCurveLinear
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
    double_t z_minus,
    double_t tau_Ca,
    double_t growth_rate ) const;

private:
  double_t eps;
};

/**
 * \class GrowthCurveGaussian
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
    double_t z_minus,
    double_t tau_Ca,
    double_t growth_rate ) const;

private:
  double_t eta;
  double_t eps;
};


/**
 * \class SynapticElement
 * Synaptic element of a node (like Axon or dendrite) for the purposes
 * of synaptic plasticity
 */
class SynapticElement
{

public:
  /**
  * \fn SynapticElement()
  * Constructor.
  */
  SynapticElement();

  /**
  * \fn SynapticElement(const SynapticElement& se)
  * Copy Constructor.
  * @param se SynapticElement
  */
  SynapticElement( const SynapticElement& se );

  /**
  * \fn SynapticElement(const SynapticElement& se)
  * copy assignment operator.
  * @param other SynapticElement
  */
  SynapticElement& operator=( const SynapticElement& other );

  /**
  * \fn SynapticElement()
  * Destructor.
  */
  ~SynapticElement()
  {
    delete growth_curve_;
  }

  /**
   * \fn GrowthCurve* new_growth_curve(std::string name);
   * @param name linear, gaussian
   * @return pointer to a new GrowthCurve
   */
  GrowthCurve* new_growth_curve( std::string name );

  /**
  * \fn void get(DictionaryDatum&) const
  * Store current values in a dictionary.
  * @param d to write data
  */
  void get( DictionaryDatum& d ) const;

  /**
  * \fn void set(const DictionaryDatum&)
  * Set values from a dictionary.
  * @param d to take data from
  */
  void set( const DictionaryDatum& d );

  /**
  * \fn double_t get_z_value(Archiving_Node const *a, double_t t) const
  * Get the number of synaptic_element at the time t (in ms)
  * @param a node of this synaptic_element
  * @param t Current time (in ms)
  */
  void update( double_t t, double_t t_minus, double_t Ca_minus, double_t tau_Ca );
  int_t
  get_z_vacant() const
  {
    return std::floor( z_minus_ ) - z_connected_;
  }
  int_t
  get_z_connected() const
  {
    return z_connected_;
  }
  void
  connect( int_t n )
  {
    z_connected_ += n;
  }

  void
  set_growth_curve( GrowthCurve* g )
  {
    delete growth_curve_;
    growth_curve_ = g;
  }

  double_t
  get_growth_rate() const
  {
    return growth_rate_;
  }

  void
  set_z_minus( const double_t z )
  {
    z_minus_ = z;
  }
  double_t
  get_z_minus() const
  {
    return z_minus_;
  }

  bool
  continuous() const
  {
    return continuous_;
  }

private:
  double_t z_minus_;
  double_t z_minus_t_;
  int_t z_connected_;
  bool continuous_;
  double_t growth_rate_;
  double_t tau_vacant_;
  GrowthCurve* growth_curve_;
};

} // of namespace

#endif
