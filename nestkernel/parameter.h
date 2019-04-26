/*
 *  parameter.h
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

#ifndef PARAMETER_H_
#define PARAMETER_H_

// C++ includes:
#include <limits>
#include <math.h>

// Includes from librandom:
#include "normal_randomdev.h"
#include "randomgen.h"

// Includes from nestkernel:
#include "nest_names.h"
#include "nest_types.h"
#include "nestmodule.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{

/**
 * Abstract base class for parameters
 */
class Parameter
{
public:
  /**
   * Default constructor
   */
  Parameter()
  // : cutoff_( -std::numeric_limits< double >::infinity() )
  {
  }

  // /**
  //  * Constructor
  //  * @param cutoff Values less than the cutoff are set to zero.
  //  */
  // Parameter( /* double cutoff */ )
  // // : cutoff_( cutoff )
  // {
  // }

  /**
   * Constructor
   * Parameter that can be set in the Dictionary:
   *  cutoff - Values less than the cutoff are set to zero.
   * @param d dictionary with parameter values
   */
  Parameter( const DictionaryDatum& d )
  {
    // updateValue< double >( d, names::cutoff, cutoff_ );
  }

  /**
   * Virtual destructor
   */
  virtual ~Parameter()
  {
  }

  /**
   * @returns the value of the parameter.
   */
  virtual double value( librandom::RngPtr& rng ) const = 0;
  // {
  // return raw_value();
  // double val = raw_value( p, rng );
  // if ( val < cutoff_ )
  // {
  //   return 0.0;
  // }
  // else
  // {
  //   return val;
  // }
  // }

  /**
   * Clone method.
   * @returns dynamically allocated copy of parameter object
   */
  virtual Parameter* clone() const = 0;

  /**
   * Create the product of this parameter with another.
   * @returns a new dynamically allocated parameter.
   */
  virtual Parameter* multiply_parameter( const Parameter& other ) const;
  /**
   * Create the quotient of this parameter with another.
   * @returns a new dynamically allocated parameter.
   */
  virtual Parameter* divide_parameter( const Parameter& other ) const;
  /**
   * Create the sum of this parameter with another.
   * @returns a new dynamically allocated parameter.
   */
  virtual Parameter* add_parameter( const Parameter& other ) const;
  /**
   * Create the difference of this parameter with another.
   * @returns a new dynamically allocated parameter.
   */
  virtual Parameter* subtract_parameter( const Parameter& other ) const;

  // private:
  // double cutoff_;
};

/**
 * Parameter with constant value.
 */
class ConstantParameter : public Parameter
{
public:
  ConstantParameter( double value )
    : Parameter()
    , value_( value )
  {
  }

  /**
   * Parameters:
   * value - constant value of this parameter
   */
  ConstantParameter( const DictionaryDatum& d )
    : Parameter( d )
  {
    value_ = getValue< double >( d, "value" );
  }

  ~ConstantParameter()
  {
  }

  /**
   * @returns the constant value of this parameter.
   */
  double
  value( librandom::RngPtr& ) const
  {
    return value_;
  }

  Parameter*
  clone() const
  {
    return new ConstantParameter( value_ );
  }

private:
  double value_;
};


/**
 * Random parameter with uniform distribution in [min,max)
 */
class UniformParameter : public Parameter
{
public:
  /**
   * Parameters:
   * min - minimum value
   * max - maximum value
   */
  UniformParameter( const DictionaryDatum& d )
    : Parameter( d )
    , lower_( 0.0 )
    , range_( 1.0 )
  {
    updateValue< double >( d, names::min, lower_ );
    updateValue< double >( d, names::max, range_ );
    if ( lower_ >= range_ )
    {
      throw BadProperty(
        "topology::UniformParameter: "
        "min < max required." );
    }

    range_ -= lower_;
  }

  double
  value( librandom::RngPtr& rng ) const
  {
    return lower_ + rng->drand() * range_;
  }

  Parameter*
  clone() const
  {
    return new UniformParameter( *this );
  }

private:
  double lower_, range_;
};


/**
 * Random parameter with normal distribution, optionally truncated to [min,max).
 * Truncation is implemented by rejection.
 */
