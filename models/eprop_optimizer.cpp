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

// nestkernel
#include "nest_impl.h"


namespace nest
{
EpropOptimizer::EpropOptimizer()
  : batch_size_( 1 )
  , eta_( 1e-4 )
  , Wmin_( 0.0 )
  , Wmax_( 100.0 )
  , sum_gradients_( 0.0 )
  , optimization_step_( 0 )
{
}

void
EpropOptimizer::get_status( DictionaryDatum& d ) const
{
  def< long >( d, names::batch_size, batch_size_ );
  def< double >( d, names::eta, eta_ );
  def< double >( d, names::Wmin, Wmin_ );
  def< double >( d, names::Wmax, Wmax_ );
}

void
EpropOptimizer::set_status( const DictionaryDatum& d )
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
  updateValue< double >( d, names::Wmin, Wmin_ );
  updateValue< double >( d, names::Wmax, Wmax_ );
  if ( new_Wmin > new_Wmax )
  {
    throw BadProperty( "Wmin ≤ Wmax required." );
  }
  Wmin_ = new_Wmin;
  Wmax_ = new_Wmax;
}

double
EpropOptimizer::optimized_weight( const size_t idx_current_update, const double gradient_change, double weight )
{
  sum_gradients_ += gradient_change;

  const size_t current_optimization_step = 1 + idx_current_update / batch_size_;
  if ( optimization_step_ < current_optimization_step )
  {
    sum_gradients_ /= batch_size_;
    weight = std::max( Wmin_, std::min( do_optimize_( weight, current_optimization_step ), Wmax_ ) );
    optimization_step_ = current_optimization_step;
  }
  return weight;
}

EpropOptimizerGradientDescent::EpropOptimizerGradientDescent()
  : EpropOptimizer()
{
}

double
EpropOptimizerGradientDescent::do_optimize_( double weight, size_t )
{
  weight -= eta_ * sum_gradients_;
  sum_gradients_ = 0;
  return weight;
}

EpropOptimizerAdam::EpropOptimizerAdam()
  : EpropOptimizer()
  , beta1_( 0.9 )
  , beta2_( 0.999 )
  , epsilon_( 1e-8 )
  , adam_m_( 0.0 )
  , adam_v_( 0.0 )
{
}

void
EpropOptimizerAdam::get_status( DictionaryDatum& d ) const
{
  EpropOptimizer::get_status( d );

  def< double >( d, names::adam_beta1, beta1_ );
  def< double >( d, names::adam_beta2, beta2_ );
  def< double >( d, names::adam_epsilon, epsilon_ );
  def< double >( d, names::adam_m, adam_m_ );
  def< double >( d, names::adam_v, adam_v_ );
}

void
EpropOptimizerAdam::set_status( const DictionaryDatum& d )
{
  EpropOptimizer::set_status( d );

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

double
EpropOptimizerAdam::do_optimize_( double weight, size_t current_optimization_step )
{
  for ( ; optimization_step_ < current_optimization_step; ++optimization_step_ )
  {
    const double adam_beta1_factor = 1.0 - std::pow( beta1_, optimization_step_ );
    const double adam_beta2_factor = 1.0 - std::pow( beta2_, optimization_step_ );

    const double alpha_t = eta_ * std::sqrt( adam_beta2_factor ) / adam_beta1_factor;

    adam_m_ = beta1_ * adam_m_ + ( 1.0 - beta1_ ) * sum_gradients_;
    adam_v_ = beta2_ * adam_v_ + ( 1.0 - beta2_ ) * sum_gradients_ * sum_gradients_;

    weight -= alpha_t * adam_m_ / ( std::sqrt( adam_v_ ) + epsilon_ );

    // Set gradients to zero for following iterations since more than
    // one cycle indicates past learning periods with vanishing gradients
    sum_gradients_ = 0.0; // reset for following iterations
  }

  return weight;
}

} // namespace nest
