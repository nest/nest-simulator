/*
 *  topology_parameter.h
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

#ifndef TOPOLOGY_PARAMETER_H
#define TOPOLOGY_PARAMETER_H

// C++ includes:
#include <limits>
#include <math.h>

// Includes from librandom:
#include "normal_randomdev.h"
#include "randomgen.h"

// Includes from nestkernel:
#include "nest_names.h"
#include "nest_types.h"

// Includes from sli:
#include "dictdatum.h"
#include "dictutils.h"

// Includes from topology:
#include "position.h"
#include "topology_names.h"
#include "topologymodule.h"

namespace nest
{
class TopologyParameter;
typedef lockPTRDatum< TopologyParameter, &TopologyModule::TopologyParameterType >
  TopologyParameterDatum;

/**
 * Abstract base class for parameters
 */
class TopologyParameter
{
public:
  /**
   * Default constructor
   */
  TopologyParameter()
    : cutoff_( -std::numeric_limits< double >::infinity() )
  {
  }

  /**
   * Constructor
   * @param cutoff Values less than the cutoff are set to zero.
   */
  TopologyParameter( double cutoff )
    : cutoff_( cutoff )
  {
  }

  /**
   * Constructor
   * Parameter that can be set in the Dictionary:
   *  cutoff - Values less than the cutoff are set to zero.
   * @param d dictionary with parameter values
   */
  TopologyParameter( const DictionaryDatum& d )
    : cutoff_( -std::numeric_limits< double >::infinity() )
  {
    updateValue< double >( d, names::cutoff, cutoff_ );
  }

  /**
   * Virtual destructor
   */
  virtual ~TopologyParameter()
  {
  }

  /**
   * @returns the value of the parameter at the given point.
   */
  double
  value( const Position< 2 >& p, librandom::RngPtr& rng ) const
  {
    double val = raw_value( p, rng );
    if ( val < cutoff_ )
    {
      return 0.0;
    }
    else
    {
      return val;
    }
  }

  /**
   * @returns the value of the parameter at the given point.
   */
  double
  value( const Position< 3 >& p, librandom::RngPtr& rng ) const
  {
    double val = raw_value( p, rng );
    if ( val < cutoff_ )
    {
      return 0.0;
    }
    else
    {
      return val;
    }
  }

  /**
   * Raw value disregarding cutoff.
   * @returns the value of the parameter at the given point.
   */
  virtual double
  raw_value( const Position< 2 >&, librandom::RngPtr& ) const
  {
    throw KernelException( "Parameter not valid for 2D layer" );
  }

  /**
   * Raw value disregarding cutoff.
   * @returns the value of the parameter at the given point.
   */
  virtual double
  raw_value( const Position< 3 >&, librandom::RngPtr& ) const
  {
    throw KernelException( "Parameter not valid for 3D layer" );
  }

  /**
   * @returns the value of the parameter at the given point.
   */
  double value( const std::vector< double >& pt, librandom::RngPtr& rng ) const;

  /**
   * Clone method.
   * @returns dynamically allocated copy of parameter object
   */
  virtual TopologyParameter* clone() const = 0;

  /**
   * Create the product of this parameter with another.
   * @returns a new dynamically allocated parameter.
   */
  virtual TopologyParameter* multiply_topology_parameter(
    const TopologyParameter& other ) const;
  /**
   * Create the quotient of this parameter with another.
   * @returns a new dynamically allocated parameter.
   */
  virtual TopologyParameter* divide_topology_parameter(
    const TopologyParameter& other ) const;
  /**
   * Create the sum of this parameter with another.
   * @returns a new dynamically allocated parameter.
   */
  virtual TopologyParameter* add_topology_parameter(
    const TopologyParameter& other ) const;
  /**
   * Create the difference of this parameter with another.
   * @returns a new dynamically allocated parameter.
   */
  virtual TopologyParameter* subtract_topology_parameter(
    const TopologyParameter& other ) const;

private:
  double cutoff_;
};

/**
 * Parameter with constant value.
 */
