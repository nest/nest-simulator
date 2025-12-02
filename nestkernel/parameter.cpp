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

#include "node.h"
#include "node_collection.h"
#include "spatial.h"
#include "vp_manager_impl.h"

#include "parameter.h"


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
  normal_dists_.resize( kernel().vp_manager.get_num_threads(), dist );
}

double
NormalParameter::value( RngPtr rng, Node* node )
{
  const auto tid = node ? kernel().vp_manager.vp_to_thread( kernel().vp_manager.node_id_to_vp( node->get_node_id() ) )
                        : kernel().vp_manager.get_thread_id();
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
  lognormal_dists_.resize( kernel().vp_manager.get_num_threads(), dist );
}

double
LognormalParameter::value( RngPtr rng, Node* node )
{
  const auto tid = node ? kernel().vp_manager.vp_to_thread( kernel().vp_manager.node_id_to_vp( node->get_node_id() ) )
                        : kernel().vp_manager.get_thread_id();
  return lognormal_dists_[ tid ]( rng );
}


double
NodePosParameter::get_node_pos_( Node* node ) const
{
  if ( not node )
  {
    throw KernelException( "NodePosParameter: not node" );
  }
  NodeCollectionPTR nc = kernel().node_manager.node_id_to_node_collection( node );
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

} // namespace nest
