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
#include <cmath>

// Includes from librandom:
#include "normal_randomdev.h"
#include "randomgen.h"

// Includes from nestkernel:
#include "nest_names.h"
#include "nest_types.h"
#include "nestmodule.h"
#include "node_collection.h"

// Includes from sli:
#include "dictutils.h"

namespace nest
{

class AbstractLayer;

/**
 * Abstract base class for parameters.
 */
class Parameter
{
public:
  /**
   * Creates an Parameter with default values.
   */
  Parameter() = default;

  /**
   * Creates a Parameter with values specified in a dictionary.
   * @param d dictionary with parameter values
   */
  Parameter( const DictionaryDatum& )
  {
  }

  /**
   * Virtual destructor
   */
  virtual ~Parameter() = default;

  /**
   * Generates a value based on parameter specifications and arguments.
   * Note that not all parameters support all overloaded versions.
   * @returns the value of the parameter.
   */
  virtual double value( librandom::RngPtr& rng, Node* node ) const = 0;
  virtual double
  value( librandom::RngPtr& rng, index, Node*, thread ) const
  {
    return value( rng, nullptr );
  }

  virtual double
  value( librandom::RngPtr& rng,
    const std::vector< double >&,
    const std::vector< double >&,
    const AbstractLayer& ) const
  {
    return value( rng, nullptr );
  }

  /**
   * Create a copy of the parameter.
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
   * Create comparison of this parameter with another.
   * @returns a new dynamically allocated parameter.
   */
  virtual Parameter* compare_parameter( const Parameter& other, const DictionaryDatum& d ) const;
  /**
   * Create parameter choosing between two other parameters,
   * based on this parameter.
   * @returns a new dynamically allocated parameter.
   */
  virtual Parameter* conditional_parameter( const Parameter& if_true, const Parameter& if_false ) const;

  /**
   * Create parameter whose value is the minimum of a given parameter's value and the given value.
   * @returns a new dynamically allocated parameter.
   */
  virtual Parameter* min( const double other ) const;
  /**
   * Create parameter whose value is the maximum of a given parameter's value and the given value.
   * @returns a new dynamically allocated parameter.
   */
  virtual Parameter* max( const double other ) const;
  /**
   * Create parameter redrawing the value if the value of a parameter is outside the set limits.
   * @returns a new dynamically allocated parameter.
   */
  virtual Parameter* redraw( const double min, const double max ) const;

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
  /**
   * Create this parameter raised to the power of an exponent.
   * @returns a new dynamically allocated parameter.
   */
  virtual Parameter* pow( const double exponent ) const;

  /**
   * Create a parameter that can generate position vectors from a given set of parameters.
   * @returns a new dynamically allocated parameter.
   */
  virtual Parameter* dimension_parameter( const Parameter& y_parameter ) const;
  virtual Parameter* dimension_parameter( const Parameter& y_parameter, const Parameter& z_parameter ) const;

  /**
   * Applies a parameter on a single-node ID NodeCollection and given array of positions.
   * @returns array of result values, one per position in the TokenArray.
   */
  std::vector< double > apply( const NodeCollectionPTR&, const TokenArray& ) const;

  /**
   * Check if the Parameter is based on spatial properties.
   * @returns true if the Parameter is based on spatial properties, false otherwise.
   */
  bool is_spatial() const;

protected:
  bool parameter_is_spatial_{ false };

  Node* node_id_to_node_ptr_( const index, const thread ) const;
};

/**
 * Parameter with constant value.
 */
class ConstantParameter : public Parameter
{
public:
  using Parameter::value;

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

  ~ConstantParameter() override = default;

  /**
   * @returns the constant value of this parameter.
   */
  double
  value( librandom::RngPtr&, Node* ) const override
  {
    return value_;
  }

  Parameter*
  clone() const override
  {
    return new ConstantParameter( value_ );
  }

private:
  double value_;
};


/**
 * Random parameter with uniform distribution in [min,max).
 */
class UniformParameter : public Parameter
{
public:
  using Parameter::value;

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
  value( librandom::RngPtr& rng, Node* ) const override
  {
    return lower_ + rng->drand() * range_;
  }

  Parameter*
  clone() const override
  {
    return new UniformParameter( *this );
  }

private:
  double lower_, range_;
};


/**
 * Random parameter with normal distribution.
 */
class NormalParameter : public Parameter
{
public:
  using Parameter::value;