class ConstantTopologyParameter : public TopologyParameter
{
public:
  ConstantTopologyParameter( double value )
    : TopologyParameter()
    , value_( value )
  {
  }

  /**
   * Parameters:
   * value - constant value of this parameter
   */
  ConstantTopologyParameter( const DictionaryDatum& d )
    : TopologyParameter( d )
  {
    value_ = getValue< double >( d, "value" );
  }

  ~ConstantTopologyParameter()
  {
  }

  /**
   * @returns the constant value of this parameter.
   */
  double
  raw_value( const Position< 2 >&, librandom::RngPtr& ) const
  {
    return value_;
  }
  double
  raw_value( const Position< 3 >&, librandom::RngPtr& ) const
  {
    return value_;
  }

  TopologyParameter*
  clone() const
  {
    return new ConstantTopologyParameter( value_ );
  }

private:
  double value_;
};

/**
 * Abstract base class for parameters only depending on distance.
 */
class RadialTopologyParameter : public TopologyParameter
{
public:
  RadialTopologyParameter()
    : TopologyParameter()
  {
  }

  RadialTopologyParameter( double cutoff )
    : TopologyParameter( cutoff )
  {
  }

  RadialTopologyParameter( const DictionaryDatum& d )
    : TopologyParameter( d )
  {
  }

  virtual double raw_value( double ) const = 0;

  double
  raw_value( const Position< 2 >& p, librandom::RngPtr& ) const
  {
    return raw_value( p.length() );
  }
  double
  raw_value( const Position< 3 >& p, librandom::RngPtr& ) const
  {
    return raw_value( p.length() );
  }
};

/**
 * Linear (affine) parameter p(d) = c + a*d.
 */
class LinearTopologyParameter : public RadialTopologyParameter
{
public:
  /**
   * Parameters:
   * a - coefficient of linear function
   * c - constant offset
   */
  LinearTopologyParameter( const DictionaryDatum& d )
    : RadialTopologyParameter( d )
    , a_( 1.0 )
    , c_( 0.0 )
  {
    updateValue< double >( d, names::a, a_ );
    updateValue< double >( d, names::c, c_ );
  }

  double
  raw_value( double x ) const
  {
    return a_ * x + c_;
  }

  TopologyParameter*
  clone() const
  {
    return new LinearTopologyParameter( *this );
  }

private:
  double a_, c_;
};


/**
 * Exponential parameter p(d) = c + a*exp(-d/tau).
 */
class ExponentialTopologyParameter : public RadialTopologyParameter
{
public:
  /**
   * Parameters:
   * a   - coefficient of exponential term
   * tau - length scale factor
   * c   - constant offset
   */
  ExponentialTopologyParameter( const DictionaryDatum& d )
    : RadialTopologyParameter( d )
    , a_( 1.0 )
    , c_( 0.0 )
    , tau_( 1.0 )
  {
    updateValue< double >( d, names::a, a_ );
    updateValue< double >( d, names::c, c_ );
    updateValue< double >( d, names::tau, tau_ );
    if ( tau_ <= 0 )
    {
      throw BadProperty(
        "topology::ExponentialParameter: "
        "tau > 0 required." );
    }
  }

  double
  raw_value( double x ) const
  {
    return c_ + a_ * std::exp( -x / tau_ );
  }

  TopologyParameter*
  clone() const
  {
    return new ExponentialTopologyParameter( *this );
  }

private:
  double a_, c_, tau_;
};


/**
 * Gaussian parameter p(d) = c + p_center*exp(-(d-mean)^2/(2*sigma^2))
 */
class GaussianTopologyParameter : public RadialTopologyParameter
{
public:
  /**
   * Parameters:
   * c        - constant offset
   * p_center - value at center of gaussian
   * mean     - distance to center
   * sigma    - width of gaussian
   */
  GaussianTopologyParameter( const DictionaryDatum& d )
    : RadialTopologyParameter( d )
    , c_( 0.0 )
    , p_center_( 1.0 )
    , mean_( 0.0 )
    , sigma_( 1.0 )
  {
    updateValue< double >( d, names::c, c_ );
    updateValue< double >( d, names::p_center, p_center_ );
    updateValue< double >( d, names::mean, mean_ );
    updateValue< double >( d, names::sigma, sigma_ );
    if ( sigma_ <= 0 )
    {
      throw BadProperty(
        "topology::GaussianParameter: "
        "sigma > 0 required." );
    }
  }