class NormalParameter : public Parameter
{
public:
  /**
   * Parameters:
   * mean  - mean value
   * sigma - standard distribution
   * min   - minimum value
   * max   - maximum value
   */
  NormalParameter( const DictionaryDatum& d )
    : Parameter( d )
    , mean_( 0.0 )
    , sigma_( 1.0 )
    , min_( -std::numeric_limits< double >::infinity() )
    , max_( std::numeric_limits< double >::infinity() )
    , rdev()
  {
    updateValue< double >( d, names::mean, mean_ );
    updateValue< double >( d, names::sigma, sigma_ );
    updateValue< double >( d, names::min, min_ );
    updateValue< double >( d, names::max, max_ );
    if ( sigma_ <= 0 )
    {
      throw BadProperty(
        "topology::NormalParameter: "
        "sigma > 0 required." );
    }
    if ( min_ >= max_ )
    {
      throw BadProperty(
        "topology::NormalParameter: "
        "min < max required." );
    }
  }

  double
  value( librandom::RngPtr& rng ) const
  {
    double val;
    do
    {
      val = mean_ + rdev( rng ) * sigma_;
    } while ( ( val < min_ ) or ( val >= max_ ) );
    return val;
  }

  Parameter*
  clone() const
  {
    return new NormalParameter( *this );
  }

private:
  double mean_, sigma_, min_, max_;
  librandom::NormalRandomDev rdev;
};


/**
 * Random parameter with lognormal distribution, optionally truncated to
 * [min,max). Truncation is implemented by rejection.
 */
class LognormalParameter : public Parameter
{
public:
  /**
   * Parameters:
   * mu    - mean value of logarithm
   * sigma - standard distribution of logarithm
   * min   - minimum value
   * max   - maximum value
   */
  LognormalParameter( const DictionaryDatum& d )
    : Parameter( d )
    , mu_( 0.0 )
    , sigma_( 1.0 )
    , min_( -std::numeric_limits< double >::infinity() )
    , max_( std::numeric_limits< double >::infinity() )
    , rdev()
  {
    updateValue< double >( d, names::mu, mu_ );
    updateValue< double >( d, names::sigma, sigma_ );
    updateValue< double >( d, names::min, min_ );
    updateValue< double >( d, names::max, max_ );
    if ( sigma_ <= 0 )
    {
      throw BadProperty(
        "topology::LognormalParameter: "
        "sigma > 0 required." );
    }
    if ( min_ >= max_ )
    {
      throw BadProperty(
        "topology::LognormalParameter: "
        "min < max required." );
    }
  }

  double
  value( librandom::RngPtr& rng ) const
  {
    double val;
    do
    {
      val = std::exp( mu_ + rdev( rng ) * sigma_ );
    } while ( ( val < min_ ) or ( val >= max_ ) );
    return val;
  }

  Parameter*
  clone() const
  {
    return new LognormalParameter( *this );
  }

private:
  double mu_, sigma_, min_, max_;
  librandom::NormalRandomDev rdev;
};


/**
 * Parameter class representing the product of two parameters
 */
class ProductParameter : public Parameter
{
public:
  /**
   * Construct the product of the two given parameters. Copies are made
   * of the supplied Parameter objects.
   */
  ProductParameter( const Parameter& m1, const Parameter& m2 )
    : Parameter()
    , parameter1_( m1.clone() )
    , parameter2_( m2.clone() )
  {
  }

  /**
   * Copy constructor.
   */
  ProductParameter( const ProductParameter& p )
    : Parameter( p )
    , parameter1_( p.parameter1_->clone() )
    , parameter2_( p.parameter2_->clone() )
  {
  }

  ~ProductParameter()
  {
    delete parameter1_;
    delete parameter2_;
  }

  /**
   * @returns the value of the product.
   */
  double
  value( librandom::RngPtr& rng ) const
  {
    return parameter1_->value( rng ) * parameter2_->value( rng );
  }

  Parameter*
  clone() const
  {
    return new ProductParameter( *this );
  }

protected:
  Parameter* parameter1_, *parameter2_;
};

/**
 * Parameter class representing the quotient of two parameters
 */
class QuotientParameter : public Parameter
{
public:
  /**
   * Construct the quotient of the two given parameters. Copies are made
   * of the supplied Parameter objects.
   */
  QuotientParameter( const Parameter& m1, const Parameter& m2 )
    : Parameter()
    , parameter1_( m1.clone() )
    , parameter2_( m2.clone() )
  {
  }