  /**
   * Parameters:
   * mean  - mean value
   * sigma - standard distribution
   */
  NormalParameter( const DictionaryDatum& d )
    : Parameter( d )
    , mean_( 0.0 )
    , std_( 1.0 )
    , rdev()
  {
    updateValue< double >( d, names::mean, mean_ );
    updateValue< double >( d, names::std, std_ );
    if ( std_ <= 0 )
    {
      throw BadProperty(
        "nest::NormalParameter: "
        "std > 0 required." );
    }
  }

  double
  value( librandom::RngPtr& rng, Node* ) const override
  {
    return mean_ + rdev( rng ) * std_;
  }

  Parameter*
  clone() const override
  {
    return new NormalParameter( *this );
  }

private:
  double mean_, std_;
  librandom::NormalRandomDev rdev;
};


/**
 * Random parameter with lognormal distribution.
 */
class LognormalParameter : public Parameter
{
public:
  using Parameter::value;

  /**
   * Parameters:
   * mu    - mean value of logarithm
   * sigma - standard distribution of logarithm
   */
  LognormalParameter( const DictionaryDatum& d )
    : Parameter( d )
    , mean_( 0.0 )
    , std_( 1.0 )
    , rdev()
  {
    updateValue< double >( d, names::mean, mean_ );
    updateValue< double >( d, names::std, std_ );
    if ( std_ <= 0 )
    {
      throw BadProperty(
        "nest::LognormalParameter: "
        "std > 0 required." );
    }
  }

  double
  value( librandom::RngPtr& rng, Node* ) const override
  {
    return std::exp( mean_ + rdev( rng ) * std_ );
  }

  Parameter*
  clone() const override
  {
    return new LognormalParameter( *this );
  }

private:
  double mean_, std_;
  librandom::NormalRandomDev rdev;
};


/**
 * Random parameter with exponential distribution.
 */
class ExponentialParameter : public Parameter
{
public:
  using Parameter::value;

  /**
   * Parameters:
   * scale - the scale parameter
   */
  ExponentialParameter( const DictionaryDatum& d )
    : Parameter( d )
    , beta_( 1.0 )
  {
    updateValue< double >( d, names::beta, beta_ );
  }

  double
  value( librandom::RngPtr& rng, Node* ) const override
  {
    return beta_ * ( -std::log( 1 - rng->drand() ) );
  }

  Parameter*
  clone() const override
  {
    return new ExponentialParameter( *this );
  }

private:
  double beta_;
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
   *             0: x, 1: y, 2: z.
   * synaptic_endpoint - If specified, specifies if the position should be taken
   *                     from the presynaptic or postsynaptic node in a connection.
   *                     0: unspecified, 1: presynaptic, 2: postsynaptic.
   */
  NodePosParameter( const DictionaryDatum& d )
    : Parameter( d )
    , dimension_( 0 )
    , synaptic_endpoint_( 0 )
  {
    parameter_is_spatial_ = true;
    bool dimension_specified = updateValue< long >( d, names::dimension, dimension_ );
    if ( not dimension_specified )
    {
      throw BadParameterValue( "Dimension must be specified when creating a node position parameter." );
    }
    if ( dimension_ < 0 )
    {
      throw BadParameterValue( "Node position parameter dimension cannot be negative." );
    }
    updateValue< long >( d, names::synaptic_endpoint, synaptic_endpoint_ );
    if ( synaptic_endpoint_ < 0 or 2 < synaptic_endpoint_ )
    {
      throw BadParameterValue( "Synaptic endpoint must either be unspecified (0), source (1) or target (2)." );
    }
  }

  double
  value( librandom::RngPtr& rng, Node* node ) const override
  {
    if ( synaptic_endpoint_ != 0 )
    {
      throw BadParameterValue( "Source or target position parameter can only be used when connecting." );
    }
    return get_node_pos_( rng, node );
  }

  double
  value( librandom::RngPtr&, index, Node*, thread ) const override
  {
    throw KernelException( "Node position parameter can only be used when using ConnectLayers." );
  }

