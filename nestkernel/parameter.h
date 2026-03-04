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

// Includes from nestkernel:
#include "node.h"
#include "node_collection.h"
#include "random_generators.h"


namespace nest
{
class Parameter;
using ParameterPTR = std::shared_ptr< Parameter >;

class AbstractLayer;
class Dictionary;

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
   * @returns array of result values, one per position in the vector.
   */
  std::vector< double > apply( const NodeCollectionPTR&, const std::vector< std::vector< double > >& );

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
  explicit ConstantParameter( double value );

  /**
   * Creates a ConstantParameter with the value specified in a dictionary.
   *
   * @param d dictionary with the parameter value
   *
   * The dictionary must include the following entry:
   * value - constant value of this parameter
   */
  ConstantParameter( const Dictionary& d );

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
  UniformParameter( const Dictionary& d );

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
  UniformIntParameter( const Dictionary& d );

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
  NormalParameter( const Dictionary& d );

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
  LognormalParameter( const Dictionary& d );

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
  ExponentialParameter( const Dictionary& d );

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
  NodePosParameter( const Dictionary& d );

  double value( RngPtr, Node* node ) override;

  double value( RngPtr,
    const std::vector< double >& source_pos,
    const std::vector< double >& target_pos,
    const AbstractLayer&,
    Node* ) override;

private:
  long dimension_;
  int synaptic_endpoint_;

  double get_node_pos_( Node* node ) const;
};


/**
 * Parameter representing the spatial distance between two nodes, optionally in a specific dimension.
 */
class SpatialDistanceParameter : public Parameter
{
public:
  SpatialDistanceParameter( const Dictionary& d );

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
  ProductParameter( const ParameterPTR m1, const ParameterPTR m2 );

  ProductParameter( const ProductParameter& p );

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
  ParameterPTR const parameter1_;
  ParameterPTR const parameter2_;
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
  QuotientParameter( ParameterPTR m1, ParameterPTR m2 );

  QuotientParameter( const QuotientParameter& p );

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
  ParameterPTR const parameter1_;
  ParameterPTR const parameter2_;
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
  SumParameter( ParameterPTR m1, ParameterPTR m2 );

  SumParameter( const SumParameter& p );

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
  ParameterPTR const parameter1_;
  ParameterPTR const parameter2_;
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
  DifferenceParameter( ParameterPTR m1, ParameterPTR m2 );

  DifferenceParameter( const DifferenceParameter& p );

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
  ParameterPTR const parameter1_;
  ParameterPTR const parameter2_;
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
  ComparingParameter( ParameterPTR m1, ParameterPTR m2, const Dictionary& d );

  ComparingParameter( const ComparingParameter& p );

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
  ParameterPTR const parameter1_;
  ParameterPTR const parameter2_;

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
  ConditionalParameter( ParameterPTR condition, ParameterPTR if_true, ParameterPTR if_false );

  ConditionalParameter( const ConditionalParameter& p );

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
  ParameterPTR const condition_;
  ParameterPTR const if_true_;
  ParameterPTR const if_false_;
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
  MinParameter( ParameterPTR p, const double other_value );

  MinParameter( const MinParameter& p );

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
  ParameterPTR const p_;
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
  MaxParameter( ParameterPTR p, const double other_value );

  MaxParameter( const MaxParameter& p );

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
  ParameterPTR const p_;
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
  RedrawParameter( ParameterPTR p, const double min, const double max );

  RedrawParameter( const RedrawParameter& p );

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
  ParameterPTR const p_;
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
  ExpParameter( ParameterPTR p );

  ExpParameter( const ExpParameter& p );

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
  ParameterPTR const p_;
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
  SinParameter( ParameterPTR p );

  SinParameter( const SinParameter& p );

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
  ParameterPTR const p_;
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
  CosParameter( ParameterPTR p );

  CosParameter( const CosParameter& p );

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
  ParameterPTR const p_;
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
  PowParameter( ParameterPTR p, const double exponent );

  PowParameter( const PowParameter& p );

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
  ParameterPTR const p_;
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
  DimensionParameter( ParameterPTR px, ParameterPTR py );

  DimensionParameter( ParameterPTR px, ParameterPTR py, ParameterPTR pz );

