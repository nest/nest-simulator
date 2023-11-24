/*
 *  eprop_optimizer.h
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

#ifndef EPROP_OPTIMIZER_H
#define EPROP_OPTIMIZER_H

// Includes from sli
#include "dictdatum.h"

namespace nest
{

/**
 * Base class for e-prop optimizers.
 *
 * An optimizer is used by an e-prop synapse to optimize the weight.
 *
 * An optimizer may have internal state which is maintained from call to call of the `optimized_weight()` method.
 * Each optimized object belongs to exactly one e-prop synapse.
 */
class EpropOptimizer
{
public:
  EpropOptimizer();
  virtual ~EpropOptimizer()
  {
  }
  EpropOptimizer( const EpropOptimizer& ) = delete;
  EpropOptimizer& operator=( const EpropOptimizer& ) = delete;

  virtual void get_status( DictionaryDatum& d ) const;
  virtual void set_status( const DictionaryDatum& d );

  //! Return optimized weight based on current weight
  double optimized_weight( const size_t idx_current_update, const double gradient_change, double weight );

  double
  get_Wmin() const
  {
    return Wmin_;
  }

  double
  get_Wmax() const
  {
    return Wmax_;
  }

  virtual std::string get_name() const = 0;

protected:
  //! Actually perform specific optimization, called by optimized_weight()
  virtual double do_optimize_( double weight, size_t current_opt_step ) = 0;

  size_t batch_size_; //!< size of optimization batches
  double eta_;        //!< learning rate
  double Wmin_;       //!< lower bound for weight
  double Wmax_;       //!< upper bound for weight

  double sum_gradients_;     //!< Sum of gradients accumulated in current batch
  size_t optimization_step_; //!< Current optimization step; optimization happens evert batch_size_ steps.
};

class EpropOptimizerGradientDescent : public EpropOptimizer
{
public:
  EpropOptimizerGradientDescent();
  EpropOptimizerGradientDescent( const EpropOptimizerGradientDescent& ) = delete;
  EpropOptimizerGradientDescent& operator=( const EpropOptimizerGradientDescent& ) = delete;

  std::string
  get_name() const override
  {
    return "gradient_descent";
  }

protected:
  //! Return optimized weight based on current weight
  double do_optimize_( double weight, size_t current_opt_step ) override;
};

class EpropOptimizerAdam : public EpropOptimizer
{
public:
  EpropOptimizerAdam();
  EpropOptimizerAdam( const EpropOptimizerAdam& ) = delete;
  EpropOptimizerAdam& operator=( const EpropOptimizerAdam& ) = delete;

  void get_status( DictionaryDatum& d ) const override;
  void set_status( const DictionaryDatum& d ) override;

  std::string
  get_name() const override
  {
    return "adam";
  }

protected:
  //! Return optimized weight based on current weight
  double do_optimize_( double weight, size_t current_opt_step ) override;

private:
  double beta1_;   //!< Exponential decay rate for first moment estimate of Adam optimizer.
  double beta2_;   //!< Exponential decay rate for second moment estimate of Adam optimizer.
  double epsilon_; //!< Small constant for numerical stability of Adam optimizer.

  double adam_m_;
  double adam_v_;
};

} // namespace nest

#endif // EPROP_OPTIMIZER_H
