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

// Includes from nestkernel:
#include "nest_names.h"
#include "nest_types.h"
#include "nestmodule.h"
#include "node_collection.h"
#include "random_generators.h"

// Includes from libnestutil:
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
   * Creates a Parameter with specifications specified in a dictionary.
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
  virtual double value( RngPtr rng, Node* node ) = 0;
  virtual double
  value( RngPtr rng, index, Node*, thread )
  {
    return value( rng, nullptr );
  }

  virtual double
  value( RngPtr rng, const std::vector< double >&, const std::vector< double >&, const AbstractLayer& )
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
  std::vector< double > apply( const NodeCollectionPTR&, const TokenArray& );

  /**
   * Check if the Parameter is based on spatial properties.
   * @returns true if the Parameter is based on spatial properties, false otherwise.
   */
  bool is_spatial() const;

  /**
   * Check if the Parameter only returns integer values.
   * @returns true if the Parameter only returns integers, false otherwise.
   */
  bool returns_int_only() const;

protected:
  bool parameter_is_spatial_{ false };
  bool parameter_returns_int_only_{ false };

  Node* node_id_to_node_ptr_( const index, const thread ) const;
  bool value_is_integer_( const double value ) const;
};

/**
 * Parameter with constant value.
 */
class ConstantParameter : public Parameter
{
public:
  using Parameter::value;

  /**
   * Creates a ConstantParameter with a specified value.
   * @param value parameter value
   */
  ConstantParameter( double value )
    : Parameter()
    , value_( value )
  {
  }

  /**
   * Creates a ConstantParameter with the value specified in a dictionary.
   * @param d dictionary with the parameter value
   *
   * The dictionary must include the following entry:
   * value - constant value of this parameter
   */
  ConstantParameter( const DictionaryDatum& d )
    : Parameter( d )
  {
    value_ = getValue< double >( d, "value" );
    parameter_returns_int_only_ = value_is_integer_( value_ );
  }

  ~ConstantParameter() override = default;

  /**
   * @returns the constant value of this parameter.
   */
  double
  value( RngPtr, Node* ) override
  {
    return value_;
  }