  double
  value( librandom::RngPtr&,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& ) const override
  {
    switch ( synaptic_endpoint_ )
    {
    case 0:
      throw BadParameterValue( "Node position parameter cannot be used when connecting." );
    case 1:
    {
      return source_pos[ dimension_ ];
    }
    case 2:
      return target_pos[ dimension_ ];
    }
    throw KernelException( "Wrong synaptic_endpoint_." );
  }

  Parameter*
  clone() const override
  {
    return new NodePosParameter( *this );
  }

private:
  int dimension_;
  int synaptic_endpoint_;

  double get_node_pos_( librandom::RngPtr& rng, Node* node ) const;
};


/**
 * Parameter representing the spatial distance between two nodes, optionally in a specific dimension.
 */
class SpatialDistanceParameter : public Parameter
{
public:
  SpatialDistanceParameter( const DictionaryDatum& d )
    : Parameter( d )
    , dimension_( 0 )
  {
    parameter_is_spatial_ = true;
    updateValue< long >( d, names::dimension, dimension_ );
    if ( dimension_ < 0 )
    {
      throw BadParameterValue( "Spatial distance parameter dimension cannot be negative." );
    }
  }

  double
  value( librandom::RngPtr&, Node* ) const override
  {
    throw BadParameterValue( "Spatial distance parameter can only be used when connecting." );
  }

  double
  value( librandom::RngPtr&, index, Node*, thread ) const override
  {
    throw KernelException( "Spatial distance parameter can only be used when using ConnectLayers." );
  }

  double value( librandom::RngPtr& rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) const override;

  Parameter*
  clone() const override
  {
    return new SpatialDistanceParameter( *this );
  }

private:
  int dimension_;
};


/**
 * Parameter class representing the product of two parameters.
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
    parameter_is_spatial_ = parameter1_->is_spatial() or parameter2_->is_spatial();
  }

  /**
   * Copy constructor.
   */
  ProductParameter( const ProductParameter& p )
    : Parameter( p )
    , parameter1_( p.parameter1_->clone() )
    , parameter2_( p.parameter2_->clone() )
  {
    parameter_is_spatial_ = parameter1_->is_spatial() or parameter2_->is_spatial();
  }

  ~ProductParameter() override
  {
    delete parameter1_;
    delete parameter2_;
  }

  /**
   * @returns the value of the product.
   */
  double
  value( librandom::RngPtr& rng, Node* node ) const override
  {
    return parameter1_->value( rng, node ) * parameter2_->value( rng, node );
  }

  double
  value( librandom::RngPtr& rng, index snode_id, Node* target, thread target_thread ) const override
  {
    return parameter1_->value( rng, snode_id, target, target_thread )
      * parameter2_->value( rng, snode_id, target, target_thread );
  }

  double
  value( librandom::RngPtr& rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) const override
  {
    return parameter1_->value( rng, source_pos, target_pos, layer )
      * parameter2_->value( rng, source_pos, target_pos, layer );
  }

  Parameter*
  clone() const override
  {
    return new ProductParameter( *this );
  }

protected:
  Parameter* parameter1_, *parameter2_;
};

/**
 * Parameter class representing the quotient of two parameters.
 */
class QuotientParameter : public Parameter
{
public:
  /**
   * Construct the quotient of two given parameters. Copies are made
   * of the supplied Parameter objects.
   */
  QuotientParameter( const Parameter& m1, const Parameter& m2 )
    : Parameter()
    , parameter1_( m1.clone() )
    , parameter2_( m2.clone() )
  {
    parameter_is_spatial_ = parameter1_->is_spatial() or parameter2_->is_spatial();
  }

  /**
   * Copy constructor.
   */
  QuotientParameter( const QuotientParameter& p )
    : Parameter( p )
    , parameter1_( p.parameter1_->clone() )
    , parameter2_( p.parameter2_->clone() )
  {
    parameter_is_spatial_ = parameter1_->is_spatial() or parameter2_->is_spatial();
  }

  ~QuotientParameter() override
  {
    delete parameter1_;
    delete parameter2_;
  }

  /**
   * @returns the value of the product.
   */
  double
  value( librandom::RngPtr& rng, Node* node ) const override
  {
    return parameter1_->value( rng, node ) / parameter2_->value( rng, node );
  }

  double
  value( librandom::RngPtr& rng, index snode_id, Node* target, thread target_thread ) const override
  {
    return parameter1_->value( rng, snode_id, target, target_thread )
      / parameter2_->value( rng, snode_id, target, target_thread );
  }

