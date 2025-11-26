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
#include <cmath>
#include <limits>

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
   * Creates a Parameter, with optionally specified attributes.
   *
   * @param is_spatial true if the Parameter contains spatial elements
   * @param returns_int_only true if the value of the parameter can only be an integer
   */
  Parameter( bool is_spatial = false, bool returns_int_only = false )
    : is_spatial_( is_spatial )
    , returns_int_only_( returns_int_only )
  {
  }

  explicit Parameter( const Parameter& p ) = default;

  virtual ~Parameter() = default;

  /**
   * Generates a value based on parameter specifications and arguments.
   *
   * Used when getting a parameter value based on random values or node attributes,
   * like position. Note that not all parameters support all overloaded versions.
   * @param rng pointer to the random number generator
   * @param node pointer to the node, used when the node position is relevant
   * @returns the value of the parameter.
   */
  virtual double value( RngPtr rng, Node* node ) = 0;

  /**
   * Generates a value based on parameter specifications and arguments.
   *
   * Used when connecting spatial nodes. Note that not all parameters
   * support all overloaded versions.
   * @param rng pointer to the random number generator
   * @param source_pos position of the source node
   * @param target_pos position of the target node
   * @param layer spatial layer
   * @param node target node, required for normal and lognormal parameters
   * @returns the value of the parameter.
   */
  virtual double value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer,
    Node* node );

  /**
   * Applies a parameter on a single-node ID NodeCollection and given array of positions.
   *
   * @returns array of result values, one per position in the TokenArray.
   */
  std::vector< double > apply( const NodeCollectionPTR&, const TokenArray& );

  /**
   * Check if the Parameter is based on spatial properties.
   *
   * @returns true if the Parameter is based on spatial properties, false otherwise.
   */
  bool is_spatial() const;

  /**
   * Check if the Parameter only returns integer values.
   *
   * @returns true if the Parameter only returns integers, false otherwise.
   */
  bool returns_int_only() const;

protected:
  bool is_spatial_ { false };
  bool returns_int_only_ { false };

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
   *
   * @param value parameter value
   */
  explicit ConstantParameter( double value )
    : value_( value )
  {
    returns_int_only_ = value_is_integer_( value_ );
  }

  /**
   * Creates a ConstantParameter with the value specified in a dictionary.
   *
   * @param d dictionary with the parameter value
   *
   * The dictionary must include the following entry:
   * value - constant value of this parameter
   */
  ConstantParameter( const DictionaryDatum& d )
  {
    value_ = getValue< double >( d, "value" );
    returns_int_only_ = value_is_integer_( value_ );
  }

  ~ConstantParameter() override = default;

  /**
   * @returns the constant value of this parameter.
   */
  double value( RngPtr, Node* ) override;

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
   *
   * @param d dictionary with parameter specifications
   *
   * The dictionary can include the following entries:
   * min - minimum value
   * max - maximum value
   */
  UniformParameter( const DictionaryDatum& d )
    : lower_( 0.0 )
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

  double value( RngPtr rng, Node* ) override;

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
   *
   * @param d dictionary with parameter specifications
   *
   * The dictionary can include the following entries:
   * max - maximum value
   */
  UniformIntParameter( const DictionaryDatum& d )
    : Parameter( false, true )
    , max_( 1.0 )
  {
    updateValue< long >( d, names::max, max_ );
    if ( max_ <= 0 )
    {
      throw BadProperty( "nest::UniformIntParameter: max > 0 required." );
    }
  }

  double value( RngPtr rng, Node* ) override;

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
   *
   * @param d dictionary with parameter specifications
   *
   * The dictionary can include the following entries:
   * mean - mean value
   * std - standard deviation
   */
  NormalParameter( const DictionaryDatum& d );

  double value( RngPtr rng, Node* node ) override;

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
   *
   * @param d dictionary with parameter specifications
   *
   * The dictionary can include the following entries:
   * mean - mean value of logarithm
   * sigma - standard distribution of logarithm
   */
  LognormalParameter( const DictionaryDatum& d );

  double value( RngPtr rng, Node* node ) override;

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
   *
   * @param d dictionary with parameter specifications
   *
   * The dictionary can include the following entries:
   * beta - the scale parameter
   */
  ExponentialParameter( const DictionaryDatum& d )
    : beta_( 1.0 )
  {
    updateValue< double >( d, names::beta, beta_ );
  }

  double value( RngPtr rng, Node* ) override;

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
   *
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
    : Parameter( true )
    , dimension_( 0 )
    , synaptic_endpoint_( 0 )
  {
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

  double value( RngPtr, Node* node ) override;

  double value( RngPtr,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer&,
    Node* ) override;

private:
  int dimension_;
  int synaptic_endpoint_;

  double get_node_pos_( Node* node ) const;
};


