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

#include "node_collection.h"
#include "node.h"
#include "spatial.h"

// includes from sli
#include "sharedptrdatum.h"

#include "parameter.h"

template class sharedPtrDatum< nest::Parameter, &nest::NestModule::ParameterType >;

namespace nest
{
Node*
Parameter::node_id_to_node_ptr_( const index node_id, const thread t ) const
{
  return kernel().node_manager.get_node_or_proxy( node_id, t );
}

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
  const index source_lid = nc->operator[]( 0 ) - source_metadata->get_first_node_id();
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
    auto value = this->value( rng, source_pos, target_pos, *source_layer.get() );
    result.push_back( value );
  }
  return result;
}

NormalParameter::NormalParameter( const DictionaryDatum& d )
  : Parameter( d )
  , mean_( 0.0 )
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
  normal_dists_.resize( kernel().vp_manager.get_num_threads(), dist );
}

double
NormalParameter::value( RngPtr rng, Node* )
{
  return normal_dists_[ kernel().vp_manager.get_thread_id() ]( rng );
}


LognormalParameter::LognormalParameter( const DictionaryDatum& d )
  : Parameter( d )
  , mean_( 0.0 )
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
  lognormal_dists_.resize( kernel().vp_manager.get_num_threads(), dist );
}

double
LognormalParameter::value( RngPtr rng, Node* )
{
  return lognormal_dists_[ kernel().vp_manager.get_thread_id() ]( rng );
}


double
NodePosParameter::get_node_pos_( RngPtr, Node* node ) const
{
  if ( not node )
  {
    throw KernelException( "NodePosParameter: not node" );
  }
  NodeCollectionPTR nc = node->get_nc();
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
  index lid = node->get_node_id() - meta->get_first_node_id();
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
  const AbstractLayer& layer )
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
    throw KernelException( String::compose(
      "SpatialDistanceParameter dimension must be either 0 for unspecified,"
      " or 1-3 for x-z. Got ",
      dimension_ ) );
    break;
  }
}

RedrawParameter::RedrawParameter( const Parameter& p, const double min, const double max )
  : Parameter( p )
  , p_( p.clone() )
  , min_( min )
  , max_( max )
  , max_redraws_( 1000 )
{
  parameter_is_spatial_ = p_->is_spatial();
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
RedrawParameter::value( RngPtr rng, index snode_id, Node* target, thread target_thread )
{
  double value;
  size_t num_redraws = 0;
  do
  {
    if ( num_redraws++ == max_redraws_ )
    {
      throw KernelException( String::compose( "Number of redraws exceeded limit of %1", max_redraws_ ) );
    }
    value = p_->value( rng, snode_id, target, target_thread );
  } while ( value < min_ or value > max_ );
  return value;
}

double
RedrawParameter::value( RngPtr rng,
  const std::vector< double >& source_pos,
  const std::vector< double >& target_pos,
  const AbstractLayer& layer )
{
  double value;
  size_t num_redraws = 0;
  do
  {
    if ( num_redraws++ == max_redraws_ )
    {
      throw KernelException( String::compose( "Number of redraws exceeded limit of %1", max_redraws_ ) );
    }
    value = p_->value( rng, source_pos, target_pos, layer );
  } while ( value < min_ or value > max_ );

  return value;
}


ExpDistParameter::ExpDistParameter( const DictionaryDatum& d )
  : p_( getValue< ParameterDatum >( d, "x" )->clone() )
  , inv_beta_( 1.0 / getValue< double >( d, "beta" ) )
{
  parameter_is_spatial_ = true;

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
  const AbstractLayer& layer )
{
  return std::exp( -p_->value( rng, source_pos, target_pos, layer ) * inv_beta_ );
}

GaussianParameter::GaussianParameter( const DictionaryDatum& d )
  : p_( getValue< ParameterDatum >( d, "x" )->clone() )
  , mean_( getValue< double >( d, "mean" ) )
  , inv_two_std2_( 1.0 / ( 2 * getValue< double >( d, "std" ) * getValue< double >( d, "std" ) ) )
{
  parameter_is_spatial_ = true;

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
  const AbstractLayer& layer )
{
  const auto dx = p_->value( rng, source_pos, target_pos, layer ) - mean_;
  return std::exp( -dx * dx * inv_two_std2_ );
}


Gaussian2DParameter::Gaussian2DParameter( const DictionaryDatum& d )
  : px_( getValue< ParameterDatum >( d, "x" )->clone() )
  , py_( getValue< ParameterDatum >( d, "y" )->clone() )
  , mean_x_( getValue< double >( d, "mean_x" ) )
  , mean_y_( getValue< double >( d, "mean_y" ) )
  , x_term_const_( 1. / ( 2. * ( 1. - getValue< double >( d, "rho" ) * getValue< double >( d, "rho" ) )
                          * getValue< double >( d, "std_x" ) * getValue< double >( d, "std_x" ) ) )
  , y_term_const_( 1. / ( 2. * ( 1. - getValue< double >( d, "rho" ) * getValue< double >( d, "rho" ) )
                          * getValue< double >( d, "std_y" ) * getValue< double >( d, "std_y" ) ) )
  , xy_term_const_(
      getValue< double >( d, "rho" ) / ( ( 1. - getValue< double >( d, "rho" ) * getValue< double >( d, "rho" ) )
                                         * getValue< double >( d, "std_x" ) * getValue< double >( d, "std_y" ) ) )
{
  parameter_is_spatial_ = true;

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
  const AbstractLayer& layer )
{
  const auto dx = px_->value( rng, source_pos, target_pos, layer ) - mean_x_;
  const auto dy = py_->value( rng, source_pos, target_pos, layer ) - mean_y_;
  return std::exp( -dx * dx * x_term_const_ - dy * dy * y_term_const_ + dx * dy * xy_term_const_ );
}


GammaParameter::GammaParameter( const DictionaryDatum& d )
  : p_( getValue< ParameterDatum >( d, "x" )->clone() )
  , kappa_( getValue< double >( d, "kappa" ) )
  , inv_theta_( 1.0 / getValue< double >( d, "theta" ) )
  , delta_( std::pow( inv_theta_, kappa_ ) / std::tgamma( kappa_ ) )
{
  parameter_is_spatial_ = true;

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
  const AbstractLayer& layer )
{
  const auto x = p_->value( rng, source_pos, target_pos, layer );
  return std::pow( x, kappa_ - 1. ) * std::exp( -1. * inv_theta_ * x ) * delta_;
}

} /* namespace nest */
