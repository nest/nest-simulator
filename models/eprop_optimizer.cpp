/*
 *  eprop_optimizer.cpp
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

#include "eprop_optimizer.h"

// Includes from nestkernel
#include "exceptions.h"
#include "nest_names.h"

// Includes from sli
#include "dictutils.h"

namespace nest
{
EpropOptimizerCommonProperties::EpropOptimizerCommonProperties()
  : batch_size_( 1 )
  , eta_( 1e-4 )
  , Wmin_( 0.0 )
  , Wmax_( 100.0 )
{
}

EpropOptimizerCommonProperties::EpropOptimizerCommonProperties( const EpropOptimizerCommonProperties& cp )
  : batch_size_( cp.batch_size_ )
  , eta_( cp.eta_ )
  , Wmin_( cp.Wmin_ )
  , Wmax_( cp.Wmax_ )
{
}

void
EpropOptimizerCommonProperties::get_status( DictionaryDatum& d ) const
{
  def< std::string >( d, names::optimizer, get_name() );
  def< long >( d, names::batch_size, batch_size_ );
  def< double >( d, names::eta, eta_ );
  def< double >( d, names::Wmin, Wmin_ );
  def< double >( d, names::Wmax, Wmax_ );
}

void
EpropOptimizerCommonProperties::set_status( const DictionaryDatum& d )
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
    throw BadProperty( "Wmin ≤ Wmax required." );
  }
  Wmin_ = new_Wmin;
  Wmax_ = new_Wmax;
}

EpropOptimizer::EpropOptimizer()
  : sum_gradients_( 0.0 )
  , optimization_step_( 1 )
{
  // std::cout << "Creating optimizer " << this << std::endl;
}

double
EpropOptimizer::optimized_weight( const EpropOptimizerCommonProperties& cp,
  const size_t idx_current_update,
  const double gradient_change,
  double weight )
{
  sum_gradients_ += gradient_change;

  const size_t current_optimization_step = 1 + idx_current_update / cp.batch_size_;
  if ( optimization_step_ < current_optimization_step )
  {
    sum_gradients_ /= cp.batch_size_;
    weight = std::max( cp.Wmin_, std::min( do_optimize_( cp, weight, current_optimization_step ), cp.Wmax_ ) );
    optimization_step_ = current_optimization_step;
  }
  return weight;
}

EpropOptimizerCommonProperties*
EpropOptimizerCommonPropertiesGradientDescent::clone() const
{
  return new EpropOptimizerCommonPropertiesGradientDescent( *this );
}

EpropOptimizer*
EpropOptimizerCommonPropertiesGradientDescent::get_optimizer() const
{
  return new EpropOptimizerGradientDescent();
}

EpropOptimizerGradientDescent::EpropOptimizerGradientDescent()
  : EpropOptimizer()
{
}

double
EpropOptimizerGradientDescent::do_optimize_( const EpropOptimizerCommonProperties& cp, double weight, size_t )
{
  weight -= cp.eta_ * sum_gradients_;
  sum_gradients_ = 0;
  return weight;
}

EpropOptimizerCommonPropertiesAdam::EpropOptimizerCommonPropertiesAdam()
  : EpropOptimizerCommonProperties()
  , beta1_( 0.9 )
  , beta2_( 0.999 )
  , epsilon_( 1e-8 )
{
}


EpropOptimizerCommonProperties*
EpropOptimizerCommonPropertiesAdam::clone() const
{
  return new EpropOptimizerCommonPropertiesAdam( *this );
}

EpropOptimizer*
EpropOptimizerCommonPropertiesAdam::get_optimizer() const
{
  return new EpropOptimizerAdam();
}

void
EpropOptimizerCommonPropertiesAdam::get_status( DictionaryDatum& d ) const
{
  EpropOptimizerCommonProperties::get_status( d );

  def< double >( d, names::adam_beta1, beta1_ );
  def< double >( d, names::adam_beta2, beta2_ );
  def< double >( d, names::adam_epsilon, epsilon_ );
}

void
EpropOptimizerCommonPropertiesAdam::set_status( const DictionaryDatum& d )
{
  EpropOptimizerCommonProperties::set_status( d );

  updateValue< double >( d, names::adam_beta1, beta1_ );
  updateValue< double >( d, names::adam_beta2, beta2_ );
  updateValue< double >( d, names::adam_epsilon, epsilon_ );

  if ( beta1_ < 0.0 or 1.0 <= beta1_ )
  {
    throw BadProperty( "adam_beta1 must be in [0,1)." );
  }

  if ( beta2_ < 0.0 or 1.0 <= beta2_ )
  {
    throw BadProperty( "adam_beta2 must be in [0,1)." );
  }

  if ( epsilon_ < 0.0 )
  {
    throw BadProperty( "adam_epsilon must be >= 0." );
  }
}

EpropOptimizerAdam::EpropOptimizerAdam()
  : EpropOptimizer()
  , adam_m_( 0.0 )
  , adam_v_( 0.0 )
{
}

double
EpropOptimizerAdam::do_optimize_( const EpropOptimizerCommonProperties& cp,
  double weight,
  size_t current_optimization_step )
{
  const EpropOptimizerCommonPropertiesAdam& acp = dynamic_cast< const EpropOptimizerCommonPropertiesAdam& >( cp );

  for ( ; optimization_step_ < current_optimization_step; ++optimization_step_ )
  {
    const double adam_beta1_factor = 1.0 - std::pow( acp.beta1_, optimization_step_ );
    const double adam_beta2_factor = 1.0 - std::pow( acp.beta2_, optimization_step_ );

    const double alpha_t = cp.eta_ * std::sqrt( adam_beta2_factor ) / adam_beta1_factor;

    adam_m_ = acp.beta1_ * adam_m_ + ( 1.0 - acp.beta1_ ) * sum_gradients_;
    adam_v_ = acp.beta2_ * adam_v_ + ( 1.0 - acp.beta2_ ) * sum_gradients_ * sum_gradients_;

    weight -= alpha_t * adam_m_ / ( std::sqrt( adam_v_ ) + acp.epsilon_ );

    // Set gradients to zero for following iterations since more than
    // one cycle indicates past learning periods with vanishing gradients
    sum_gradients_ = 0.0; // reset for following iterations
  }

  return weight;
}

} // namespace nest