/**
 * Parameter representing the spatial distance between two nodes, optionally in a specific dimension.
 */
class SpatialDistanceParameter : public Parameter
{
public:
  SpatialDistanceParameter( const DictionaryDatum& d )
    : Parameter( true )
    , dimension_( 0 )
  {
    updateValue< long >( d, names::dimension, dimension_ );
    if ( dimension_ < 0 )
    {
      throw BadParameterValue( "Spatial distance parameter dimension cannot be negative." );
    }
  }

  double value( RngPtr, Node* ) override;

  double value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer,
    Node* ) override;

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
   * Construct the product of the two given parameters.
   *
   * Copies are made of the supplied Parameter objects.
   */
  ProductParameter( const std::shared_ptr< Parameter > m1, const std::shared_ptr< Parameter > m2 )
    : Parameter( m1->is_spatial() or m2->is_spatial(), m1->returns_int_only() and m2->returns_int_only() )
    , parameter1_( m1 )
    , parameter2_( m2 )
  {
  }

  ProductParameter( const ProductParameter& p )
    : Parameter( p )
    , parameter1_( p.parameter1_ )
    , parameter2_( p.parameter2_ )
  {
  }

  /**
   * @returns the value of the product.
   */
  double value( RngPtr rng, Node* node ) override;

  double value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer,
    Node* node ) override;

protected:
  std::shared_ptr< Parameter > const parameter1_;
  std::shared_ptr< Parameter > const parameter2_;
};

/**
 * Parameter class representing the quotient of two parameters.
 */
class QuotientParameter : public Parameter
{
public:
  /**
   * Construct the quotient of two given parameters.
   *
   * Copies are made of the supplied Parameter objects.
   */
  QuotientParameter( std::shared_ptr< Parameter > m1, std::shared_ptr< Parameter > m2 )
    : Parameter( m1->is_spatial() or m2->is_spatial(), m1->returns_int_only() and m2->returns_int_only() )
    , parameter1_( m1 )
    , parameter2_( m2 )
  {
  }

  QuotientParameter( const QuotientParameter& p )
    : Parameter( p )
    , parameter1_( p.parameter1_ )
    , parameter2_( p.parameter2_ )
  {
  }

  /**
   * @returns the value of the product.
   */
  double value( RngPtr rng, Node* node ) override;

  double value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer,
    Node* node ) override;

protected:
  std::shared_ptr< Parameter > const parameter1_;
  std::shared_ptr< Parameter > const parameter2_;
};

/**
 * Parameter class representing the sum of two parameters
 */
class SumParameter : public Parameter
{
public:
  /**
   * Construct the sum of two given parameters.
   *
   * Copies are made of the supplied Parameter objects.
   */
  SumParameter( std::shared_ptr< Parameter > m1, std::shared_ptr< Parameter > m2 )
    : Parameter( m1->is_spatial() or m2->is_spatial(), m1->returns_int_only() and m2->returns_int_only() )
    , parameter1_( m1 )
    , parameter2_( m2 )
  {
  }

  SumParameter( const SumParameter& p )
    : Parameter( p )
    , parameter1_( p.parameter1_ )
    , parameter2_( p.parameter2_ )
  {
  }

  /**
   * @returns the value of the sum.
   */
  double value( RngPtr rng, Node* node ) override;