  Parameter*
  clone() const override
  {
    return new ConstantParameter( *this );
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
   * Creates a UniformParameter with specifications specified in a dictionary.
   * @param d dictionary with parameter specifications
   *
   * The dictionary can include the following entries:
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
  value( RngPtr rng, Node* ) override
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
 * Random parameter with uniform distribution in [0,max), yielding integer values.
 */
class UniformIntParameter : public Parameter
{
public:
  using Parameter::value;

  /**
   * Creates a UniformIntParameter with specifications specified in a dictionary.
   * @param d dictionary with parameter specifications
   *
   * The dictionary can include the following entries:
   * max - maximum value
   */
  UniformIntParameter( const DictionaryDatum& d )
    : Parameter( d )
    , max_( 1.0 )
  {
    updateValue< long >( d, names::max, max_ );
    if ( max_ <= 0 )
    {
      throw BadProperty( "nest::UniformIntParameter: max > 0 required." );
    }
    parameter_returns_int_only_ = true;
  }

  double
  value( RngPtr rng, Node* ) override
  {
    return rng->ulrand( max_ );
  }

  Parameter*
  clone() const override
  {
    return new UniformIntParameter( *this );
  }

private:
  double max_;
};


/**
 * Random parameter with normal distribution.
 */
class NormalParameter : public Parameter
{
public:
  using Parameter::value;

  /**
   * Creates a NormalParameter with specifications specified in a dictionary.
   * @param d dictionary with parameter specifications
   *
   * The dictionary can include the following entries:
   * mean - mean value
   * std - standard deviation
   */
  NormalParameter( const DictionaryDatum& d );

  double value( RngPtr rng, Node* ) override;

  Parameter*
  clone() const override
  {
    return new NormalParameter( *this );
  }

private:
  double mean_, std_;
  std::vector< normal_distribution > normal_dists_;
};


/**
 * Random parameter with lognormal distribution.
 */
class LognormalParameter : public Parameter
{
public:
  using Parameter::value;

  /**
   * Creates a LognormalParameter with specifications specified in a dictionary.
   * @param d dictionary with parameter specifications
   *
   * The dictionary can include the following entries:
   * mean - mean value of logarithm
   * sigma - standard distribution of logarithm
   */
  LognormalParameter( const DictionaryDatum& d );

  double value( RngPtr rng, Node* ) override;

  Parameter*
  clone() const override
  {
    return new LognormalParameter( *this );
  }

private:
  double mean_, std_;
  std::vector< lognormal_distribution > lognormal_dists_;
};


/**
 * Random parameter with exponential distribution.
 */
class ExponentialParameter : public Parameter
{
public:
  using Parameter::value;

  /**
   * Creates a ExponentialParameter with specifications specified in a dictionary.
   * @param d dictionary with parameter specifications
   *
   * The dictionary can include the following entries:
   * beta - the scale parameter
   */
  ExponentialParameter( const DictionaryDatum& d )
    : Parameter( d )
    , beta_( 1.0 )
  {
    updateValue< double >( d, names::beta, beta_ );
  }

  double
  value( RngPtr rng, Node* ) override
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
   * Creates a NodePosParameter with specifications specified in a dictionary.
   * @param d dictionary with parameter specifications
   *
   * The dictionary can include the following entries:
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
  value( RngPtr rng, Node* node ) override
  {
    if ( synaptic_endpoint_ != 0 )
    {
      throw BadParameterValue( "Source or target position parameter can only be used when connecting." );
    }
    return get_node_pos_( rng, node );
  }

  double
  value( RngPtr, index, Node*, thread ) override
  {
    throw KernelException( "Node position parameter can only be used when connecting spatially distributed nodes." );
  }

  double
  value( RngPtr,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& ) override
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

  double get_node_pos_( RngPtr rng, Node* node ) const;
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
  value( RngPtr, Node* ) override
  {
    throw BadParameterValue( "Spatial distance parameter can only be used when connecting." );
  }

  double
  value( RngPtr, index, Node*, thread ) override
  {
    throw KernelException( "Spatial distance parameter can only be used when connecting spatially distributed nodes." );
  }

  double value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) override;

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
    parameter_returns_int_only_ = parameter1_->returns_int_only() and parameter2_->returns_int_only();
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
    parameter_returns_int_only_ = parameter1_->returns_int_only() and parameter2_->returns_int_only();
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
  value( RngPtr rng, Node* node ) override
  {
    return parameter1_->value( rng, node ) * parameter2_->value( rng, node );
  }

  double
  value( RngPtr rng, index snode_id, Node* target, thread target_thread ) override
  {
    return parameter1_->value( rng, snode_id, target, target_thread )
      * parameter2_->value( rng, snode_id, target, target_thread );
  }

  double
  value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) override
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
  Parameter* const parameter1_;
  Parameter* const parameter2_;
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
    parameter_returns_int_only_ = parameter1_->returns_int_only() and parameter2_->returns_int_only();
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
    parameter_returns_int_only_ = parameter1_->returns_int_only() and parameter2_->returns_int_only();
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
  value( RngPtr rng, Node* node ) override
  {
    return parameter1_->value( rng, node ) / parameter2_->value( rng, node );
  }

  double
  value( RngPtr rng, index snode_id, Node* target, thread target_thread ) override
  {
    return parameter1_->value( rng, snode_id, target, target_thread )
      / parameter2_->value( rng, snode_id, target, target_thread );
  }

  double
  value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) override
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
  Parameter* const parameter1_;
  Parameter* const parameter2_;
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
    parameter_returns_int_only_ = parameter1_->returns_int_only() and parameter2_->returns_int_only();
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
    parameter_returns_int_only_ = parameter1_->returns_int_only() and parameter2_->returns_int_only();
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
  value( RngPtr rng, Node* node ) override
  {
    return parameter1_->value( rng, node ) + parameter2_->value( rng, node );
  }

  double
  value( RngPtr rng, index snode_id, Node* target, thread target_thread ) override
  {
    return parameter1_->value( rng, snode_id, target, target_thread )
      + parameter2_->value( rng, snode_id, target, target_thread );
  }

  double
  value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) override
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
  Parameter* const parameter1_;
  Parameter* const parameter2_;
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
    parameter_returns_int_only_ = parameter1_->returns_int_only() and parameter2_->returns_int_only();
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
    parameter_returns_int_only_ = parameter1_->returns_int_only() and parameter2_->returns_int_only();
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
  value( RngPtr rng, Node* node ) override
  {
    return parameter1_->value( rng, node ) - parameter2_->value( rng, node );
  }

  double
  value( RngPtr rng, index snode_id, Node* target, thread target_thread ) override
  {
    return parameter1_->value( rng, snode_id, target, target_thread )
      - parameter2_->value( rng, snode_id, target, target_thread );
  }

  double
  value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) override
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
  Parameter* const parameter1_;
  Parameter* const parameter2_;
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
    parameter_returns_int_only_ = p_->returns_int_only();
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
  value( RngPtr rng, Node* node ) override
  {
    return p_->value( rng, node );
  }

  double
  value( RngPtr rng, index snode_id, Node* target, thread target_thread ) override
  {
    return p_->value( rng, snode_id, target, target_thread );
  }

  double
  value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) override
  {
    return p_->value( rng, source_pos, target_pos, layer );
  }

  Parameter*
  clone() const override
  {
    return new ConverseParameter( *this );
  }