  /**
   * Copy constructor.
   */
  QuotientParameter( const QuotientParameter& p )
    : Parameter( p )
    , parameter1_( p.parameter1_->clone() )
    , parameter2_( p.parameter2_->clone() )
  {
  }

  ~QuotientParameter()
  {
    delete parameter1_;
    delete parameter2_;
  }

  /**
   * @returns the value of the product.
   */
  double
  value( librandom::RngPtr& rng ) const
  {
    return parameter1_->value( rng ) / parameter2_->value( rng );
  }

  Parameter*
  clone() const
  {
    return new QuotientParameter( *this );
  }

protected:
  Parameter* parameter1_, *parameter2_;
};

/**
 * Parameter class representing the sum of two parameters
 */
class SumParameter : public Parameter
{
public:
  /**
   * Construct the sum of the two given parameters. Copies are made
   * of the supplied Parameter objects.
   */
  SumParameter( const Parameter& m1, const Parameter& m2 )
    : Parameter()
    , parameter1_( m1.clone() )
    , parameter2_( m2.clone() )
  {
  }

  /**
   * Copy constructor.
   */
  SumParameter( const SumParameter& p )
    : Parameter( p )
    , parameter1_( p.parameter1_->clone() )
    , parameter2_( p.parameter2_->clone() )
  {
  }

  ~SumParameter()
  {
    delete parameter1_;
    delete parameter2_;
  }

  /**
   * @returns the value of the sum.
   */
  double
  value( librandom::RngPtr& rng ) const
  {
    return parameter1_->value( rng ) + parameter2_->value( rng );
  }

  Parameter*
  clone() const
  {
    return new SumParameter( *this );
  }

protected:
  Parameter* parameter1_, *parameter2_;
};

/**
 * Parameter class representing the difference of two parameters
 */
class DifferenceParameter : public Parameter
{
public:
  /**
   * Construct the difference of the two given parameters. Copies are made
   * of the supplied Parameter objects.
   */
  DifferenceParameter( const Parameter& m1, const Parameter& m2 )
    : Parameter()
    , parameter1_( m1.clone() )
    , parameter2_( m2.clone() )
  {
  }

  /**
   * Copy constructor.
   */
  DifferenceParameter( const DifferenceParameter& p )
    : Parameter( p )
    , parameter1_( p.parameter1_->clone() )
    , parameter2_( p.parameter2_->clone() )
  {
  }

  ~DifferenceParameter()
  {
    delete parameter1_;
    delete parameter2_;
  }

  /**
   * @returns the value of the difference.
   */
  double
  value( librandom::RngPtr& rng ) const
  {
    return parameter1_->value( rng ) - parameter2_->value( rng );
  }

  Parameter*
  clone() const
  {
    return new DifferenceParameter( *this );
  }

protected:
  Parameter* parameter1_, *parameter2_;
};

/**
 * Parameter class for a parameter oriented in the opposite direction.
 */
class ConverseParameter : public Parameter
{
public:
  /**
   * Construct the converse of the given parameter. A copy is made of the
   * supplied Parameter object.
   */
  ConverseParameter( const Parameter& p )
    : Parameter( p )
    , p_( p.clone() )
  {
  }

  /**
   * Copy constructor.
   */
  ConverseParameter( const ConverseParameter& p )
    : Parameter( p )
    , p_( p.p_->clone() )
  {
  }

  ~ConverseParameter()
  {
    delete p_;
  }

  /**
   * @returns the value of the parameter.
   */
  double
  value( librandom::RngPtr& rng ) const
  {
    return p_->value( rng );
  }

  Parameter*
  clone() const
  {
    return new ConverseParameter( *this );
  }

protected:
  Parameter* p_;
};

inline Parameter*
Parameter::multiply_parameter( const Parameter& other ) const
{
  return new ProductParameter( *this, other );
}

inline Parameter*
Parameter::divide_parameter( const Parameter& other ) const
{
  return new QuotientParameter( *this, other );
}

inline Parameter*
Parameter::add_parameter( const Parameter& other ) const
{
  return new SumParameter( *this, other );
}

inline Parameter*
Parameter::subtract_parameter( const Parameter& other ) const
{
  return new DifferenceParameter( *this, other );
}


} // namespace nest

#endif
