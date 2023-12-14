/*
 *  weight_optimizer.cpp
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

#include "weight_optimizer.h"

// Includes from nestkernel
#include "exceptions.h"
#include "nest_names.h"

// Includes from sli
#include "dictutils.h"

namespace nest
{
WeightOptimizerCommonProperties::WeightOptimizerCommonProperties()
  : batch_size_( 1 )
  , eta_( 1e-4 )
  , Wmin_( 0.0 )
  , Wmax_( 100.0 )
{
}

WeightOptimizerCommonProperties::WeightOptimizerCommonProperties( const WeightOptimizerCommonProperties& cp )
  : batch_size_( cp.batch_size_ )
  , eta_( cp.eta_ )
  , Wmin_( cp.Wmin_ )
  , Wmax_( cp.Wmax_ )
{
}

void
WeightOptimizerCommonProperties::get_status( DictionaryDatum& d ) const
{
  def< std::string >( d, names::optimizer, get_name() );
  def< long >( d, names::batch_size, batch_size_ );
  def< double >( d, names::eta, eta_ );
  def< double >( d, names::Wmin, Wmin_ );
  def< double >( d, names::Wmax, Wmax_ );
}

void
WeightOptimizerCommonProperties::set_status( const DictionaryDatum& d )
{
  long new_batch_size = batch_size_;
  updateValue< long >( d, names::batch_size, new_batch_size );
  if ( new_batch_size <= 0 )
  {
    throw BadProperty( "Optimization batch_size > 0 required." );
  }
  batch_size_ = new_batch_size;

  double new_eta = eta_;
  updateValue< double >( d, names::eta, new_eta );
  if ( new_eta <= 0 )
  {
    throw BadProperty( "Learning rate eta > 0 required." );
  }
  eta_ = new_eta;

  double new_Wmin = Wmin_;
  double new_Wmax = Wmax_;
  updateValue< double >( d, names::Wmin, new_Wmin );
  updateValue< double >( d, names::Wmax, new_Wmax );
  if ( new_Wmin > new_Wmax )
  {
    throw BadProperty( "Minimal weight Wmin ≤ maximal weight Wmax required." );
  }
  Wmin_ = new_Wmin;
  Wmax_ = new_Wmax;
}

WeightOptimizer::WeightOptimizer()
  : sum_gradients_( 0.0 )
  , optimization_step_( 1 )
{
}

void
WeightOptimizer::get_status( DictionaryDatum& d ) const
{
}

void
WeightOptimizer::set_status( const DictionaryDatum& d )
{
}

double
WeightOptimizer::optimized_weight( const WeightOptimizerCommonProperties& cp,
  const size_t idx_current_update,
  const double gradient_change,
  double weight )
{
  sum_gradients_ += gradient_change;

  const size_t current_optimization_step = 1 + idx_current_update / cp.batch_size_;
  if ( optimization_step_ < current_optimization_step )
  {
    sum_gradients_ /= cp.batch_size_;
    weight = std::max( cp.Wmin_, std::min( optimize_( cp, weight, current_optimization_step ), cp.Wmax_ ) );
    optimization_step_ = current_optimization_step;
  }
  return weight;
}

WeightOptimizerCommonProperties*
WeightOptimizerCommonPropertiesGradientDescent::clone() const
{
  return new WeightOptimizerCommonPropertiesGradientDescent( *this );
}

WeightOptimizer*
WeightOptimizerCommonPropertiesGradientDescent::get_optimizer() const
{
  return new WeightOptimizerGradientDescent();
}

WeightOptimizerGradientDescent::WeightOptimizerGradientDescent()
  : WeightOptimizer()
{
}

double
WeightOptimizerGradientDescent::optimize_( const WeightOptimizerCommonProperties& cp, double weight, size_t )
{
  weight -= cp.eta_ * sum_gradients_;
  sum_gradients_ = 0;
  return weight;
}

WeightOptimizerCommonPropertiesAdam::WeightOptimizerCommonPropertiesAdam()
  : WeightOptimizerCommonProperties()
  , beta1_( 0.9 )
  , beta2_( 0.999 )
  , epsilon_( 1e-8 )
{
}


WeightOptimizerCommonProperties*
WeightOptimizerCommonPropertiesAdam::clone() const
{
  return new WeightOptimizerCommonPropertiesAdam( *this );
}

WeightOptimizer*
WeightOptimizerCommonPropertiesAdam::get_optimizer() const
{
  return new WeightOptimizerAdam();
}

void
WeightOptimizerCommonPropertiesAdam::get_status( DictionaryDatum& d ) const
{
  WeightOptimizerCommonProperties::get_status( d );

  def< double >( d, names::beta_1, beta1_ );
  def< double >( d, names::beta_2, beta2_ );
  def< double >( d, names::epsilon, epsilon_ );
}

void
WeightOptimizerCommonPropertiesAdam::set_status( const DictionaryDatum& d )
{
  WeightOptimizerCommonProperties::set_status( d );

  updateValue< double >( d, names::beta_1, beta1_ );
  updateValue< double >( d, names::beta_2, beta2_ );
  updateValue< double >( d, names::epsilon, epsilon_ );

  if ( beta1_ < 0.0 or 1.0 <= beta1_ )
  {
    throw BadProperty( "For Adam optimizer, beta_1 from interval [0,1) required." );
  }

  if ( beta2_ < 0.0 or 1.0 <= beta2_ )
  {
    throw BadProperty( "For Adam optimizer, beta_2 from interval [0,1) required." );
  }

  if ( epsilon_ < 0.0 )
  {
    throw BadProperty( "For Adam optimizer, epsilon ≥ 0 required." );
  }
}

WeightOptimizerAdam::WeightOptimizerAdam()
  : WeightOptimizer()
  , m_( 0.0 )
  , v_( 0.0 )
{
}

void
WeightOptimizerAdam::get_status( DictionaryDatum& d ) const
{
  WeightOptimizer::get_status( d );
  def< double >( d, names::m, m_ );
  def< double >( d, names::v, v_ );
}

void
WeightOptimizerAdam::set_status( const DictionaryDatum& d )
{
  WeightOptimizer::set_status( d );
  updateValue< double >( d, names::m, m_ );
  updateValue< double >( d, names::v, v_ );
}


double
WeightOptimizerAdam::optimize_( const WeightOptimizerCommonProperties& cp,
  double weight,
  size_t current_optimization_step )
{
  const WeightOptimizerCommonPropertiesAdam& acp = dynamic_cast< const WeightOptimizerCommonPropertiesAdam& >( cp );

  for ( ; optimization_step_ < current_optimization_step; ++optimization_step_ )
  {
    const double beta1_factor = 1.0 - std::pow( acp.beta1_, optimization_step_ );
    const double beta2_factor = 1.0 - std::pow( acp.beta2_, optimization_step_ );

    const double alpha_t = cp.eta_ * std::sqrt( beta2_factor ) / beta1_factor;

    m_ = acp.beta1_ * m_ + ( 1.0 - acp.beta1_ ) * sum_gradients_;
    v_ = acp.beta2_ * v_ + ( 1.0 - acp.beta2_ ) * sum_gradients_ * sum_gradients_;

    weight -= alpha_t * m_ / ( std::sqrt( v_ ) + acp.epsilon_ );

    // Set gradients to zero for following iterations since more than
    // one cycle indicates past learning periods with vanishing gradients
    sum_gradients_ = 0.0; // reset for following iterations
  }

  return weight;
}

} // namespace nest