protected:
  Parameter* const p_;
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
    parameter_returns_int_only_ = true;
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
  value( RngPtr rng, Node* node ) override
  {
    return compare_( parameter1_->value( rng, node ), parameter2_->value( rng, node ) );
  }

  double
  value( RngPtr rng, index snode_id, Node* target, thread target_thread ) override
  {
    return compare_( parameter1_->value( rng, snode_id, target, target_thread ),
      parameter2_->value( rng, snode_id, target, target_thread ) );
  }

  double
  value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) override
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
  Parameter* const parameter1_;
  Parameter* const parameter2_;

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
    parameter_returns_int_only_ = if_true_->returns_int_only() and if_false_->returns_int_only();
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
    parameter_returns_int_only_ = if_true_->returns_int_only() and if_false_->returns_int_only();
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
  value( RngPtr rng, Node* node ) override
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
  value( RngPtr rng, index snode_id, Node* target, thread target_thread ) override
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
  value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) override
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
  Parameter* const condition_;
  Parameter* const if_true_;
  Parameter* const if_false_;
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
    parameter_returns_int_only_ = p_->returns_int_only() and value_is_integer_( other_value_ );
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
    parameter_returns_int_only_ = p_->returns_int_only() and value_is_integer_( other_value_ );
  }

  ~MinParameter() override
  {
    delete p_;
  }

  /**
   * @returns the value of the parameter.
   */
  double
  value( RngPtr rng, Node* node ) override
  {
    return std::min( p_->value( rng, node ), other_value_ );
  }

  double
  value( RngPtr rng, index snode_id, Node* target, thread target_thread ) override
  {
    return std::min( p_->value( rng, snode_id, target, target_thread ), other_value_ );
  }

  double
  value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) override
  {
    return std::min( p_->value( rng, source_pos, target_pos, layer ), other_value_ );
  }

  Parameter*
  clone() const override
  {
    return new MinParameter( *this );
  }

protected:
  Parameter* const p_;
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
    parameter_returns_int_only_ = p_->returns_int_only() and value_is_integer_( other_value_ );
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
    parameter_returns_int_only_ = p_->returns_int_only() and value_is_integer_( other_value_ );
  }

  ~MaxParameter() override
  {
    delete p_;
  }

  /**
   * @returns the value of the parameter.
   */
  double
  value( RngPtr rng, Node* node ) override
  {
    return std::max( p_->value( rng, node ), other_value_ );
  }

  double
  value( RngPtr rng, index snode_id, Node* target, thread target_thread ) override
  {
    return std::max( p_->value( rng, snode_id, target, target_thread ), other_value_ );
  }

  double
  value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) override
  {
    return std::max( p_->value( rng, source_pos, target_pos, layer ), other_value_ );
  }

  Parameter*
  clone() const override
  {
    return new MaxParameter( *this );
  }

protected:
  Parameter* const p_;
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
    parameter_returns_int_only_ = p_->returns_int_only();
  }

  ~RedrawParameter() override
  {
    delete p_;
  }

  /**
   * @returns the value of the parameter.
   */
  double value( RngPtr rng, Node* node ) override;
  double value( RngPtr rng, index snode_id, Node* target, thread target_thread ) override;
  double value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) override;

  Parameter*
  clone() const override
  {
    return new RedrawParameter( *this );
  }

protected:
  Parameter* const p_;
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
  value( RngPtr rng, Node* node ) override
  {
    return std::exp( p_->value( rng, node ) );
  }

  double
  value( RngPtr rng, index snode_id, Node* target, thread target_thread ) override
  {
    return std::exp( p_->value( rng, snode_id, target, target_thread ) );
  }

  double
  value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) override
  {
    return std::exp( p_->value( rng, source_pos, target_pos, layer ) );
  }

  Parameter*
  clone() const override
  {
    return new ExpParameter( *this );
  }

