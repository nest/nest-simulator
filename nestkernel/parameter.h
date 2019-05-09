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
  {
  }

  /**
   * Constructor
   * Parameter that can be set in the Dictionary:
   * @param d dictionary with parameter values
   */
  Parameter( const DictionaryDatum& d )
  {
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
  virtual double value( librandom::RngPtr& rng, Node* node ) const = 0;
  virtual double
  value( librandom::RngPtr& rng, Node* source, Node* target ) const = 0;

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
  /**
   * Create the exponential of this parameter.
   * @returns a new dynamically allocated parameter.
   */
  virtual Parameter* exp() const;
  /**
   * Create the sine of this parameter.
   * @returns a new dynamically allocated parameter.
   */
  virtual Parameter* sin() const;
  /**
   * Create the cosine of this parameter.
   * @returns a new dynamically allocated parameter.
   */
  virtual Parameter* cos() const;
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
  value( librandom::RngPtr&, Node* ) const
  {
    return value_;
  }

  double
  value( librandom::RngPtr& rng, Node* source, Node* ) const
  {
    return value( rng, source );
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
        "nest::UniformParameter: "
        "min < max required." );
    }

    range_ -= lower_;
  }

  double
  value( librandom::RngPtr& rng, Node* ) const
  {
    return lower_ + rng->drand() * range_;
  }

  double
  value( librandom::RngPtr& rng, Node* source, Node* ) const
  {
    return value( rng, source );
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
        "nest::NormalParameter: "
        "sigma > 0 required." );
    }
    if ( min_ >= max_ )
    {
      throw BadProperty(
        "nest::NormalParameter: "
        "min < max required." );
    }
  }

  double
  value( librandom::RngPtr& rng, Node* ) const
  {
    double val;
    do
    {
      val = mean_ + rdev( rng ) * sigma_;
    } while ( ( val < min_ ) or ( val >= max_ ) );
    return val;
  }

  double
  value( librandom::RngPtr& rng, Node* source, Node* ) const
  {
    return value( rng, source );
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
        "nest::LognormalParameter: "
        "sigma > 0 required." );
    }
    if ( min_ >= max_ )
    {
      throw BadProperty(
        "nest::LognormalParameter: "
        "min < max required." );
    }
  }

  double
  value( librandom::RngPtr& rng, Node* ) const
  {
    double val;
    do
    {
      val = std::exp( mu_ + rdev( rng ) * sigma_ );
    } while ( ( val < min_ ) or ( val >= max_ ) );
    return val;
  }

  double
  value( librandom::RngPtr& rng, Node* source, Node* ) const
  {
    return value( rng, source );
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
 * Exponential parameter.
 */
class ExponentialParameter : public Parameter
{
public:
  /**
   * Parameters:
   * scale - the scale parameter
   */
  ExponentialParameter( const DictionaryDatum& d )
    : Parameter( d )
    , scale_( 1.0 )
  {
    updateValue< double >( d, names::scale, scale_ );
  }

  double
  value( librandom::RngPtr& rng, Node* ) const
  {
    return scale_ * ( -std::log( 1 - rng->drand() ) );
  }

  double
  value( librandom::RngPtr& rng, Node* source, Node* ) const
  {
    return value( rng, source );
  }

  Parameter*
  clone() const
  {
    return new ExponentialParameter( *this );
  }

private:
  double scale_;
};


/**
 * Node position parameter.
 */
class NodePosParameter : public Parameter
{
public:
  /**
   * Parameters:
   * dimension - Dimension from which to get the position value of the node.
   * type_id - If specified, specifies if the position should be taken from the
   *           presynaptic or postsynaptic node in a connection.
   *           0: unspecified, 1: presynaptic, 2: postsynaptic.
   */
  NodePosParameter( const DictionaryDatum& d )
    : Parameter( d )
    , dimension_( 0 )
    , node_location_( 0 )
  {
    updateValue< long >( d, names::dimension, dimension_ );
    updateValue< long >(
      d, names::type_id, node_location_ ); // TODO: Better name than "type_id"?
    if ( node_location_ < 0 or 2 < node_location_ )
    {
      throw BadParameterValue(
        "Node location must either be unspecified (0), source (1) or target "
        "(2)" );
    }
  }

  double
  value( librandom::RngPtr& rng, Node* node ) const
  {
    if ( node_location_ != 0 )
    {
      throw BadParameterValue(
        "Source or target position parameter can only be used when "
        "connecting." );
    }
    return get_node_pos_( rng, node );
  }
  double
  value( librandom::RngPtr& rng, Node* source, Node* target ) const
  {
    switch ( node_location_ )
    {
    case 0:
      throw BadParameterValue(
        "Node position parameter cannot be used when connecting." );
    case 1:
      return get_node_pos_( rng, source );
    case 2:
      return get_node_pos_( rng, target );
    }
    // TODO: assert that we don't get here
    throw KernelException(
        "Wrong node_location_." );
  }

  Parameter*
  clone() const
  {
    return new NodePosParameter( *this );
  }

private:
  int dimension_;
  int node_location_;

  double get_node_pos_( librandom::RngPtr& rng, Node* node ) const;
};


/**
 * Node distance parameter.
 */
class SpatialDistanceParameter : public Parameter
{
public:
  SpatialDistanceParameter( const DictionaryDatum& d )
    : Parameter( d )
  {
  }

  double
  value( librandom::RngPtr& rng, Node* ) const
  {
    throw BadParameterValue(
      "Spatial distance parameter can only be used when connecting." );
  }

  double value( librandom::RngPtr&, Node*, Node* ) const;

  Parameter*
  clone() const
  {
    return new SpatialDistanceParameter( *this );
  }
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
  value( librandom::RngPtr& rng, Node* node ) const
  {
    return parameter1_->value( rng, node ) * parameter2_->value( rng, node );
  }

  double
  value( librandom::RngPtr& rng, Node* source, Node* target ) const
  {
    return parameter1_->value( rng, source, target )
      * parameter2_->value( rng, source, target );
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
  value( librandom::RngPtr& rng, Node* node ) const
  {
    return parameter1_->value( rng, node ) / parameter2_->value( rng, node );
  }

  double
  value( librandom::RngPtr& rng, Node* source, Node* target ) const
  {
    return parameter1_->value( rng, source, target )
      / parameter2_->value( rng, source, target );
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
  value( librandom::RngPtr& rng, Node* node ) const
  {
    return parameter1_->value( rng, node ) + parameter2_->value( rng, node );
  }

  double
  value( librandom::RngPtr& rng, Node* source, Node* target ) const
  {
    return parameter1_->value( rng, source, target )
      + parameter2_->value( rng, source, target );
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
  value( librandom::RngPtr& rng, Node* node ) const
  {
    return parameter1_->value( rng, node ) - parameter2_->value( rng, node );
  }

  double
  value( librandom::RngPtr& rng, Node* source, Node* target ) const
  {
    return parameter1_->value( rng, source, target )
      - parameter2_->value( rng, source, target );
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
  value( librandom::RngPtr& rng, Node* node ) const
  {
    return p_->value( rng, node );
  }

  double
  value( librandom::RngPtr& rng, Node* source, Node* target ) const
  {
    return p_->value( rng, source, target );
  }

  Parameter*
  clone() const
  {
    return new ConverseParameter( *this );
  }

protected:
  Parameter* p_;
};

/**
 * Parameter class representing the exponential of a parameter.
 */
class ExpParameter : public Parameter
{
public:
  /**
   * Construct the exponential of the given parameter. A copy is made of the
   * supplied Parameter object.
   */
  ExpParameter( const Parameter& p )
    : Parameter( p )
    , p_( p.clone() )
  {
  }

  /**
   * Copy constructor.
   */
  ExpParameter( const ExpParameter& p )
    : Parameter( p )
    , p_( p.p_->clone() )
  {
  }

  ~ExpParameter()
  {
    delete p_;
  }

  /**
   * @returns the value of the parameter.
   */
  double
  value( librandom::RngPtr& rng, Node* node ) const
  {
    return std::exp( p_->value( rng, node ) );
  }

  double
  value( librandom::RngPtr& rng, Node* source, Node* target ) const
  {
    return std::exp( p_->value( rng, source, target ) );
  }

  Parameter*
  clone() const
  {
    return new ExpParameter( *this );
  }

protected:
  Parameter* p_;
};


/**
 * Parameter class representing the sine of a parameter.
 */
class SinParameter : public Parameter
{
public:
  /**
   * Construct the sine of the given parameter. A copy is made of the
   * supplied Parameter object.
   */
  SinParameter( const Parameter& p )
    : Parameter( p )
    , p_( p.clone() )
  {
  }

  /**
   * Copy constructor.
   */
  SinParameter( const SinParameter& p )
    : Parameter( p )
    , p_( p.p_->clone() )
  {
  }

  ~SinParameter()
  {
    delete p_;
  }

  /**
   * @returns the value of the parameter.
   */
  double
  value( librandom::RngPtr& rng, Node* node ) const
  {
    return std::sin( p_->value( rng, node ) );
  }

  double
  value( librandom::RngPtr& rng, Node* source, Node* target ) const
  {
    return std::sin( p_->value( rng, source, target ) );
  }

  Parameter*
  clone() const
  {
    return new SinParameter( *this );
  }

protected:
  Parameter* p_;
};

/**
 * Parameter class representing the cosine of a parameter.
 */
class CosParameter : public Parameter
{
public:
  /**
   * Construct the exponential of the given parameter. A copy is made of the
   * supplied Parameter object.
   */
  CosParameter( const Parameter& p )
    : Parameter( p )
    , p_( p.clone() )
  {
  }

  /**
   * Copy constructor.
   */
  CosParameter( const CosParameter& p )
    : Parameter( p )
    , p_( p.p_->clone() )
  {
  }

  ~CosParameter()
  {
    delete p_;
  }

  /**
   * @returns the value of the parameter.
   */
  double
  value( librandom::RngPtr& rng, Node* node ) const
  {
    return std::cos( p_->value( rng, node ) );
  }

  double
  value( librandom::RngPtr& rng, Node* source, Node* target ) const
  {
    return std::cos( p_->value( rng, source, target ) );
  }

  Parameter*
  clone() const
  {
    return new CosParameter( *this );
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

inline Parameter*
Parameter::exp() const
{
  return new ExpParameter( *this );
}

inline Parameter*
Parameter::sin() const
{
  return new SinParameter( *this );
}

inline Parameter*
Parameter::cos() const
{
  return new CosParameter( *this );
}


} // namespace nest

#endif
