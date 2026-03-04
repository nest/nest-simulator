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

#include "dictionary.h"
#include "node.h"
#include "node_collection.h"
#include "parameter.h"
#include "spatial.h"


namespace nest
{

std::vector< double >
Parameter::apply( const NodeCollectionPTR& nc, const std::vector< std::vector< double > >& positions )
{
  std::vector< double > result;
  result.reserve( positions.size() );
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
  for ( auto& target_pos : positions )
  {
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

NormalParameter::NormalParameter( const Dictionary& d )
  : mean_( 0.0 )
  , std_( 1.0 )
{
  d.update_value( names::mean, mean_ );
  d.update_value( names::std, std_ );
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


LognormalParameter::LognormalParameter( const Dictionary& d )
  : mean_( 0.0 )
  , std_( 1.0 )
{
  d.update_value( names::mean, mean_ );
  d.update_value( names::std, std_ );
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

SpatialDistanceParameter::SpatialDistanceParameter( const Dictionary& d )
  : Parameter( true )
  , dimension_( 0 )
{
  d.update_integer_value( names::dimension, dimension_ );
  if ( dimension_ < 0 )
  {
    throw BadParameterValue( "Spatial distance parameter dimension cannot be negative." );
  }
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

ProductParameter::ProductParameter( const ParameterPTR m1, const ParameterPTR m2 )
  : Parameter( m1->is_spatial() or m2->is_spatial(), m1->returns_int_only() and m2->returns_int_only() )
  , parameter1_( m1 )
  , parameter2_( m2 )
{
}

ProductParameter::ProductParameter( const ProductParameter& p )
  : Parameter( p )
  , parameter1_( p.parameter1_ )
  , parameter2_( p.parameter2_ )
{
}

RedrawParameter::RedrawParameter( const ParameterPTR p, const double min, const double max )
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

RedrawParameter::RedrawParameter( const RedrawParameter& p )
  : Parameter( p )
  , p_( p.p_ )
  , min_( p.min_ )
  , max_( p.max_ )
  , max_redraws_( p.max_redraws_ )
{
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


ExpDistParameter::ExpDistParameter( const Dictionary& d )
  : Parameter( true )
  , p_( d.get< ParameterPTR >( "x" ) )
  , inv_beta_( 1.0 / d.get< double >( "beta" ) )
{
  const auto beta = d.get< double >( "beta" );
  if ( beta <= 0 )
  {
    throw BadProperty( "beta > 0 required for exponential distribution parameter, got beta=" + std::to_string( beta ) );
  }
}

ExpDistParameter::ExpDistParameter( const ExpDistParameter& p )
  : Parameter( p )
  , p_( p.p_ )
  , inv_beta_( p.inv_beta_ )
{
  assert( is_spatial_ == p.is_spatial() );
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

GaussianParameter::GaussianParameter( const Dictionary& d )
  : Parameter( true )
  , p_( d.get< ParameterPTR >( "x" ) )
  , mean_( d.get< double >( "mean" ) )
  , inv_two_std2_( 1.0 / ( 2 * d.get< double >( "std" ) * d.get< double >( "std" ) ) )
{
  const auto std = d.get< double >( "std" );
  if ( std <= 0 )
  {
    throw BadProperty( "std > 0 required for gaussian distribution parameter, got std=" + std::to_string( std ) );
  }
}

GaussianParameter::GaussianParameter( const GaussianParameter& p )
  : Parameter( p )
  , p_( p.p_ )
  , mean_( p.mean_ )
  , inv_two_std2_( p.inv_two_std2_ )
{
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


Gaussian2DParameter::Gaussian2DParameter( const Dictionary& d )
  : Parameter( true )
  , px_( d.get< ParameterPTR >( "x" ) )
  , py_( d.get< ParameterPTR >( "y" ) )
  , mean_x_( d.get< double >( "mean_x" ) )
  , mean_y_( d.get< double >( "mean_y" ) )
  , x_term_const_( 1.
      / ( 2. * ( 1. - d.get< double >( "rho" ) * d.get< double >( "rho" ) ) * d.get< double >( "std_x" )
        * d.get< double >( "std_x" ) ) )
  , y_term_const_( 1.
      / ( 2. * ( 1. - d.get< double >( "rho" ) * d.get< double >( "rho" ) ) * d.get< double >( "std_y" )
        * d.get< double >( "std_y" ) ) )
  , xy_term_const_( d.get< double >( "rho" )
      / ( ( 1. - d.get< double >( "rho" ) * d.get< double >( "rho" ) ) * d.get< double >( "std_x" )
        * d.get< double >( "std_y" ) ) )
{
  const auto rho = d.get< double >( "rho" );
  const auto std_x = d.get< double >( "std_x" );
  const auto std_y = d.get< double >( "std_y" );
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

Gaussian2DParameter::Gaussian2DParameter( const Gaussian2DParameter& p )
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


GaborParameter::GaborParameter( const Dictionary& d )
  : Parameter( true )
  , px_( d.get< ParameterPTR >( "x" ) )
  , py_( d.get< ParameterPTR >( "y" ) )
  , cos_( std::cos( d.get< double >( "theta" ) * numerics::pi / 180. ) )
  , sin_( std::sin( d.get< double >( "theta" ) * numerics::pi / 180. ) )
  , gamma_( d.get< double >( "gamma" ) )
  , inv_two_std2_( 1.0 / ( 2 * d.get< double >( "std" ) * d.get< double >( "std" ) ) )
  , lambda_( d.get< double >( "lam" ) )
  , psi_( d.get< double >( "psi" ) )
{
  const auto gamma = d.get< double >( "gamma" );
  const auto std = d.get< double >( "std" );
  if ( std <= 0 )
  {
    throw BadProperty( String::compose( "std > 0 required for gabor function parameter, got std=%1", std ) );
  }
  if ( gamma <= 0 )
  {
    throw BadProperty( String::compose( "gamma > 0 required for gabor function parameter, got gamma=%1", gamma ) );
  }
}

GaborParameter::GaborParameter( const GaborParameter& p )
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


GammaParameter::GammaParameter( const Dictionary& d )
  : Parameter( true )
  , p_( d.get< ParameterPTR >( "x" ) )
  , kappa_( d.get< double >( "kappa" ) )
  , inv_theta_( 1.0 / d.get< double >( "theta" ) )
  , delta_( std::pow( inv_theta_, kappa_ ) / std::tgamma( kappa_ ) )
{
  if ( kappa_ <= 0 )
  {
    throw BadProperty( "kappa > 0 required for gamma distribution parameter, got kappa=" + std::to_string( kappa_ ) );
  }
  const auto theta = d.get< double >( "theta" );
  if ( theta <= 0 )
  {
    throw BadProperty( "theta > 0 required for gamma distribution parameter, got theta=" + std::to_string( theta ) );
  }
}

GammaParameter::GammaParameter( const GammaParameter& p )
  : Parameter( p )
  , p_( p.p_ )
  , kappa_( p.kappa_ )
  , inv_theta_( p.inv_theta_ )
  , delta_( p.delta_ )
{
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


ParameterPTR
multiply_parameter( const ParameterPTR first, const ParameterPTR second )
{
  return ParameterPTR( new ProductParameter( first, second ) );
}

ParameterPTR
divide_parameter( const ParameterPTR first, const ParameterPTR second )
{
  return ParameterPTR( new QuotientParameter( first, second ) );
}

ParameterPTR
add_parameter( const ParameterPTR first, const ParameterPTR second )
{
  return ParameterPTR( new SumParameter( first, second ) );
}

ParameterPTR
subtract_parameter( const ParameterPTR first, const ParameterPTR second )
{
  return ParameterPTR( new DifferenceParameter( first, second ) );
}

ParameterPTR
compare_parameter( const ParameterPTR first, const ParameterPTR second, const Dictionary& d )
{
  return ParameterPTR( new ComparingParameter( first, second, d ) );
}

ParameterPTR
conditional_parameter( const ParameterPTR condition, const ParameterPTR if_true, const ParameterPTR if_false )
{
  return ParameterPTR( new ConditionalParameter( condition, if_true, if_false ) );
}

ParameterPTR
min_parameter( const ParameterPTR parameter, const double other )
{
  return ParameterPTR( new MinParameter( parameter, other ) );
}

ParameterPTR
max_parameter( const ParameterPTR parameter, const double other )
{
  return ParameterPTR( new MaxParameter( parameter, other ) );
}

ParameterPTR
redraw_parameter( const ParameterPTR parameter, const double min, const double max )
{
  return ParameterPTR( new RedrawParameter( parameter, min, max ) );
}

ParameterPTR
exp_parameter( const ParameterPTR parameter )
{
  return ParameterPTR( new ExpParameter( parameter ) );
}

ParameterPTR
sin_parameter( const ParameterPTR parameter )
{
  return ParameterPTR( new SinParameter( parameter ) );
}

ParameterPTR
cos_parameter( const ParameterPTR parameter )
{
  return ParameterPTR( new CosParameter( parameter ) );
}

ParameterPTR
pow_parameter( const ParameterPTR parameter, const double exponent )
{
  return ParameterPTR( new PowParameter( parameter, exponent ) );
}

ParameterPTR
dimension_parameter( const ParameterPTR x_parameter, const ParameterPTR y_parameter )
{
  return ParameterPTR( new DimensionParameter( x_parameter, y_parameter ) );
}

ParameterPTR
dimension_parameter( const ParameterPTR x_parameter, const ParameterPTR y_parameter, const ParameterPTR z_parameter )
{
  return ParameterPTR( new DimensionParameter( x_parameter, y_parameter, z_parameter ) );
}

double
ConstantParameter::value( RngPtr, Node* )
{
  return value_;
}

UniformParameter::UniformParameter( const Dictionary& d )
  : lower_( 0.0 )
  , range_( 1.0 )
{
  d.update_value( names::min, lower_ );
  d.update_value( names::max, range_ );
  if ( lower_ >= range_ )
  {
    throw BadProperty(
      "nest::UniformParameter: "
      "min < max required." );
  }

  range_ -= lower_;
}

double
UniformParameter::value( RngPtr rng, Node* )
{
  return lower_ + rng->drand() * range_;
}
UniformIntParameter::UniformIntParameter( const Dictionary& d )
  : Parameter( false, true )
  , max_( 1.0 )
{
  d.update_integer_value( names::max, max_ );
  if ( max_ <= 0 )
  {
    throw BadProperty( "nest::UniformIntParameter: max > 0 required." );
  }
}

double
UniformIntParameter::value( RngPtr rng, Node* )
{
  return rng->ulrand( max_ );
}

ExponentialParameter::ExponentialParameter( const Dictionary& d )
  : beta_( 1.0 )
{
  d.update_value( names::beta, beta_ );
}

double
ExponentialParameter::value( RngPtr rng, Node* )
{
  return beta_ * ( -std::log( 1 - rng->drand() ) );
}

NodePosParameter::NodePosParameter( const Dictionary& d )
  : Parameter( true )
  , dimension_( 0 )
  , synaptic_endpoint_( 0 )
{
  bool dimension_specified = d.update_integer_value( names::dimension, dimension_ );
  if ( not dimension_specified )
  {
    throw BadParameterValue( "Dimension must be specified when creating a node position parameter." );
  }
  if ( dimension_ < 0 )
  {
    throw BadParameterValue( "Node position parameter dimension cannot be negative." );
  }
  d.update_integer_value( names::synaptic_endpoint, synaptic_endpoint_ );
  if ( synaptic_endpoint_ < 0 or 2 < synaptic_endpoint_ )
  {
    throw BadParameterValue( "Synaptic endpoint must either be unspecified (0), source (1) or target (2)." );
  }
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

QuotientParameter::QuotientParameter( ParameterPTR m1, ParameterPTR m2 )
  : Parameter( m1->is_spatial() or m2->is_spatial(), m1->returns_int_only() and m2->returns_int_only() )
  , parameter1_( m1 )
  , parameter2_( m2 )
{
}

QuotientParameter::QuotientParameter( const QuotientParameter& p )
  : Parameter( p )
  , parameter1_( p.parameter1_ )
  , parameter2_( p.parameter2_ )
{
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

SumParameter::SumParameter( ParameterPTR m1, ParameterPTR m2 )
  : Parameter( m1->is_spatial() or m2->is_spatial(), m1->returns_int_only() and m2->returns_int_only() )
  , parameter1_( m1 )
  , parameter2_( m2 )
{
}

SumParameter::SumParameter( const SumParameter& p )
  : Parameter( p )
  , parameter1_( p.parameter1_ )
  , parameter2_( p.parameter2_ )
{
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

DifferenceParameter::DifferenceParameter( ParameterPTR m1, ParameterPTR m2 )
  : Parameter( m1->is_spatial() or m2->is_spatial(), m1->returns_int_only() and m2->returns_int_only() )
  , parameter1_( m1 )
  , parameter2_( m2 )
{
}

DifferenceParameter::DifferenceParameter( const DifferenceParameter& p )
  : Parameter( p )
  , parameter1_( p.parameter1_ )
  , parameter2_( p.parameter2_ )
{
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

ComparingParameter::ComparingParameter( ParameterPTR m1, ParameterPTR m2, const Dictionary& d )
  : Parameter( m1->is_spatial() or m2->is_spatial(), true )
  , parameter1_( m1 )
  , parameter2_( m2 )
  , comparator_( -1 )
{
  if ( not d.update_integer_value( names::comparator, comparator_ ) )
  {
    throw BadParameter( "A comparator has to be specified." );
  }
  if ( comparator_ < 0 or 5 < comparator_ )
  {
    throw BadParameter( "Comparator specification has to be in the range 0-5." );
  }
}

ComparingParameter::ComparingParameter( const ComparingParameter& p )
  : Parameter( p )
  , parameter1_( p.parameter1_ )
  , parameter2_( p.parameter2_ )
  , comparator_( p.comparator_ )
{
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

ConditionalParameter::ConditionalParameter( ParameterPTR condition, ParameterPTR if_true, ParameterPTR if_false )
  : Parameter( condition->is_spatial() or if_true->is_spatial() or if_false->is_spatial(),
    if_true->returns_int_only() and if_false->returns_int_only() )
  , condition_( condition )
  , if_true_( if_true )
  , if_false_( if_false )
{
}

ConditionalParameter::ConditionalParameter( const ConditionalParameter& p )
  : Parameter( p )
  , condition_( p.condition_ )
  , if_true_( p.if_true_ )
  , if_false_( p.if_false_ )
{
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

MinParameter::MinParameter( ParameterPTR p, const double other_value )
  : Parameter( p->is_spatial(), p->returns_int_only() and value_is_integer_( other_value ) )
  , p_( p )
  , other_value_( other_value )
{
  assert( is_spatial_ == p->is_spatial() );
}

MinParameter::MinParameter( const MinParameter& p )
  : Parameter( p )
  , p_( p.p_ )
  , other_value_( p.other_value_ )
{
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

MaxParameter::MaxParameter( ParameterPTR p, const double other_value )
  : Parameter( p->is_spatial(), p->returns_int_only() and value_is_integer_( other_value ) )
  , p_( p )
  , other_value_( other_value )
{
}

MaxParameter::MaxParameter( const MaxParameter& p )
  : Parameter( p )
  , p_( p.p_ )
  , other_value_( p.other_value_ )
{
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

ExpParameter::ExpParameter( ParameterPTR p )
  : Parameter( p->is_spatial() )
  , p_( p )
{
}

ExpParameter::ExpParameter( const ExpParameter& p )
  : Parameter( p )
  , p_( p.p_ )
{
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

SinParameter::SinParameter( ParameterPTR p )
  : Parameter( p->is_spatial() )
  , p_( p )
{
}

SinParameter::SinParameter( const SinParameter& p )
  : Parameter( p )
  , p_( p.p_ )
{
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

CosParameter::CosParameter( ParameterPTR p )
  : Parameter( p->is_spatial() )
  , p_( p )
{
}

CosParameter::CosParameter( const CosParameter& p )
  : Parameter( p )
  , p_( p.p_ )
{
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

PowParameter::PowParameter( ParameterPTR p, const double exponent )
  : Parameter( p->is_spatial(), p->returns_int_only() )
  , p_( p )
  , exponent_( exponent )
{
}

PowParameter::PowParameter( const PowParameter& p )
  : Parameter( p )
  , p_( p.p_ )
  , exponent_( p.exponent_ )
{
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

DimensionParameter::DimensionParameter( ParameterPTR px, ParameterPTR py )
  : Parameter( true )
  , num_dimensions_( 2 )
  , px_( px )
  , py_( py )
  , pz_( nullptr )
{
}

DimensionParameter::DimensionParameter( ParameterPTR px, ParameterPTR py, ParameterPTR pz )
  : Parameter( true )
  , num_dimensions_( 3 )
  , px_( px )
  , py_( py )
  , pz_( pz )
{
}

DimensionParameter::DimensionParameter( const DimensionParameter& p )
  : Parameter( p )
  , num_dimensions_( p.num_dimensions_ )
  , px_( p.px_ )
  , py_( p.py_ )
  , pz_( p.pz_ )
{
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
ConstantParameter::ConstantParameter( double value )
  : value_( value )
{
  returns_int_only_ = value_is_integer_( value_ );
}

ConstantParameter::ConstantParameter( const Dictionary& d )
{
  value_ = d.get< double >( "value" );
  returns_int_only_ = value_is_integer_( value_ );
}

} // namespace nest
