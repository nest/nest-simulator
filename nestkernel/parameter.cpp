/*
 *  parameter.cpp
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

#include <cmath>

#include "nest.h"
#include "node.h"
#include "node_collection.h"
#include "parameter.h"
#include "spatial.h"


namespace nest
{

std::vector< double >
Parameter::apply( const NodeCollectionPTR& nc, const TokenArray& token_array )
{
  std::vector< double > result;
  result.reserve( token_array.size() );
  RngPtr rng = get_rank_synced_rng();

  // Get source layer from the NodeCollection
  auto source_metadata = nc->get_metadata();
  if ( not source_metadata.get() )
  {
    throw KernelException( "apply: not meta" );
  }
  auto const* const source_layer_metadata = dynamic_cast< LayerMetadata const* >( source_metadata.get() );
  if ( not source_layer_metadata )
  {
    throw KernelException( "apply: not layer_meta" );
  }
  AbstractLayerPTR source_layer = source_layer_metadata->get_layer();
  if ( not source_layer.get() )
  {
    throw KernelException( "apply: not valid layer" );
  }

  assert( nc->size() == 1 );
  const size_t source_lid = nc->operator[]( 0 ) - source_metadata->get_first_node_id();
  std::vector< double > source_pos = source_layer->get_position_vector( source_lid );

  // For each position, calculate the displacement, then calculate the parameter value
  for ( auto&& token : token_array )
  {
    std::vector< double > target_pos = getValue< std::vector< double > >( token );
    if ( target_pos.size() != source_pos.size() )
    {
      throw BadProperty(
        String::compose( "Parameter apply: Target position has %1 dimensions, but source position has %2 dimensions.",
          target_pos.size(),
          source_pos.size() ) );
    }
    auto value = this->value( rng, source_pos, target_pos, *source_layer.get(), nullptr );
    result.push_back( value );
  }
  return result;
}

NormalParameter::NormalParameter( const DictionaryDatum& d )
  : mean_( 0.0 )
  , std_( 1.0 )
{
  updateValue< double >( d, names::mean, mean_ );
  updateValue< double >( d, names::std, std_ );
  if ( std_ <= 0 )
  {
    throw BadProperty( "nest::NormalParameter: std > 0 required." );
  }
  normal_distribution dist;
  normal_distribution::param_type param( mean_, std_ );
  dist.param( param );
  assert( normal_dists_.size() == 0 );
  normal_dists_.resize( kernel::manager< VPManager >.get_num_threads(), dist );
}

double
NormalParameter::value( RngPtr rng, Node* node )
{
  const auto tid = node
    ? kernel::manager< VPManager >.vp_to_thread( kernel::manager< VPManager >.node_id_to_vp( node->get_node_id() ) )
    : kernel::manager< VPManager >.get_thread_id();
  return normal_dists_[ tid ]( rng );
}


LognormalParameter::LognormalParameter( const DictionaryDatum& d )
  : mean_( 0.0 )
  , std_( 1.0 )
{
  updateValue< double >( d, names::mean, mean_ );
  updateValue< double >( d, names::std, std_ );
  if ( std_ <= 0 )
  {
    throw BadProperty( "nest::LognormalParameter: std > 0 required." );
  }
  lognormal_distribution dist;
  const lognormal_distribution::param_type param( mean_, std_ );
  dist.param( param );
  assert( lognormal_dists_.size() == 0 );
  lognormal_dists_.resize( kernel::manager< VPManager >.get_num_threads(), dist );
}

double
LognormalParameter::value( RngPtr rng, Node* node )
{
  const auto tid = node
    ? kernel::manager< VPManager >.vp_to_thread( kernel::manager< VPManager >.node_id_to_vp( node->get_node_id() ) )
    : kernel::manager< VPManager >.get_thread_id();
  return lognormal_dists_[ tid ]( rng );
}


double
NodePosParameter::get_node_pos_( Node* node ) const
{
  if ( not node )
  {
    throw KernelException( "NodePosParameter: not node" );
  }
  NodeCollectionPTR nc = kernel::manager< NodeManager >.node_id_to_node_collection( node );
  if ( not nc.get() )
  {
    throw KernelException( "NodePosParameter: not nc" );
  }
  NodeCollectionMetadataPTR meta = nc->get_metadata();
  if ( not meta.get() )
  {
    throw KernelException( "NodePosParameter: not meta" );
  }
  auto const* const layer_meta = dynamic_cast< LayerMetadata const* >( meta.get() );
  if ( not layer_meta )
  {
    throw KernelException( "NodePosParameter: not layer_meta" );
  }
  AbstractLayerPTR layer = layer_meta->get_layer();
  if ( not layer.get() )
  {
    throw KernelException( "NodePosParameter: not valid layer" );
  }
  size_t lid = node->get_node_id() - meta->get_first_node_id();
  std::vector< double > pos = layer->get_position_vector( lid );
  if ( ( unsigned int ) dimension_ >= pos.size() )
  {
    throw KernelException(
      "Node position dimension must be within the defined number of "
      "dimensions for the node." );
  }
  return pos[ dimension_ ];
}

double
SpatialDistanceParameter::value( RngPtr,
  const std::vector< double >& source_pos,
  const std::vector< double >& target_pos,
  const AbstractLayer& layer,
  Node* )
{
  switch ( dimension_ )
  {
  case 0:
  {
    return layer.compute_distance( source_pos, target_pos );
  }
  case 1:
  case 2:
  case 3:
    if ( ( unsigned int ) dimension_ > layer.get_num_dimensions() )
    {
      throw KernelException(
        "Spatial distance dimension must be within the defined number of "
        "dimensions for the nodes." );
    }
    return std::abs( layer.compute_displacement( source_pos, target_pos, dimension_ - 1 ) );
  default:
    throw KernelException(
      String::compose( "SpatialDistanceParameter dimension must be either 0 for unspecified,"
                       " or 1-3 for x-z. Got ",
        dimension_ ) );
    break;
  }
}

RedrawParameter::RedrawParameter( const std::shared_ptr< Parameter > p, const double min, const double max )
  : Parameter( p->is_spatial() )
  , p_( p )
  , min_( min )
  , max_( max )
  , max_redraws_( 1000 )
{
  if ( min > max )
  {
    throw BadParameterValue( "min <= max required." );
  }
  if ( max < min )
  {
    throw BadParameterValue( "max >= min required." );
  }
}

double
RedrawParameter::value( RngPtr rng, Node* node )
{
  double value;
  size_t num_redraws = 0;
  do
  {
    if ( num_redraws++ == max_redraws_ )
    {
      throw KernelException( String::compose( "Number of redraws exceeded limit of %1", max_redraws_ ) );
    }
    value = p_->value( rng, node );
  } while ( value < min_ or value > max_ );
  return value;
}

double
RedrawParameter::value( RngPtr rng,
  const std::vector< double >& source_pos,
  const std::vector< double >& target_pos,
  const AbstractLayer& layer,
  Node* node )
{
  double value;
  size_t num_redraws = 0;
  do
  {
    if ( num_redraws++ == max_redraws_ )
    {
      throw KernelException( String::compose( "Number of redraws exceeded limit of %1", max_redraws_ ) );
    }
    value = p_->value( rng, source_pos, target_pos, layer, node );
  } while ( value < min_ or value > max_ );

  return value;
}


ExpDistParameter::ExpDistParameter( const DictionaryDatum& d )
  : Parameter( true )
  , p_( getValue< ParameterDatum >( d, "x" ) )
  , inv_beta_( 1.0 / getValue< double >( d, "beta" ) )
{
  const auto beta = getValue< double >( d, "beta" );
  if ( beta <= 0 )
  {
    throw BadProperty( "beta > 0 required for exponential distribution parameter, got beta=" + std::to_string( beta ) );
  }
}

double
ExpDistParameter::value( RngPtr rng,
  const std::vector< double >& source_pos,
  const std::vector< double >& target_pos,
  const AbstractLayer& layer,
  Node* node )
{
  return std::exp( -p_->value( rng, source_pos, target_pos, layer, node ) * inv_beta_ );
}

GaussianParameter::GaussianParameter( const DictionaryDatum& d )
  : Parameter( true )
  , p_( getValue< ParameterDatum >( d, "x" ) )
  , mean_( getValue< double >( d, "mean" ) )
  , inv_two_std2_( 1.0 / ( 2 * getValue< double >( d, "std" ) * getValue< double >( d, "std" ) ) )
{
  const auto std = getValue< double >( d, "std" );
  if ( std <= 0 )
  {
    throw BadProperty( "std > 0 required for gaussian distribution parameter, got std=" + std::to_string( std ) );
  }
}

double
GaussianParameter::value( RngPtr rng,
  const std::vector< double >& source_pos,
  const std::vector< double >& target_pos,
  const AbstractLayer& layer,
  Node* node )
{
  const auto dx = p_->value( rng, source_pos, target_pos, layer, node ) - mean_;
  return std::exp( -dx * dx * inv_two_std2_ );
}


Gaussian2DParameter::Gaussian2DParameter( const DictionaryDatum& d )
  : Parameter( true )
  , px_( getValue< ParameterDatum >( d, "x" ) )
  , py_( getValue< ParameterDatum >( d, "y" ) )
  , mean_x_( getValue< double >( d, "mean_x" ) )
  , mean_y_( getValue< double >( d, "mean_y" ) )
  , x_term_const_( 1.
      / ( 2. * ( 1. - getValue< double >( d, "rho" ) * getValue< double >( d, "rho" ) )
        * getValue< double >( d, "std_x" ) * getValue< double >( d, "std_x" ) ) )
  , y_term_const_( 1.
      / ( 2. * ( 1. - getValue< double >( d, "rho" ) * getValue< double >( d, "rho" ) )
        * getValue< double >( d, "std_y" ) * getValue< double >( d, "std_y" ) ) )
  , xy_term_const_( getValue< double >( d, "rho" )
      / ( ( 1. - getValue< double >( d, "rho" ) * getValue< double >( d, "rho" ) ) * getValue< double >( d, "std_x" )
        * getValue< double >( d, "std_y" ) ) )
{
  const auto rho = getValue< double >( d, "rho" );
  const auto std_x = getValue< double >( d, "std_x" );
  const auto std_y = getValue< double >( d, "std_y" );
  if ( rho >= 1 or rho <= -1 )
  {
    throw BadProperty(
      "-1 < rho < 1 required for gaussian2d distribution parameter, got rho=" + std::to_string( rho ) );
  }
  if ( std_x <= 0 )
  {
    throw BadProperty(
      "std_x > 0 required for gaussian2d distribution parameter, got std_x=" + std::to_string( std_x ) );
  }
  if ( std_y <= 0 )
  {
    throw BadProperty(
      "std_y > 0 required for gaussian2d distribution parameter, got std_y=" + std::to_string( std_y ) );
  }
}

double
Gaussian2DParameter::value( RngPtr rng,
  const std::vector< double >& source_pos,
  const std::vector< double >& target_pos,
  const AbstractLayer& layer,
  Node* node )
{
  const auto dx = px_->value( rng, source_pos, target_pos, layer, node ) - mean_x_;
  const auto dy = py_->value( rng, source_pos, target_pos, layer, node ) - mean_y_;
  return std::exp( -dx * dx * x_term_const_ - dy * dy * y_term_const_ + dx * dy * xy_term_const_ );
}


GaborParameter::GaborParameter( const DictionaryDatum& d )
  : Parameter( true )
  , px_( getValue< ParameterDatum >( d, "x" ) )
  , py_( getValue< ParameterDatum >( d, "y" ) )
  , cos_( std::cos( getValue< double >( d, "theta" ) * numerics::pi / 180. ) )
  , sin_( std::sin( getValue< double >( d, "theta" ) * numerics::pi / 180. ) )
  , gamma_( getValue< double >( d, "gamma" ) )
  , inv_two_std2_( 1.0 / ( 2 * getValue< double >( d, "std" ) * getValue< double >( d, "std" ) ) )
  , lambda_( getValue< double >( d, "lam" ) )
  , psi_( getValue< double >( d, "psi" ) )
{
  const auto gamma = getValue< double >( d, "gamma" );
  const auto std = getValue< double >( d, "std" );
  if ( std <= 0 )
  {
    throw BadProperty( "std > 0 required for gabor function parameter, got std=" + std::to_string( std ) );
  }
  if ( gamma <= 0 )
  {
    throw BadProperty( "gamma > 0 required for gabor function parameter, got gamma=" + std::to_string( gamma ) );
  }
}

double
GaborParameter::value( RngPtr rng,
  const std::vector< double >& source_pos,
  const std::vector< double >& target_pos,
  const AbstractLayer& layer,
  Node* node )
{
  const auto dx = px_->value( rng, source_pos, target_pos, layer, node );
  const auto dy = py_->value( rng, source_pos, target_pos, layer, node );
  const auto dx_prime = dx * cos_ + dy * sin_;
  const auto dy_prime = -dx * sin_ + dy * cos_;
  const auto gabor_exp =
    std::exp( -gamma_ * gamma_ * dx_prime * dx_prime * inv_two_std2_ - dy_prime * dy_prime * inv_two_std2_ );
  const auto gabor_cos_plus =
    std::max( std::cos( 2 * numerics::pi * dy_prime / lambda_ + psi_ * numerics::pi / 180. ), 0. );
  const auto gabor_res = gabor_exp * gabor_cos_plus;

  return gabor_res;
}


GammaParameter::GammaParameter( const DictionaryDatum& d )
  : Parameter( true )
  , p_( getValue< ParameterDatum >( d, "x" ) )
  , kappa_( getValue< double >( d, "kappa" ) )
  , inv_theta_( 1.0 / getValue< double >( d, "theta" ) )
  , delta_( std::pow( inv_theta_, kappa_ ) / std::tgamma( kappa_ ) )
{
  if ( kappa_ <= 0 )
  {
    throw BadProperty( "kappa > 0 required for gamma distribution parameter, got kappa=" + std::to_string( kappa_ ) );
  }
  const auto theta = getValue< double >( d, "theta" );
  if ( theta <= 0 )
  {
    throw BadProperty( "theta > 0 required for gamma distribution parameter, got theta=" + std::to_string( theta ) );
  }
}

double
GammaParameter::value( RngPtr rng,
  const std::vector< double >& source_pos,
  const std::vector< double >& target_pos,
  const AbstractLayer& layer,
  Node* node )
{
  const auto x = p_->value( rng, source_pos, target_pos, layer, node );
  return std::pow( x, kappa_ - 1. ) * std::exp( -1. * inv_theta_ * x ) * delta_;
}


std::shared_ptr< Parameter >
multiply_parameter( const std::shared_ptr< Parameter > first, const std::shared_ptr< Parameter > second )
{
  return std::shared_ptr< Parameter >( new ProductParameter( first, second ) );
}

std::shared_ptr< Parameter >
divide_parameter( const std::shared_ptr< Parameter > first, const std::shared_ptr< Parameter > second )
{
  return std::shared_ptr< Parameter >( new QuotientParameter( first, second ) );
}

std::shared_ptr< Parameter >
add_parameter( const std::shared_ptr< Parameter > first, const std::shared_ptr< Parameter > second )
{
  return std::shared_ptr< Parameter >( new SumParameter( first, second ) );
}

std::shared_ptr< Parameter >
subtract_parameter( const std::shared_ptr< Parameter > first, const std::shared_ptr< Parameter > second )
{
  return std::shared_ptr< Parameter >( new DifferenceParameter( first, second ) );
}

std::shared_ptr< Parameter >
compare_parameter( const std::shared_ptr< Parameter > first,
  const std::shared_ptr< Parameter > second,
  const DictionaryDatum& d )
{
  return std::shared_ptr< Parameter >( new ComparingParameter( first, second, d ) );
}

std::shared_ptr< Parameter >
conditional_parameter( const std::shared_ptr< Parameter > condition,
  const std::shared_ptr< Parameter > if_true,
  const std::shared_ptr< Parameter > if_false )
{
  return std::shared_ptr< Parameter >( new ConditionalParameter( condition, if_true, if_false ) );
}

std::shared_ptr< Parameter >
min_parameter( const std::shared_ptr< Parameter > parameter, const double other )
{
  return std::shared_ptr< Parameter >( new MinParameter( parameter, other ) );
}

std::shared_ptr< Parameter >
max_parameter( const std::shared_ptr< Parameter > parameter, const double other )
{
  return std::shared_ptr< Parameter >( new MaxParameter( parameter, other ) );
}

std::shared_ptr< Parameter >
redraw_parameter( const std::shared_ptr< Parameter > parameter, const double min, const double max )
{
  return std::shared_ptr< Parameter >( new RedrawParameter( parameter, min, max ) );
}

std::shared_ptr< Parameter >
exp_parameter( const std::shared_ptr< Parameter > parameter )
{
  return std::shared_ptr< Parameter >( new ExpParameter( parameter ) );
}

std::shared_ptr< Parameter >
sin_parameter( const std::shared_ptr< Parameter > parameter )
{
  return std::shared_ptr< Parameter >( new SinParameter( parameter ) );
}

std::shared_ptr< Parameter >
cos_parameter( const std::shared_ptr< Parameter > parameter )
{
  return std::shared_ptr< Parameter >( new CosParameter( parameter ) );
}

std::shared_ptr< Parameter >
pow_parameter( const std::shared_ptr< Parameter > parameter, const double exponent )
{
  return std::shared_ptr< Parameter >( new PowParameter( parameter, exponent ) );
}

std::shared_ptr< Parameter >
dimension_parameter( const std::shared_ptr< Parameter > x_parameter, const std::shared_ptr< Parameter > y_parameter )
{
  return std::shared_ptr< Parameter >( new DimensionParameter( x_parameter, y_parameter ) );
}

std::shared_ptr< Parameter >
dimension_parameter( const std::shared_ptr< Parameter > x_parameter,
  const std::shared_ptr< Parameter > y_parameter,
  const std::shared_ptr< Parameter > z_parameter )
{
  return std::shared_ptr< Parameter >( new DimensionParameter( x_parameter, y_parameter, z_parameter ) );
}

double
ConstantParameter::value( RngPtr, Node* )
{
  return value_;
}

double
UniformParameter::value( RngPtr rng, Node* )
{
  return lower_ + rng->drand() * range_;
}

double
UniformIntParameter::value( RngPtr rng, Node* )
{
  return rng->ulrand( max_ );
}

double
ExponentialParameter::value( RngPtr rng, Node* )
{
  return beta_ * ( -std::log( 1 - rng->drand() ) );
}

double
NodePosParameter::value( RngPtr, Node* node )
{
  if ( synaptic_endpoint_ != 0 )
  {
    throw BadParameterValue( "Source or target position parameter can only be used when connecting." );
  }
  if ( not node )
  {
    throw KernelException( "Node position parameter can only be used when connecting spatially distributed nodes." );
  }
  return get_node_pos_( node );
}

double
NodePosParameter::value( RngPtr,
  const std::vector< double >& source_pos,
  const std::vector< double >& target_pos,
  const AbstractLayer&,
  Node* )
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

double
SpatialDistanceParameter::value( RngPtr, Node* )
{
  throw BadParameterValue( "Spatial distance parameter can only be used when connecting." );
}

double
ProductParameter::value( RngPtr rng, Node* node )
{
  return parameter1_->value( rng, node ) * parameter2_->value( rng, node );
}

double
ProductParameter::value( RngPtr rng,
  const std::vector< double >& source_pos,
  const std::vector< double >& target_pos,
  const AbstractLayer& layer,
  Node* node )
{
  return parameter1_->value( rng, source_pos, target_pos, layer, node )
    * parameter2_->value( rng, source_pos, target_pos, layer, node );
}

double
QuotientParameter::value( RngPtr rng, Node* node )
{
  return parameter1_->value( rng, node ) / parameter2_->value( rng, node );
}

double
QuotientParameter::value( RngPtr rng,
  const std::vector< double >& source_pos,
  const std::vector< double >& target_pos,
  const AbstractLayer& layer,
  Node* node )
{
  return parameter1_->value( rng, source_pos, target_pos, layer, node )
    / parameter2_->value( rng, source_pos, target_pos, layer, node );
}

double
SumParameter::value( RngPtr rng, Node* node )
{
  return parameter1_->value( rng, node ) + parameter2_->value( rng, node );
}

double
SumParameter::value( RngPtr rng,
  const std::vector< double >& source_pos,
  const std::vector< double >& target_pos,
  const AbstractLayer& layer,
  Node* node )
{
  return parameter1_->value( rng, source_pos, target_pos, layer, node )
    + parameter2_->value( rng, source_pos, target_pos, layer, node );
}

double
DifferenceParameter::value( RngPtr rng, Node* node )
{
  return parameter1_->value( rng, node ) - parameter2_->value( rng, node );
}

double
DifferenceParameter::value( RngPtr rng,
  const std::vector< double >& source_pos,
  const std::vector< double >& target_pos,
  const AbstractLayer& layer,
  Node* node )
{
  return parameter1_->value( rng, source_pos, target_pos, layer, node )
    - parameter2_->value( rng, source_pos, target_pos, layer, node );
}

double
ComparingParameter::value( RngPtr rng, Node* node )
{
  return compare_( parameter1_->value( rng, node ), parameter2_->value( rng, node ) );
}

double
ComparingParameter::value( RngPtr rng,
  const std::vector< double >& source_pos,
  const std::vector< double >& target_pos,
  const AbstractLayer& layer,
  Node* node )
{
  return compare_( parameter1_->value( rng, source_pos, target_pos, layer, node ),
    parameter2_->value( rng, source_pos, target_pos, layer, node ) );
}

bool
ComparingParameter::compare_( double value_a, double value_b ) const
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

double
ConditionalParameter::value( RngPtr rng, Node* node )
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
ConditionalParameter::value( RngPtr rng,
  const std::vector< double >& source_pos,
  const std::vector< double >& target_pos,
  const AbstractLayer& layer,
  Node* node )
{
  if ( condition_->value( rng, source_pos, target_pos, layer, node ) )
  {
    return if_true_->value( rng, source_pos, target_pos, layer, node );
  }
  else
  {
    return if_false_->value( rng, source_pos, target_pos, layer, node );
  }
}

double
MinParameter::value( RngPtr rng, Node* node )
{
  return std::min( p_->value( rng, node ), other_value_ );
}

double
MinParameter::value( RngPtr rng,
  const std::vector< double >& source_pos,
  const std::vector< double >& target_pos,
  const AbstractLayer& layer,
  Node* node )
{
  return std::min( p_->value( rng, source_pos, target_pos, layer, node ), other_value_ );
}

double
MaxParameter::value( RngPtr rng, Node* node )
{
  return std::max( p_->value( rng, node ), other_value_ );
}

double
MaxParameter::value( RngPtr rng,
  const std::vector< double >& source_pos,
  const std::vector< double >& target_pos,
  const AbstractLayer& layer,
  Node* node )
{
  return std::max( p_->value( rng, source_pos, target_pos, layer, node ), other_value_ );
}

double
ExpParameter::value( RngPtr rng, Node* node )
{
  return std::exp( p_->value( rng, node ) );
}

double
ExpParameter::value( RngPtr rng,
  const std::vector< double >& source_pos,
  const std::vector< double >& target_pos,
  const AbstractLayer& layer,
  Node* node )
{
  return std::exp( p_->value( rng, source_pos, target_pos, layer, node ) );
}

double
SinParameter::value( RngPtr rng, Node* node )
{
  return std::sin( p_->value( rng, node ) );
}

double
SinParameter::value( RngPtr rng,
  const std::vector< double >& source_pos,
  const std::vector< double >& target_pos,
  const AbstractLayer& layer,
  Node* node )
{
  return std::sin( p_->value( rng, source_pos, target_pos, layer, node ) );
}

double
CosParameter::value( RngPtr rng, Node* node )
{
  return std::cos( p_->value( rng, node ) );
}

double
CosParameter::value( RngPtr rng,
  const std::vector< double >& source_pos,
  const std::vector< double >& target_pos,
  const AbstractLayer& layer,
  Node* node )
{
  return std::cos( p_->value( rng, source_pos, target_pos, layer, node ) );
}

double
PowParameter::value( RngPtr rng, Node* node )
{
  return std::pow( p_->value( rng, node ), exponent_ );
}

double
PowParameter::value( RngPtr rng,
  const std::vector< double >& source_pos,
  const std::vector< double >& target_pos,
  const AbstractLayer& layer,
  Node* node )
{
  return std::pow( p_->value( rng, source_pos, target_pos, layer, node ), exponent_ );
}

double
DimensionParameter::value( RngPtr, Node* )
{
  throw KernelException( "Cannot get value of DimensionParameter." );
}

/**
 * Generates a position with values for each dimension generated from their respective parameters.
 *
 * @returns The position, given as an array.
 */