  DimensionParameter( const DimensionParameter& p );

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
  ParameterPTR const px_;
  ParameterPTR const py_;
  ParameterPTR const pz_;
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
  ExpDistParameter( const Dictionary& d );

  ExpDistParameter( const ExpDistParameter& p );

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
  ParameterPTR const p_;
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
  GaussianParameter( const Dictionary& d );

  GaussianParameter( const GaussianParameter& p );

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
  ParameterPTR const p_;
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
  Gaussian2DParameter( const Dictionary& d );

  Gaussian2DParameter( const Gaussian2DParameter& p );

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
  ParameterPTR const px_;
  ParameterPTR const py_;
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
  GaborParameter( const Dictionary& d );

  /**
   * Copy constructor.
   */
  GaborParameter( const GaborParameter& p );

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
  GammaParameter( const Dictionary& d );

  GammaParameter( const GammaParameter& p );

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
  ParameterPTR const p_;
  const double kappa_;
  const double inv_theta_;
  const double delta_;
};

/**
 * Create the product of one parameter with another.
 *
 * @returns a new dynamically allocated parameter.
 */
ParameterPTR multiply_parameter( const ParameterPTR first, const ParameterPTR second );

/**
 * Create the quotient of one parameter with another.
 *
 * @returns a new dynamically allocated parameter.
 */
ParameterPTR divide_parameter( const ParameterPTR first, const ParameterPTR second );

/**
 * Create the sum of one parameter with another.
 *
 * @returns a new dynamically allocated parameter.
 */
ParameterPTR add_parameter( const ParameterPTR first, const ParameterPTR second );

/**
 * Create the difference between one parameter and another.
 *
 * @returns a new dynamically allocated parameter.
 */
ParameterPTR subtract_parameter( const ParameterPTR first, const ParameterPTR second );

/**
 * Create comparison of one parameter with another.
 *
 * @returns a new dynamically allocated parameter.
 */
ParameterPTR compare_parameter( const ParameterPTR first, const ParameterPTR second, const Dictionary& d );

/**
 * Create a parameter that chooses between two other parameters,
 * based on a given condition parameter.
 *
 * The resulting value of the condition parameter
 * is treated as a bool, meaning that a zero value evaluates as false, and all other values
 * evaluate as true.
 * @returns a new dynamically allocated parameter.
 */
ParameterPTR
conditional_parameter( const ParameterPTR condition, const ParameterPTR if_true, const ParameterPTR if_false );

/**
 * Create parameter whose value is the minimum of a given parameter's value and the given value.
 *
 * @returns a new dynamically allocated parameter.
 */
ParameterPTR min_parameter( const ParameterPTR parameter, const double other );

/**
 * Create parameter whose value is the maximum of a given parameter's value and the given value.
 *
 * @returns a new dynamically allocated parameter.
 */
ParameterPTR max_parameter( const ParameterPTR parameter, const double other );

/**
 * Create parameter redrawing the value if the value of a parameter is outside the set limits.
 *
 * @returns a new dynamically allocated parameter.
 */
ParameterPTR redraw_parameter( const ParameterPTR parameter, const double min, const double max );

/**
 * Create the exponential of a parameter.
 *
 * @returns a new dynamically allocated parameter.
 */
ParameterPTR exp_parameter( const ParameterPTR parameter );

/**
 * Create the sine of a parameter.
 *
 * @returns a new dynamically allocated parameter.
 */
ParameterPTR sin_parameter( const ParameterPTR parameter );

/**
 * Create the cosine of a parameter.
 *
 * @returns a new dynamically allocated parameter.
 */
ParameterPTR cos_parameter( const ParameterPTR parameter );

/**
 * Create a parameter raised to the power of an exponent.
 *
 * @returns a new dynamically allocated parameter.
 */
ParameterPTR pow_parameter( const ParameterPTR parameter, const double exponent );

/**
 * Create a parameter that can generate position vectors from a given set of parameters.
 *
 * @returns a new dynamically allocated parameter.
 */
ParameterPTR dimension_parameter( const ParameterPTR x_parameter, const ParameterPTR y_parameter );

ParameterPTR
dimension_parameter( const ParameterPTR x_parameter, const ParameterPTR y_parameter, const ParameterPTR z_parameter );


} // namespace nest

#endif