  double
  value( librandom::RngPtr& rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) const override
  {
    return parameter1_->value( rng, source_pos, target_pos, layer )
      / parameter2_->value( rng, source_pos, target_pos, layer );
  }

  Parameter*
  clone() const override
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
   * Construct the sum of two given parameters. Copies are made
   * of the supplied Parameter objects.
   */
  SumParameter( const Parameter& m1, const Parameter& m2 )
    : Parameter()
    , parameter1_( m1.clone() )
    , parameter2_( m2.clone() )
  {
    parameter_is_spatial_ = parameter1_->is_spatial() or parameter2_->is_spatial();
  }

  /**
   * Copy constructor.
   */
  SumParameter( const SumParameter& p )
    : Parameter( p )
    , parameter1_( p.parameter1_->clone() )
    , parameter2_( p.parameter2_->clone() )
  {
    parameter_is_spatial_ = parameter1_->is_spatial() or parameter2_->is_spatial();
  }

  ~SumParameter() override
  {
    delete parameter1_;
    delete parameter2_;
  }

  /**
   * @returns the value of the sum.
   */
  double
  value( librandom::RngPtr& rng, Node* node ) const override
  {
    return parameter1_->value( rng, node ) + parameter2_->value( rng, node );
  }

  double
  value( librandom::RngPtr& rng, index snode_id, Node* target, thread target_thread ) const override
  {
    return parameter1_->value( rng, snode_id, target, target_thread )
      + parameter2_->value( rng, snode_id, target, target_thread );
  }

  double
  value( librandom::RngPtr& rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) const override
  {
    return parameter1_->value( rng, source_pos, target_pos, layer )
      + parameter2_->value( rng, source_pos, target_pos, layer );
  }

  Parameter*
  clone() const override
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
   * Construct the difference of two given parameters. Copies are made
   * of the supplied Parameter objects.
   */
  DifferenceParameter( const Parameter& m1, const Parameter& m2 )
    : Parameter()
    , parameter1_( m1.clone() )
    , parameter2_( m2.clone() )
  {
    parameter_is_spatial_ = parameter1_->is_spatial() or parameter2_->is_spatial();
  }

  /**
   * Copy constructor.
   */
  DifferenceParameter( const DifferenceParameter& p )
    : Parameter( p )
    , parameter1_( p.parameter1_->clone() )
    , parameter2_( p.parameter2_->clone() )
  {
    parameter_is_spatial_ = parameter1_->is_spatial() or parameter2_->is_spatial();
  }

  ~DifferenceParameter() override
  {
    delete parameter1_;
    delete parameter2_;
  }

  /**
   * @returns the value of the difference.
   */
  double
  value( librandom::RngPtr& rng, Node* node ) const override
  {
    return parameter1_->value( rng, node ) - parameter2_->value( rng, node );
  }

  double
  value( librandom::RngPtr& rng, index snode_id, Node* target, thread target_thread ) const override
  {
    return parameter1_->value( rng, snode_id, target, target_thread )
      - parameter2_->value( rng, snode_id, target, target_thread );
  }

  double
  value( librandom::RngPtr& rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) const override
  {
    return parameter1_->value( rng, source_pos, target_pos, layer )
      - parameter2_->value( rng, source_pos, target_pos, layer );
  }

  Parameter*
  clone() const override
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
    parameter_is_spatial_ = p_->is_spatial();
  }

  /**
   * Copy constructor.
   */
  ConverseParameter( const ConverseParameter& p )
    : Parameter( p )
    , p_( p.p_->clone() )
  {
    parameter_is_spatial_ = p_->is_spatial();
  }

  ~ConverseParameter() override
  {
    delete p_;
  }

  /**
   * @returns the value of the parameter.
   */
  double
  value( librandom::RngPtr& rng, Node* node ) const override
  {
    return p_->value( rng, node );
  }

  double
  value( librandom::RngPtr& rng, index snode_id, Node* target, thread target_thread ) const override
  {
    return p_->value( rng, snode_id, target, target_thread );
  }

  double
  value( librandom::RngPtr& rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) const override
  {
    return p_->value( rng, source_pos, target_pos, layer );
  }

  Parameter*
  clone() const override
  {
    return new ConverseParameter( *this );
  }

protected:
  Parameter* p_;
};


/**
 * Parameter class representing the comparison of two parameters.
 */