  double
  raw_value( double x ) const
  {
    return c_
      + p_center_
      * std::exp( -std::pow( x - mean_, 2 ) / ( 2 * std::pow( sigma_, 2 ) ) );
  }

  TopologyParameter*
  clone() const
  {
    return new GaussianTopologyParameter( *this );
  }

private:
  double c_, p_center_, mean_, sigma_;
};


/**
 * Bivariate Gaussian parameter
 *  p(x,y) = c + p_center*exp( -( (x-mean_x)^2/sigma_x^2 +
 *                                (y-mean_y)^2/sigma_y^2 +
 *                                2*rho*(x-mean_x)*(y-mean_y)/(sigma_x*sigma_y)
 *                               ) / (2*(1-rho^2)) )
 */
class Gaussian2DTopologyParameter : public TopologyParameter
{
public:
  /**
   * Parameters:
   * c        - constant offset
   * p_center - value at center
   * mean_x   - x-coordinate of center
   * mean_y   - y-coordinate of center
   * sigma_x  - width in x-direction
   * sigma_y  - width in y-direction
   * rho      - correlation of x and y
   */
  Gaussian2DTopologyParameter( const DictionaryDatum& d );

  double
  raw_value( const Position< 2 >& pos, librandom::RngPtr& ) const
  {
    return c_
      + p_center_
      * std::exp(
          -( ( pos[ 0 ] - mean_x_ ) * ( pos[ 0 ] - mean_x_ )
              / ( sigma_x_ * sigma_x_ )
            + ( pos[ 1 ] - mean_y_ ) * ( pos[ 1 ] - mean_y_ )
              / ( sigma_y_ * sigma_y_ )
            - 2. * rho_ * ( pos[ 0 ] - mean_x_ ) * ( pos[ 1 ] - mean_y_ )
              / ( sigma_x_ * sigma_y_ ) ) / ( 2. * ( 1. - rho_ * rho_ ) ) );
  }

  double
  raw_value( const Position< 3 >& pos, librandom::RngPtr& rng ) const
  {
    return raw_value( Position< 2 >( pos[ 0 ], pos[ 1 ] ), rng );
  }

  TopologyParameter*
  clone() const
  {
    return new Gaussian2DTopologyParameter( *this );
  }

private:
  double c_, p_center_, mean_x_, sigma_x_, mean_y_, sigma_y_, rho_;
};

/**
 * Gamma parameter p(d) = d^(kappa-1)*exp(-d/theta)/(theta^kappa*Gamma(kappa))
 */
class GammaTopologyParameter : public RadialTopologyParameter
{
public:
  /**
   * Parameters:
   * kappa    - shape of gamma distribution
   * theta    - scale of gamma distribution
   */
  GammaTopologyParameter( const DictionaryDatum& d )
    : RadialTopologyParameter( d )
    , kappa_( 1.0 )
    , theta_( 1.0 )
    , inv_theta_( 1.0 / theta_ )
    , delta_( 1.0 ) // consistent, cannot be computed explicitly here
  {
    updateValue< double >( d, names::kappa, kappa_ );
    updateValue< double >( d, names::theta, theta_ );
    if ( kappa_ <= 0 )
    {
      throw BadProperty(
        "topology::GammaParameter: "
        "kappa > 0 required." );
    }
    if ( theta_ <= 0 )
    {
      throw BadProperty(
        "topology::GammaParameter: "
        "theta > 0 required." );
    }

    inv_theta_ = 1. / theta_;
    // TODO: tgamma() is available from math.h as C99 function,
    //       but was added to C++ only per C++11. Add std::
    //       once we convert NEST compilation to C++11.
    delta_ = std::pow( inv_theta_, kappa_ ) / tgamma( kappa_ );
  }