protected:
  Parameter* const p_;
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
  value( RngPtr rng, Node* node ) override
  {
    return std::sin( p_->value( rng, node ) );
  }

  double
  value( RngPtr rng, index snode_id, Node* target, thread target_thread ) override
  {
    return std::sin( p_->value( rng, snode_id, target, target_thread ) );
  }

  double
  value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) override
  {
    return std::sin( p_->value( rng, source_pos, target_pos, layer ) );
  }

  Parameter*
  clone() const override
  {
    return new SinParameter( *this );
  }

protected:
  Parameter* const p_;
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
  value( RngPtr rng, Node* node ) override
  {
    return std::cos( p_->value( rng, node ) );
  }

  double
  value( RngPtr rng, index snode_id, Node* target, thread target_thread ) override
  {
    return std::cos( p_->value( rng, snode_id, target, target_thread ) );
  }

  double
  value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) override
  {
    return std::cos( p_->value( rng, source_pos, target_pos, layer ) );
  }

  Parameter*
  clone() const override
  {
    return new CosParameter( *this );
  }

protected:
  Parameter* const p_;
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
    parameter_returns_int_only_ = p_->returns_int_only();
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
    parameter_returns_int_only_ = p_->returns_int_only();
  }

  ~PowParameter() override
  {
    delete p_;
  }

  /**
   * @returns the value of the parameter.
   */
  double
  value( RngPtr rng, Node* node ) override
  {
    return std::pow( p_->value( rng, node ), exponent_ );
  }

  double
  value( RngPtr rng, index snode_id, Node* target, thread target_thread ) override
  {
    return std::pow( p_->value( rng, snode_id, target, target_thread ), exponent_ );
  }

  double
  value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) override
  {
    return std::pow( p_->value( rng, source_pos, target_pos, layer ), exponent_ );
  }

  Parameter*
  clone() const override
  {
    return new PowParameter( *this );
  }

protected:
  Parameter* const p_;
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
    , pz_( nullptr )
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
  value( RngPtr, Node* ) override
  {
    throw KernelException( "Cannot get value of DimensionParameter." );
  }

  double
  value( RngPtr, index, Node*, thread ) override
  {
    throw KernelException( "Cannot get value of DimensionParameter." );
  }

  /**
   * Generates a position with values for each dimension generated from their respective parameters.
   * @returns The position, given as an array.
   */
  std::vector< double >
  get_values( RngPtr rng )
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
  Parameter* const px_;
  Parameter* const py_;
  Parameter* const pz_;
};


/**
 * Parameter class representing an exponential distribution applied on a parameter.
 * Can only be used when connecting spatially distributed nodes.
 */
class ExpDistParameter : public Parameter
{
public:
  using Parameter::value;

  /**
   * Construct the parameter from a dictionary of arguments.
   */
  ExpDistParameter( const DictionaryDatum& d );

  /**
   * Copy constructor.
   */
  ExpDistParameter( const ExpDistParameter& p )
    : Parameter( p )
    , p_( p.p_->clone() )
    , inv_beta_( p.inv_beta_ )
  {
    parameter_is_spatial_ = true;
  }

  ~ExpDistParameter() override
  {
    delete p_;
  }

  /**
   * @returns the value of the parameter.
   */
  double
  value( RngPtr, Node* ) override
  {
    throw BadParameterValue( "Exponential distribution parameter can only be used when connecting." );
  }

  double
  value( RngPtr, index, Node*, thread ) override
  {
    throw KernelException(
      "Exponential distribution parameter can only be used when connecting spatially distributed nodes." );
  }

  double value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) override;

  Parameter*
  clone() const override
  {
    return new ExpDistParameter( *this );
  }

protected:
  Parameter* const p_;
  const double inv_beta_;
};


/**
 * Parameter class representing a gaussian distribution applied on a parameter.
 * Can only be used when connecting spatially distributed nodes.
 */
class GaussianParameter : public Parameter
{
public:
  using Parameter::value;

  /**
   * Construct the parameter from a dictionary of arguments.
   */
  GaussianParameter( const DictionaryDatum& d );