class ComparingParameter : public Parameter
{
public:
  /**
   * Construct the comparison of the two given parameters. Copies are made
   * of the supplied Parameter objects.
   *
   * comparator - Operator to use as a comparator.
   *              0: <
   *              2: <=
   *              4: ==
   *              5: !=
   *              3: >=
   *              1: >
   *
   */
  ComparingParameter( const Parameter& m1, const Parameter& m2, const DictionaryDatum& d )
    : Parameter()
    , parameter1_( m1.clone() )
    , parameter2_( m2.clone() )
    , comparator_( -1 )
  {
    if ( not updateValue< long >( d, names::comparator, comparator_ ) )
    {
      throw BadParameter( "A comparator has to be specified." );
    }
    if ( comparator_ < 0 or 5 < comparator_ )
    {
      throw BadParameter( "Comparator specification has to be in the range 0-5." );
    }
    parameter_is_spatial_ = parameter1_->is_spatial() or parameter2_->is_spatial();
  }

  /**
   * Copy constructor.
   */
  ComparingParameter( const ComparingParameter& p )
    : Parameter( p )
    , parameter1_( p.parameter1_->clone() )
    , parameter2_( p.parameter2_->clone() )
    , comparator_( p.comparator_ )
  {
  }

  ~ComparingParameter() override
  {
    delete parameter1_;
    delete parameter2_;
  }

  /**
   * @returns the result of the comparison, bool given as a double.
   */
  double
  value( librandom::RngPtr& rng, Node* node ) const override
  {
    return compare_( parameter1_->value( rng, node ), parameter2_->value( rng, node ) );
  }

  double
  value( librandom::RngPtr& rng, index snode_id, Node* target, thread target_thread ) const override
  {
    return compare_( parameter1_->value( rng, snode_id, target, target_thread ),
      parameter2_->value( rng, snode_id, target, target_thread ) );
  }

  double
  value( librandom::RngPtr& rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) const override
  {
    return compare_( parameter1_->value( rng, source_pos, target_pos, layer ),
      parameter2_->value( rng, source_pos, target_pos, layer ) );
  }

  Parameter*
  clone() const override
  {
    return new ComparingParameter( *this );
  }

protected:
  Parameter* parameter1_, *parameter2_;

private:
  bool
  compare_( double value_a, double value_b ) const
  {
    switch ( comparator_ )
    {
    case 0:
      return value_a < value_b;
    case 1:
      return value_a <= value_b;
    case 2:
      return value_a == value_b;
    case 3:
      return value_a != value_b;
    case 4:
      return value_a >= value_b;
    case 5:
      return value_a > value_b;
    }
    throw KernelException( "Wrong comparison operator." );
  }

  int comparator_;
};


/**
 * Parameter class choosing a value based on a comparing parameter.
 */
class ConditionalParameter : public Parameter
{
public:
  /**
   * Construct the choice of two given parameters, based on a third.
   * Copies are made of the supplied Parameter objects.
   */
  ConditionalParameter( const Parameter& condition, const Parameter& if_true, const Parameter& if_false )
    : Parameter()
    , condition_( condition.clone() )
    , if_true_( if_true.clone() )
    , if_false_( if_false.clone() )
  {
    parameter_is_spatial_ = condition_->is_spatial() or if_true_->is_spatial() or if_false_->is_spatial();
  }

  /**
   * Copy constructor.
   */
  ConditionalParameter( const ConditionalParameter& p )
    : Parameter( p )
    , condition_( p.condition_->clone() )
    , if_true_( p.if_true_->clone() )
    , if_false_( p.if_false_->clone() )
  {
    parameter_is_spatial_ = condition_->is_spatial() or if_true_->is_spatial() or if_false_->is_spatial();
  }

  ~ConditionalParameter() override
  {
    delete condition_;
    delete if_true_;
    delete if_false_;
  }

  /**
   * @returns the value chosen by the comparison.
   */
  double
  value( librandom::RngPtr& rng, Node* node ) const override
  {
    if ( condition_->value( rng, node ) )
    {
      return if_true_->value( rng, node );
    }
    else
    {
      return if_false_->value( rng, node );
    }
  }