  double value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer,
    Node* node ) override;

protected:
  std::shared_ptr< Parameter > const parameter1_;
  std::shared_ptr< Parameter > const parameter2_;
};

/**
 * Parameter class representing the difference of two parameters
 */
class DifferenceParameter : public Parameter
{
public:
  /**
   * Construct the difference of two given parameters.
   *
   * Copies are made of the supplied Parameter objects.
   */
  DifferenceParameter( std::shared_ptr< Parameter > m1, std::shared_ptr< Parameter > m2 )
    : Parameter( m1->is_spatial() or m2->is_spatial(), m1->returns_int_only() and m2->returns_int_only() )
    , parameter1_( m1 )
    , parameter2_( m2 )
  {
  }

  DifferenceParameter( const DifferenceParameter& p )
    : Parameter( p )
    , parameter1_( p.parameter1_ )
    , parameter2_( p.parameter2_ )
  {
  }

  /**
   * @returns the value of the difference.
   */
  double value( RngPtr rng, Node* node ) override;

  double value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer,
    Node* node ) override;

protected:
  std::shared_ptr< Parameter > const parameter1_;
  std::shared_ptr< Parameter > const parameter2_;
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
  ComparingParameter( std::shared_ptr< Parameter > m1, std::shared_ptr< Parameter > m2, const DictionaryDatum& d )
    : Parameter( m1->is_spatial() or m2->is_spatial(), true )
    , parameter1_( m1 )
    , parameter2_( m2 )
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
  }

  ComparingParameter( const ComparingParameter& p )
    : Parameter( p )
    , parameter1_( p.parameter1_ )
    , parameter2_( p.parameter2_ )
    , comparator_( p.comparator_ )
  {
  }

  /**
   * @returns the result of the comparison, bool given as a double.
   */
  double value( RngPtr rng, Node* node ) override;

  double value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer,
    Node* node ) override;

protected:
  std::shared_ptr< Parameter > const parameter1_;
  std::shared_ptr< Parameter > const parameter2_;

private:
  bool compare_( double value_a, double value_b ) const;

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
  ConditionalParameter( std::shared_ptr< Parameter > condition,
    std::shared_ptr< Parameter > if_true,
    std::shared_ptr< Parameter > if_false )
    : Parameter( condition->is_spatial() or if_true->is_spatial() or if_false->is_spatial(),
      if_true->returns_int_only() and if_false->returns_int_only() )
    , condition_( condition )
    , if_true_( if_true )
    , if_false_( if_false )
  {
  }

  ConditionalParameter( const ConditionalParameter& p )
    : Parameter( p )
    , condition_( p.condition_ )
    , if_true_( p.if_true_ )
    , if_false_( p.if_false_ )
  {
  }

  /**
   * @returns the value chosen by the comparison.
   */
  double value( RngPtr rng, Node* node ) override;
  double value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer,
    Node* node ) override;

protected:
  std::shared_ptr< Parameter > const condition_;
  std::shared_ptr< Parameter > const if_true_;
  std::shared_ptr< Parameter > const if_false_;
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
  MinParameter( std::shared_ptr< Parameter > p, const double other_value )
    : Parameter( p->is_spatial(), p->returns_int_only() and value_is_integer_( other_value ) )
    , p_( p )
    , other_value_( other_value )
  {
    assert( is_spatial_ == p->is_spatial() );
  }

  MinParameter( const MinParameter& p )
    : Parameter( p )
    , p_( p.p_ )
    , other_value_( p.other_value_ )
  {
  }

  /**
   * @returns the value of the parameter.
   */
  double value( RngPtr rng, Node* node ) override;

  double value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer,
    Node* node ) override;

protected:
  std::shared_ptr< Parameter > const p_;
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
  MaxParameter( std::shared_ptr< Parameter > p, const double other_value )
    : Parameter( p->is_spatial(), p->returns_int_only() and value_is_integer_( other_value ) )
    , p_( p )
    , other_value_( other_value )
  {
  }

  MaxParameter( const MaxParameter& p )
    : Parameter( p )
    , p_( p.p_ )
    , other_value_( p.other_value_ )
  {
  }

  /**
   * @returns the value of the parameter.
   */
  double value( RngPtr rng, Node* node ) override;

  double value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer,
    Node* node ) override;