  double
  raw_value( double x ) const
  {
    return std::pow( x, kappa_ - 1. ) * std::exp( -1. * inv_theta_ * x )
      * delta_;
  }

  TopologyParameter*
  clone() const
  {
    return new GammaTopologyParameter( *this );
  }

private:
  double kappa_;
  double theta_;
  double inv_theta_;
  double delta_;
};


/**
 * Random parameter with uniform distribution in [min,max)
 */
class UniformTopologyParameter : public TopologyParameter
{
public:
public:
  /**
   * Parameters:
   * min - minimum value
   * max - maximum value
   */
  UniformTopologyParameter( const DictionaryDatum& d )
    : TopologyParameter( d )
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
  raw_value( const Position< 2 >&, librandom::RngPtr& rng ) const
  {
    return lower_ + rng->drand() * range_;
  }

  double
  raw_value( const Position< 3 >&, librandom::RngPtr& rng ) const
  {
    return lower_ + rng->drand() * range_;
  }

  TopologyParameter*
  clone() const
  {
    return new UniformTopologyParameter( *this );
  }

private:
  double lower_, range_;
};


/**
 * Random parameter with normal distribution, optionally truncated to [min,max).
 * Truncation is implemented by rejection.
 */
class NormalTopologyParameter : public TopologyParameter
{
public:
public:
  /**
   * Parameters:
   * mean  - mean value
   * sigma - standard distribution
   * min   - minimum value
   * max   - maximum value
   */
  NormalTopologyParameter( const DictionaryDatum& d )
    : TopologyParameter( d )
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
  raw_value( librandom::RngPtr& rng ) const
  {
    double val;
    do
    {
      val = mean_ + rdev( rng ) * sigma_;
    } while ( ( val < min_ ) or ( val >= max_ ) );
    return val;
  }

  double
  raw_value( const Position< 2 >&, librandom::RngPtr& rng ) const
  {
    return raw_value( rng );
  }

  double
  raw_value( const Position< 3 >&, librandom::RngPtr& rng ) const
  {
    return raw_value( rng );
  }

  TopologyParameter*
  clone() const
  {
    return new NormalTopologyParameter( *this );
  }

private:
  double mean_, sigma_, min_, max_;
  librandom::NormalRandomDev rdev;
};


/**
 * Random parameter with lognormal distribution, optionally truncated to
 * [min,max). Truncation is implemented by rejection.
 */
class LognormalTopologyParameter : public TopologyParameter
{
public:
public:
  /**
   * Parameters:
   * mu    - mean value of logarithm
   * sigma - standard distribution of logarithm
   * min   - minimum value
   * max   - maximum value
   */
  LognormalTopologyParameter( const DictionaryDatum& d )
    : TopologyParameter( d )
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
  raw_value( librandom::RngPtr& rng ) const
  {
    double val;
    do
    {
      val = std::exp( mu_ + rdev( rng ) * sigma_ );
    } while ( ( val < min_ ) or ( val >= max_ ) );
    return val;
  }

  double
  raw_value( const Position< 2 >&, librandom::RngPtr& rng ) const
  {
    return raw_value( rng );
  }

  double
  raw_value( const Position< 3 >&, librandom::RngPtr& rng ) const
  {
    return raw_value( rng );
  }

  TopologyParameter*
  clone() const
  {
    return new LognormalTopologyParameter( *this );
  }

private:
  double mu_, sigma_, min_, max_;
  librandom::NormalRandomDev rdev;
};


/**
 * Parameter class representing a parameter centered at an anchor position.
 */
template < int D >
class AnchoredTopologyParameter : public TopologyParameter
{
public:
  AnchoredTopologyParameter( const TopologyParameter& p, const Position< D >& anchor )
    : TopologyParameter( p )
    , p_( p.clone() )
    , anchor_( anchor )
  {
  }

  AnchoredTopologyParameter( const AnchoredTopologyParameter& p )
    : TopologyParameter( p )
    , p_( p.p_->clone() )
    , anchor_( p.anchor_ )
  {
  }