std::vector< double >
DimensionParameter::get_values( RngPtr rng )
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
DimensionParameter::get_num_dimensions() const
{
  return num_dimensions_;
}

double
ExpDistParameter::value( RngPtr, Node* )
{
  throw BadParameterValue( "Exponential distribution parameter can only be used when connecting." );
}

double
GaussianParameter::value( RngPtr, Node* )
{
  throw BadParameterValue( "Gaussian distribution parameter can only be used when connecting." );
}

double
Gaussian2DParameter::value( RngPtr, Node* )
{
  throw BadParameterValue( "Gaussian 2D parameter can only be used when connecting." );
}

double
GaborParameter::value( RngPtr, Node* )
{
  throw BadParameterValue( "Gabor parameter can only be used when connecting." );
}

double
GammaParameter::value( RngPtr, Node* )
{
  throw BadParameterValue( "Gamma distribution parameter can only be used when connecting." );
}

double
Parameter::value( RngPtr rng,
  const std::vector< double >&,
  const std::vector< double >&,
  const AbstractLayer&,
  Node* node )
{
  return value( rng, node );
}

bool
Parameter::is_spatial() const
{
  return is_spatial_;
}

bool
Parameter::returns_int_only() const
{
  return returns_int_only_;
}

bool
Parameter::value_is_integer_( const double value ) const
{
  // Here fmod calculates the remainder of the division operation x/y. By using y=1.0, the remainder is the
  // fractional part of the value. If the fractional part is zero, the value is an integer.
  return std::fmod( value, static_cast< double >( 1.0 ) ) == 0.0;
}

} // namespace nest