protected:
  std::shared_ptr< Parameter > const p_;
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
  RedrawParameter( std::shared_ptr< Parameter > p, const double min, const double max );

  RedrawParameter( const RedrawParameter& p )
    : Parameter( p )
    , p_( p.p_ )
    , min_( p.min_ )
    , max_( p.max_ )
    , max_redraws_( p.max_redraws_ )
  {
  }

  /**
   * @returns the value of the parameter.
   */
  double value( RngPtr rng, Node* node ) override;
  double value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer,
    Node* node ) override;

protected:
  std::shared_ptr< Parameter > const p_;
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
  ExpParameter( std::shared_ptr< Parameter > p )
    : Parameter( p->is_spatial() )
    , p_( p )
  {
  }

  ExpParameter( const ExpParameter& p )
    : Parameter( p )
    , p_( p.p_ )
  {
  }

  /**
   * @returns the value of the parameter.
   */
  double value( RngPtr rng, Node* node ) override;

  double value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer,
    Node* node ) override;

protected:
  std::shared_ptr< Parameter > const p_;
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
  SinParameter( std::shared_ptr< Parameter > p )
    : Parameter( p->is_spatial() )
    , p_( p )
  {
  }

  SinParameter( const SinParameter& p )
    : Parameter( p )
    , p_( p.p_ )
  {
  }

  /**
   * @returns the value of the parameter.
   */
  double value( RngPtr rng, Node* node ) override;

  double value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer,
    Node* node ) override;

protected:
  std::shared_ptr< Parameter > const p_;
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
  CosParameter( std::shared_ptr< Parameter > p )
    : Parameter( p->is_spatial() )
    , p_( p )
  {
  }

  CosParameter( const CosParameter& p )
    : Parameter( p )
    , p_( p.p_ )
  {
  }

  /**
   * @returns the value of the parameter.
   */
  double value( RngPtr rng, Node* node ) override;

  double value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer,
    Node* node ) override;

protected:
  std::shared_ptr< Parameter > const p_;
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
  PowParameter( std::shared_ptr< Parameter > p, const double exponent )
    : Parameter( p->is_spatial(), p->returns_int_only() )
    , p_( p )
    , exponent_( exponent )
  {
  }

  PowParameter( const PowParameter& p )
    : Parameter( p )
    , p_( p.p_ )
    , exponent_( p.exponent_ )
  {
  }

  /**
   * @returns the value of the parameter.
   */
  double value( RngPtr rng, Node* node ) override;

  double value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer,
    Node* node ) override;

protected:
  std::shared_ptr< Parameter > const p_;
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
   * Construct the Parameter with one given Parameter per dimension.
   *
   * A copy is made of the supplied Parameter objects.
   */
  DimensionParameter( std::shared_ptr< Parameter > px, std::shared_ptr< Parameter > py )
    : Parameter( true )
    , num_dimensions_( 2 )
    , px_( px )
    , py_( py )
    , pz_( nullptr )
  {
  }

  DimensionParameter( std::shared_ptr< Parameter > px,
    std::shared_ptr< Parameter > py,
    std::shared_ptr< Parameter > pz )
    : Parameter( true )
    , num_dimensions_( 3 )
    , px_( px )
    , py_( py )
    , pz_( pz )
  {
  }

  DimensionParameter( const DimensionParameter& p )
    : Parameter( p )
    , num_dimensions_( p.num_dimensions_ )
    , px_( p.px_ )
    , py_( p.py_ )
    , pz_( p.pz_ )
  {
  }

  /**
   * The DimensionParameter has no double value, so this method will always throw.
   */
  double value( RngPtr, Node* ) override;

  /**
   * Generates a position with values for each dimension generated from their respective parameters.
   *
   * @returns The position, given as an array.
   */
  std::vector< double > get_values( RngPtr rng );

  int get_num_dimensions() const;