  ~AnchoredTopologyParameter()
  {
    delete p_;
  }

  double
  raw_value( const Position< D xor 1 >&, librandom::RngPtr& ) const
  {
    throw BadProperty( "Incorrect dimension." );
  }

  double
  raw_value( const Position< D >& p, librandom::RngPtr& rng ) const
  {
    return p_->raw_value( p - anchor_, rng );
  }

  TopologyParameter*
  clone() const
  {
    return new AnchoredTopologyParameter( *this );
  }

private:
  TopologyParameter* p_;
  Position< D > anchor_;
};

/**
 * Parameter class representing the product of two parameters
 */
class ProductTopologyParameter : public TopologyParameter
{
public:
  /**
   * Construct the product of the two given parameters. Copies are made
   * of the supplied Parameter objects.
   */
  ProductTopologyParameter( const TopologyParameter& m1, const TopologyParameter& m2 )
    : TopologyParameter()
    , parameter1_( m1.clone() )
    , parameter2_( m2.clone() )
  {
  }

  /**
   * Copy constructor.
   */
  ProductTopologyParameter( const ProductTopologyParameter& p )
    : TopologyParameter( p )
    , parameter1_( p.parameter1_->clone() )
    , parameter2_( p.parameter2_->clone() )
  {
  }

  ~ProductTopologyParameter()
  {
    delete parameter1_;
    delete parameter2_;
  }

  /**
   * @returns the value of the product.
   */
  double
  raw_value( const Position< 2 >& p, librandom::RngPtr& rng ) const
  {
    return parameter1_->value( p, rng ) * parameter2_->value( p, rng );
  }
  double
  raw_value( const Position< 3 >& p, librandom::RngPtr& rng ) const
  {
    return parameter1_->value( p, rng ) * parameter2_->value( p, rng );
  }

  TopologyParameter*
  clone() const
  {
    return new ProductTopologyParameter( *this );
  }

protected:
  TopologyParameter* parameter1_, *parameter2_;
};

/**
 * Parameter class representing the quotient of two parameters
 */
class QuotientTopologyParameter : public TopologyParameter
{
public:
  /**
   * Construct the quotient of the two given parameters. Copies are made
   * of the supplied Parameter objects.
   */
  QuotientTopologyParameter( const TopologyParameter& m1, const TopologyParameter& m2 )
    : TopologyParameter()
    , parameter1_( m1.clone() )
    , parameter2_( m2.clone() )
  {
  }

  /**
   * Copy constructor.
   */
  QuotientTopologyParameter( const QuotientTopologyParameter& p )
    : TopologyParameter( p )
    , parameter1_( p.parameter1_->clone() )
    , parameter2_( p.parameter2_->clone() )
  {
  }

  ~QuotientTopologyParameter()
  {
    delete parameter1_;
    delete parameter2_;
  }

  /**
   * @returns the value of the product.
   */
  double
  raw_value( const Position< 2 >& p, librandom::RngPtr& rng ) const
  {
    return parameter1_->value( p, rng ) / parameter2_->value( p, rng );
  }
  double
  raw_value( const Position< 3 >& p, librandom::RngPtr& rng ) const
  {
    return parameter1_->value( p, rng ) / parameter2_->value( p, rng );
  }

  TopologyParameter*
  clone() const
  {
    return new QuotientTopologyParameter( *this );
  }

protected:
  TopologyParameter* parameter1_, *parameter2_;
};

/**
 * Parameter class representing the sum of two parameters
 */
class SumTopologyParameter : public TopologyParameter
{
public:
  /**
   * Construct the sum of the two given parameters. Copies are made
   * of the supplied Parameter objects.
   */
  SumTopologyParameter( const TopologyParameter& m1, const TopologyParameter& m2 )
    : TopologyParameter()
    , parameter1_( m1.clone() )
    , parameter2_( m2.clone() )
  {
  }

  /**
   * Copy constructor.
   */
  SumTopologyParameter( const SumTopologyParameter& p )
    : TopologyParameter( p )
    , parameter1_( p.parameter1_->clone() )
    , parameter2_( p.parameter2_->clone() )
  {
  }