  double
  value( librandom::RngPtr& rng, index snode_id, Node* target, thread target_thread ) const override
  {
    if ( condition_->value( rng, snode_id, target, target_thread ) )
    {
      return if_true_->value( rng, snode_id, target, target_thread );
    }
    else
    {
      return if_false_->value( rng, snode_id, target, target_thread );
    }
  }

  double
  value( librandom::RngPtr& rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) const override
  {
    if ( condition_->value( rng, source_pos, target_pos, layer ) )
    {
      return if_true_->value( rng, source_pos, target_pos, layer );
    }
    else
    {
      return if_false_->value( rng, source_pos, target_pos, layer );
    }
  }

  Parameter*
  clone() const override
  {
    return new ConditionalParameter( *this );
  }

protected:
  Parameter* condition_, *if_true_, *if_false_;
};


/**
 * Parameter class representing the minimum of a parameter's value and a given value.
 */
class MinParameter : public Parameter
{
public:
  /**
   * Construct a min parameter. A copy is made of the supplied Parameter
   * object.
   */
  MinParameter( const Parameter& p, const double other_value )
    : Parameter( p )
    , p_( p.clone() )
    , other_value_( other_value )
  {
    parameter_is_spatial_ = p_->is_spatial();
  }

  /**
   * Copy constructor.
   */
  MinParameter( const MinParameter& p )
    : Parameter( p )
    , p_( p.p_->clone() )
    , other_value_( p.other_value_ )
  {
    parameter_is_spatial_ = p_->is_spatial();
  }

  ~MinParameter() override
  {
    delete p_;
  }

  /**
   * @returns the value of the parameter.
   */
  double
  value( librandom::RngPtr& rng, Node* node ) const override
  {
    return std::min( p_->value( rng, node ), other_value_ );
  }

  double
  value( librandom::RngPtr& rng, index snode_id, Node* target, thread target_thread ) const override
  {
    return std::min( p_->value( rng, snode_id, target, target_thread ), other_value_ );
  }

  double
  value( librandom::RngPtr& rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) const override
  {
    return std::min( p_->value( rng, source_pos, target_pos, layer ), other_value_ );
  }

  Parameter*
  clone() const override
  {
    return new MinParameter( *this );
  }

protected:
  const Parameter* p_;
  double other_value_;
};


/**
 * Parameter class representing the maximum of a parameter's value and a given value.
 */
class MaxParameter : public Parameter
{
public:
  /**
   * Construct a max parameter. A copy is made of the supplied Parameter
   * object.
   */
  MaxParameter( const Parameter& p, const double other_value )
    : Parameter( p )
    , p_( p.clone() )
    , other_value_( other_value )
  {
    parameter_is_spatial_ = p_->is_spatial();
  }

  /**
   * Copy constructor.
   */
  MaxParameter( const MaxParameter& p )
    : Parameter( p )
    , p_( p.p_->clone() )
    , other_value_( p.other_value_ )
  {
    parameter_is_spatial_ = p_->is_spatial();
  }

  ~MaxParameter() override
  {
    delete p_;
  }

  /**
   * @returns the value of the parameter.
   */
  double
  value( librandom::RngPtr& rng, Node* node ) const override
  {
    return std::max( p_->value( rng, node ), other_value_ );
  }

  double
  value( librandom::RngPtr& rng, index snode_id, Node* target, thread target_thread ) const override
  {
    return std::max( p_->value( rng, snode_id, target, target_thread ), other_value_ );
  }

  double
  value( librandom::RngPtr& rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) const override
  {
    return std::max( p_->value( rng, source_pos, target_pos, layer ), other_value_ );
  }

  Parameter*
  clone() const override
  {
    return new MaxParameter( *this );
  }

protected:
  const Parameter* p_;
  double other_value_;
};


/**
 * Parameter class redrawing a parameter value if it is outside of specified limits.
 */
class RedrawParameter : public Parameter
{
public:
  /**
   * Construct a redrawing parameter. A copy is made of the supplied Parameter
   * object.
   */
  RedrawParameter( const Parameter& p, const double min, const double max );

  /**
   * Copy constructor.
   */
  RedrawParameter( const RedrawParameter& p )
    : Parameter( p )
    , p_( p.p_->clone() )
    , min_( p.min_ )
    , max_( p.max_ )
    , max_redraws_( p.max_redraws_ )
  {
    parameter_is_spatial_ = p_->is_spatial();
  }

  ~RedrawParameter() override
  {
    delete p_;
  }