protected:
  int num_dimensions_;
  std::shared_ptr< Parameter > const px_;
  std::shared_ptr< Parameter > const py_;
  std::shared_ptr< Parameter > const pz_;
};


/**
 * Parameter class representing an exponential distribution applied on a parameter.
 *
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

  ExpDistParameter( const ExpDistParameter& p )
    : Parameter( p )
    , p_( p.p_ )
    , inv_beta_( p.inv_beta_ )
  {
    assert( is_spatial_ == p.is_spatial() );
  }

  /**
   * @returns the value of the parameter.
   */
  double value( RngPtr, Node* ) override;

  double value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer,
    Node* node ) override;

protected:
  std::shared_ptr< Parameter > const p_;
  const double inv_beta_;
};


/**
 * Parameter class representing a gaussian distribution applied on a parameter.
 *
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

  GaussianParameter( const GaussianParameter& p )
    : Parameter( p )
    , p_( p.p_ )
    , mean_( p.mean_ )
    , inv_two_std2_( p.inv_two_std2_ )
  {
  }

  /**
   * @returns the value of the parameter.
   */
  double value( RngPtr, Node* ) override;

  double value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer,
    Node* node ) override;

protected:
  std::shared_ptr< Parameter > const p_;
  const double mean_;
  const double inv_two_std2_;
};


/**
 * Parameter class representing a gaussian distribution in two dimensions applied on a parameter.
 *
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

  Gaussian2DParameter( const Gaussian2DParameter& p )
    : Parameter( p )
    , px_( p.px_ )
    , py_( p.py_ )
    , mean_x_( p.mean_x_ )
    , mean_y_( p.mean_y_ )
    , x_term_const_( p.x_term_const_ )
    , y_term_const_( p.y_term_const_ )
    , xy_term_const_( p.xy_term_const_ )
  {
  }

  /**
   * @returns the value of the parameter.
   */
  double value( RngPtr, Node* ) override;

  double value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer,
    Node* node ) override;

protected:
  std::shared_ptr< Parameter > const px_;
  std::shared_ptr< Parameter > const py_;
  const double mean_x_;
  const double mean_y_;
  const double x_term_const_;
  const double y_term_const_;
  const double xy_term_const_;
};

class GaborParameter : public Parameter
{
public:
  using Parameter::value;

  /**
   * Construct the parameter from a dictionary of arguments.
   */
  GaborParameter( const DictionaryDatum& d );

  /**
   * Copy constructor.
   */
  GaborParameter( const GaborParameter& p )
    : Parameter( p )
    , px_( p.px_ )
    , py_( p.py_ )
    , cos_( p.cos_ )
    , sin_( p.sin_ )
    , gamma_( p.gamma_ )
    , inv_two_std2_( p.inv_two_std2_ )
    , lambda_( p.lambda_ )
    , psi_( p.psi_ )
  {
  }

  /**
   * @returns the value of the parameter.
   */
  double value( RngPtr, Node* ) override;

  double value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer,
    Node* node ) override;

protected:
  std::shared_ptr< Parameter > const px_;
  std::shared_ptr< Parameter > const py_;
  const double cos_;
  const double sin_;
  const double gamma_;
  const double inv_two_std2_;
  const double lambda_;
  const double psi_;
};


/**
 * Parameter class representing a gamma distribution applied on a parameter.
 *
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

  GammaParameter( const GammaParameter& p )
    : Parameter( p )
    , p_( p.p_ )
    , kappa_( p.kappa_ )
    , inv_theta_( p.inv_theta_ )
    , delta_( p.delta_ )
  {
  }

  /**
   * @returns the value of the parameter.
   */
  double value( RngPtr, Node* ) override;

  double value( RngPtr rng,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer& layer,
    Node* node ) override;

protected:
  std::shared_ptr< Parameter > const p_;
  const double kappa_;
  const double inv_theta_;
  const double delta_;
};