  ~SumTopologyParameter()
  {
    delete parameter1_;
    delete parameter2_;
  }

  /**
   * @returns the value of the sum.
   */
  double
  raw_value( const Position< 2 >& p, librandom::RngPtr& rng ) const
  {
    return parameter1_->value( p, rng ) + parameter2_->value( p, rng );
  }
  double
  raw_value( const Position< 3 >& p, librandom::RngPtr& rng ) const
  {
    return parameter1_->value( p, rng ) + parameter2_->value( p, rng );
  }

  TopologyParameter*
  clone() const
  {
    return new SumTopologyParameter( *this );
  }

protected:
  TopologyParameter* parameter1_, *parameter2_;
};

/**
 * Parameter class representing the difference of two parameters
 */
class DifferenceTopologyParameter : public TopologyParameter
{
public:
  /**
   * Construct the difference of the two given parameters. Copies are made
   * of the supplied Parameter objects.
   */
  DifferenceTopologyParameter( const TopologyParameter& m1,
    const TopologyParameter& m2 )
    : TopologyParameter()
    , parameter1_( m1.clone() )
    , parameter2_( m2.clone() )
  {
  }

  /**
   * Copy constructor.
   */
  DifferenceTopologyParameter( const DifferenceTopologyParameter& p )
    : TopologyParameter( p )
    , parameter1_( p.parameter1_->clone() )
    , parameter2_( p.parameter2_->clone() )
  {
  }

  ~DifferenceTopologyParameter()
  {
    delete parameter1_;
    delete parameter2_;
  }

  /**
   * @returns the value of the difference.
   */
  double
  raw_value( const Position< 2 >& p, librandom::RngPtr& rng ) const
  {
    return parameter1_->value( p, rng ) - parameter2_->value( p, rng );
  }
  double
  raw_value( const Position< 3 >& p, librandom::RngPtr& rng ) const
  {
    return parameter1_->value( p, rng ) - parameter2_->value( p, rng );
  }

  TopologyParameter*
  clone() const
  {
    return new DifferenceTopologyParameter( *this );
  }

protected:
  TopologyParameter* parameter1_, *parameter2_;
};

/**
 * Parameter class for a parameter oriented in the opposite direction.
 */
class ConverseTopologyParameter : public TopologyParameter
{
public:
  /**
   * Construct the converse of the given parameter. A copy is made of the
   * supplied Parameter object.
   */
  ConverseTopologyParameter( const TopologyParameter& p )
    : TopologyParameter( p )
    , p_( p.clone() )
  {
  }

  /**
   * Copy constructor.
   */
  ConverseTopologyParameter( const ConverseTopologyParameter& p )
    : TopologyParameter( p )
    , p_( p.p_->clone() )
  {
  }

  ~ConverseTopologyParameter()
  {
    delete p_;
  }

  /**
   * @returns the value of the parameter.
   */
  double
  raw_value( const Position< 2 >& p, librandom::RngPtr& rng ) const
  {
    return p_->raw_value( -p, rng );
  }
  double
  raw_value( const Position< 3 >& p, librandom::RngPtr& rng ) const
  {
    return p_->raw_value( -p, rng );
  }

  TopologyParameter*
  clone() const
  {
    return new ConverseTopologyParameter( *this );
  }

protected:
  TopologyParameter* p_;
};

inline TopologyParameter*
TopologyParameter::multiply_topology_parameter( const TopologyParameter& other ) const
{
  return new ProductTopologyParameter( *this, other );
}

inline TopologyParameter*
TopologyParameter::divide_topology_parameter( const TopologyParameter& other ) const
{
  return new QuotientTopologyParameter( *this, other );
}

inline TopologyParameter*
TopologyParameter::add_topology_parameter( const TopologyParameter& other ) const
{
  return new SumTopologyParameter( *this, other );
}

inline TopologyParameter*
TopologyParameter::subtract_topology_parameter( const TopologyParameter& other ) const
{
  return new DifferenceTopologyParameter( *this, other );
}


} // namespace nest

#endif