  /**
   * @returns the value of the parameter.
   */
  double value( librandom::RngPtr& rng, Node* node ) const override;
  double value( librandom::RngPtr& rng, index snode_id, Node* target, thread target_thread ) const override;
  double value( librandom::RngPtr& rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) const override;

  Parameter*
  clone() const override
  {
    return new RedrawParameter( *this );
  }

protected:
  const Parameter* p_;
  double min_;
  double max_;
  const size_t max_redraws_;
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
    parameter_is_spatial_ = p_->is_spatial();
  }

  /**
   * Copy constructor.
   */
  ExpParameter( const ExpParameter& p )
    : Parameter( p )
    , p_( p.p_->clone() )
  {
  }

  ~ExpParameter() override
  {
    delete p_;
  }

  /**
   * @returns the value of the parameter.
   */
  double
  value( librandom::RngPtr& rng, Node* node ) const override
  {
    return std::exp( p_->value( rng, node ) );
  }

  double
  value( librandom::RngPtr& rng, index snode_id, Node* target, thread target_thread ) const override
  {
    return std::exp( p_->value( rng, snode_id, target, target_thread ) );
  }

  double
  value( librandom::RngPtr& rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) const override
  {
    return std::exp( p_->value( rng, source_pos, target_pos, layer ) );
  }

  Parameter*
  clone() const override
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
    parameter_is_spatial_ = p_->is_spatial();
  }

  /**
   * Copy constructor.
   */
  SinParameter( const SinParameter& p )
    : Parameter( p )
    , p_( p.p_->clone() )
  {
    parameter_is_spatial_ = p_->is_spatial();
  }

  ~SinParameter() override
  {
    delete p_;
  }

  /**
   * @returns the value of the parameter.
   */
  double
  value( librandom::RngPtr& rng, Node* node ) const override
  {
    return std::sin( p_->value( rng, node ) );
  }

  double
  value( librandom::RngPtr& rng, index snode_id, Node* target, thread target_thread ) const override
  {
    return std::sin( p_->value( rng, snode_id, target, target_thread ) );
  }

  double
  value( librandom::RngPtr& rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) const override
  {
    return std::sin( p_->value( rng, source_pos, target_pos, layer ) );
  }

  Parameter*
  clone() const override
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
    parameter_is_spatial_ = p_->is_spatial();
  }

  /**
   * Copy constructor.
   */
  CosParameter( const CosParameter& p )
    : Parameter( p )
    , p_( p.p_->clone() )
  {
    parameter_is_spatial_ = p_->is_spatial();
  }

  ~CosParameter() override
  {
    delete p_;
  }

  /**
   * @returns the value of the parameter.
   */
  double
  value( librandom::RngPtr& rng, Node* node ) const override
  {
    return std::cos( p_->value( rng, node ) );
  }

  double
  value( librandom::RngPtr& rng, index snode_id, Node* target, thread target_thread ) const override
  {
    return std::cos( p_->value( rng, snode_id, target, target_thread ) );
  }

  double
  value( librandom::RngPtr& rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) const override
  {
    return std::cos( p_->value( rng, source_pos, target_pos, layer ) );
  }

  Parameter*
  clone() const override
  {
    return new CosParameter( *this );
  }

protected:
  Parameter* p_;
};


/**
 * Parameter class representing the parameter raised to the power of an
 * exponent.
 */
class PowParameter : public Parameter
{
public:
  /**
   * Construct the parameter. A copy is made of the supplied Parameter object.
   */
  PowParameter( const Parameter& p, const double exponent )
    : Parameter( p )
    , p_( p.clone() )
    , exponent_( exponent )
  {
    parameter_is_spatial_ = p_->is_spatial();
  }

  /**
   * Copy constructor.
   */
  PowParameter( const PowParameter& p )
    : Parameter( p )
    , p_( p.p_->clone() )
    , exponent_( p.exponent_ )
  {
    parameter_is_spatial_ = p_->is_spatial();
  }

  ~PowParameter() override
  {
    delete p_;
  }

  /**
   * @returns the value of the parameter.
   */
  double
  value( librandom::RngPtr& rng, Node* node ) const override
  {
    return std::pow( p_->value( rng, node ), exponent_ );
  }