  /**
   * Copy constructor.
   */
  GaussianParameter( const GaussianParameter& p )
    : Parameter( p )
    , p_( p.p_->clone() )
    , mean_( p.mean_ )
    , inv_two_std2_( p.inv_two_std2_ )
  {
    parameter_is_spatial_ = true;
  }

  ~GaussianParameter() override
  {
    delete p_;
  }

  /**
   * @returns the value of the parameter.
   */
  double
  value( RngPtr, Node* ) override
  {
    throw BadParameterValue( "Gaussian distribution parameter can only be used when connecting." );
  }

  double
  value( RngPtr, index, Node*, thread ) override
  {
    throw KernelException(
      "Gaussian distribution parameter can only be used when connecting spatially distributed nodes." );
  }

  double value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) override;

  Parameter*
  clone() const override
  {
    return new GaussianParameter( *this );
  }

protected:
  Parameter* const p_;
  const double mean_;
  const double inv_two_std2_;
};


/**
 * Parameter class representing a gaussian distribution in two dimensions applied on a parameter.
 * Can only be used when connecting spatially distributed nodes.
 */
class Gaussian2DParameter : public Parameter
{
public:
  using Parameter::value;

  /**
   * Construct the parameter from a dictionary of arguments.
   */
  Gaussian2DParameter( const DictionaryDatum& d );

  /**
   * Copy constructor.
   */
  Gaussian2DParameter( const Gaussian2DParameter& p )
    : Parameter( p )
    , px_( p.px_->clone() )
    , py_( p.py_->clone() )
    , mean_x_( p.mean_x_ )
    , mean_y_( p.mean_y_ )
    , x_term_const_( p.x_term_const_ )
    , y_term_const_( p.y_term_const_ )
    , xy_term_const_( p.xy_term_const_ )
  {
    parameter_is_spatial_ = true;
  }

  ~Gaussian2DParameter() override
  {
    delete px_;
    delete py_;
  }

  /**
   * @returns the value of the parameter.
   */
  double
  value( RngPtr, Node* ) override
  {
    throw BadParameterValue( "Gaussian 2D parameter can only be used when connecting." );
  }

  double
  value( RngPtr, index, Node*, thread ) override
  {
    throw KernelException( "Gaussian 2D parameter can only be used when connecting spatially distributed nodes." );
  }

  double value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) override;

  Parameter*
  clone() const override
  {
    return new Gaussian2DParameter( *this );
  }

protected:
  Parameter* const px_;
  Parameter* const py_;
  const double mean_x_;
  const double mean_y_;
  const double x_term_const_;
  const double y_term_const_;
  const double xy_term_const_;
};


/**
 * Parameter class representing a gamma distribution applied on a parameter.
 * Can only be used when connecting spatially distributed nodes.
 */
class GammaParameter : public Parameter
{
public:
  using Parameter::value;

  /**
   * Construct the parameter from a dictionary of arguments.
   */
  GammaParameter( const DictionaryDatum& d );

  /**
   * Copy constructor.
   */
  GammaParameter( const GammaParameter& p )
    : Parameter( p )
    , p_( p.p_->clone() )
    , kappa_( p.kappa_ )
    , inv_theta_( p.inv_theta_ )
    , delta_( p.delta_ )
  {
    parameter_is_spatial_ = true;
  }

  ~GammaParameter() override
  {
    delete p_;
  }

  /**
   * @returns the value of the parameter.
   */
  double
  value( RngPtr, Node* ) override
  {
    throw BadParameterValue( "Gamma distribution parameter can only be used when connecting." );
  }

  double
  value( RngPtr, index, Node*, thread ) override
  {
    throw KernelException(
      "Gamma distribution parameter can only be used when connecting spatially distributed nodes." );
  }

  double value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer ) override;

  Parameter*
  clone() const override
  {
    return new GammaParameter( *this );
  }

protected:
  Parameter* const p_;
  const double kappa_;
  const double inv_theta_;
  const double delta_;
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


inline bool
Parameter::returns_int_only() const
{
  return parameter_returns_int_only_;
}

inline bool
Parameter::value_is_integer_( const double value ) const
{
  // Here fmod calculates the remainder of the division operation x/y. By using y=1.0,
  // the remainder is the fractional part of the value. If the fractional part
  // is zero, the value is an integer.
  return std::fmod( value, static_cast< double >( 1.0 ) ) == 0.0;
}

} // namespace nest

#endif