/**
 * Create the product of one parameter with another.
 *
 * @returns a new dynamically allocated parameter.
 */
std::shared_ptr< Parameter > multiply_parameter( const std::shared_ptr< Parameter > first,
  const std::shared_ptr< Parameter > second );

/**
 * Create the quotient of one parameter with another.
 *
 * @returns a new dynamically allocated parameter.
 */
std::shared_ptr< Parameter > divide_parameter( const std::shared_ptr< Parameter > first,
  const std::shared_ptr< Parameter > second );

/**
 * Create the sum of one parameter with another.
 *
 * @returns a new dynamically allocated parameter.
 */
std::shared_ptr< Parameter > add_parameter( const std::shared_ptr< Parameter > first,
  const std::shared_ptr< Parameter > second );

/**
 * Create the difference between one parameter and another.
 *
 * @returns a new dynamically allocated parameter.
 */
std::shared_ptr< Parameter > subtract_parameter( const std::shared_ptr< Parameter > first,
  const std::shared_ptr< Parameter > second );

/**
 * Create comparison of one parameter with another.
 *
 * @returns a new dynamically allocated parameter.
 */
std::shared_ptr< Parameter > compare_parameter( const std::shared_ptr< Parameter > first,
  const std::shared_ptr< Parameter > second,
  const DictionaryDatum& d );

/**
 * Create a parameter that chooses between two other parameters,
 * based on a given condition parameter.
 *
 * The resulting value of the condition parameter
 * is treated as a bool, meaning that a zero value evaluates as false, and all other values
 * evaluate as true.
 * @returns a new dynamically allocated parameter.
 */
std::shared_ptr< Parameter > conditional_parameter( const std::shared_ptr< Parameter > condition,
  const std::shared_ptr< Parameter > if_true,
  const std::shared_ptr< Parameter > if_false );

/**
 * Create parameter whose value is the minimum of a given parameter's value and the given value.
 *
 * @returns a new dynamically allocated parameter.
 */
std::shared_ptr< Parameter > min_parameter( const std::shared_ptr< Parameter > parameter, const double other );

/**
 * Create parameter whose value is the maximum of a given parameter's value and the given value.
 *
 * @returns a new dynamically allocated parameter.
 */
std::shared_ptr< Parameter > max_parameter( const std::shared_ptr< Parameter > parameter, const double other );

/**
 * Create parameter redrawing the value if the value of a parameter is outside the set limits.
 *
 * @returns a new dynamically allocated parameter.
 */
std::shared_ptr< Parameter >
redraw_parameter( const std::shared_ptr< Parameter > parameter, const double min, const double max );

/**
 * Create the exponential of a parameter.
 *
 * @returns a new dynamically allocated parameter.
 */
std::shared_ptr< Parameter > exp_parameter( const std::shared_ptr< Parameter > parameter );

/**
 * Create the sine of a parameter.
 *
 * @returns a new dynamically allocated parameter.
 */
std::shared_ptr< Parameter > sin_parameter( const std::shared_ptr< Parameter > parameter );

/**
 * Create the cosine of a parameter.
 *
 * @returns a new dynamically allocated parameter.
 */
std::shared_ptr< Parameter > cos_parameter( const std::shared_ptr< Parameter > parameter );

/**
 * Create a parameter raised to the power of an exponent.
 *
 * @returns a new dynamically allocated parameter.
 */
std::shared_ptr< Parameter > pow_parameter( const std::shared_ptr< Parameter > parameter, const double exponent );

/**
 * Create a parameter that can generate position vectors from a given set of parameters.
 *
 * @returns a new dynamically allocated parameter.
 */
std::shared_ptr< Parameter > dimension_parameter( const std::shared_ptr< Parameter > x_parameter,
  const std::shared_ptr< Parameter > y_parameter );

std::shared_ptr< Parameter > dimension_parameter( const std::shared_ptr< Parameter > x_parameter,
  const std::shared_ptr< Parameter > y_parameter,
  const std::shared_ptr< Parameter > z_parameter );


} // namespace nest

#endif