  double
  value( librandom::RngPtr& rng, index snode_id, Node* target, thread target_thread ) const override
  {
    return std::pow( p_->value( rng, snode_id, target, target_thread ), exponent_ );
  }

  double
  value( librandom::RngPtr& rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) const override
  {
    return std::pow( p_->value( rng, source_pos, target_pos, layer ), exponent_ );
  }

  Parameter*
  clone() const override
  {
    return new PowParameter( *this );
  }

protected:
  Parameter* p_;
  const double exponent_;
};


/**
 * Position-generating Parameter class. One Parameter per dimension is
 * stored. When getting a position vector, a value for each dimension is
 * generated from their respective Parameters.
 */
class DimensionParameter : public Parameter
{
public:
  using Parameter::value;

  /**
   * Construct the Parameter with one given Parameter per dimension. A
   * copy is made of the supplied Parameter objects.
   */
  DimensionParameter( const Parameter& px, const Parameter& py )
    : num_dimensions_( 2 )
    , px_( px.clone() )
    , py_( py.clone() )
  {
    parameter_is_spatial_ = true;
  }

  DimensionParameter( const Parameter& px, const Parameter& py, const Parameter& pz )
    : num_dimensions_( 3 )
    , px_( px.clone() )
    , py_( py.clone() )
    , pz_( pz.clone() )
  {
    parameter_is_spatial_ = true;
  }

  /**
   * Copy constructor.
   */
  DimensionParameter( const DimensionParameter& p )
    : Parameter( p )
    , num_dimensions_( p.num_dimensions_ )
    , px_( p.px_->clone() )
    , py_( p.py_->clone() )
    , pz_( p.pz_->clone() )
  {
    parameter_is_spatial_ = true;
  }

  ~DimensionParameter() override
  {
    delete px_;
    delete py_;
    if ( num_dimensions_ == 3 )
    {
      delete pz_;
    }
  }

  /**
   * The DimensionParameter has no double value, so this method will always throw.
   */
  double
  value( librandom::RngPtr&, Node* ) const override
  {
    throw KernelException( "Cannot get value of DimensionParameter." );
  }

  double
  value( librandom::RngPtr&, index, Node*, thread ) const override
  {
    throw KernelException( "Cannot get value of DimensionParameter." );
  }

  /**
   * Generates a position with values for each dimension generated from their respective parameters.
   * @returns The position, given as an array.
   */
  std::vector< double >
  get_values( librandom::RngPtr& rng )
  {
    switch ( num_dimensions_ )
    {
    case 2:
      return { px_->value( rng, nullptr ), py_->value( rng, nullptr ) };
    case 3:
      return { px_->value( rng, nullptr ), py_->value( rng, nullptr ), pz_->value( rng, nullptr ) };
    }
    throw KernelException( "Wrong number of dimensions in get_values!" );
  }

  int
  get_num_dimensions() const
  {
    return num_dimensions_;
  }

  Parameter*
  clone() const override
  {
    return new DimensionParameter( *this );
  }

protected:
  int num_dimensions_;
  Parameter* px_;
  Parameter* py_;
  Parameter* pz_;
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
Parameter::compare_parameter( const Parameter& other, const DictionaryDatum& d ) const
{
  return new ComparingParameter( *this, other, d );
}

inline Parameter*
Parameter::conditional_parameter( const Parameter& if_true, const Parameter& if_false ) const
{
  return new ConditionalParameter( *this, if_true, if_false );
}

inline Parameter*
Parameter::min( const double other_value ) const
{
  return new MinParameter( *this, other_value );
}
inline Parameter*
Parameter::max( const double other_value ) const
{
  return new MaxParameter( *this, other_value );
}
inline Parameter*
Parameter::redraw( const double min, const double max ) const
{
  return new RedrawParameter( *this, min, max );
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

inline Parameter*
Parameter::pow( const double exponent ) const
{
  return new PowParameter( *this, exponent );
}


inline Parameter*
Parameter::dimension_parameter( const Parameter& y_parameter ) const
{
  return new DimensionParameter( *this, y_parameter );
}

inline Parameter*
Parameter::dimension_parameter( const Parameter& y_parameter, const Parameter& z_parameter ) const
{
  return new DimensionParameter( *this, y_parameter, z_parameter );
}

inline bool
Parameter::is_spatial() const
{
  return parameter_is_spatial_;
}

} // namespace nest

#endif
