/*
 *  weight_optimizer.h
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

#ifndef WEIGHT_OPTIMIZER_H
#define WEIGHT_OPTIMIZER_H

// Includes from sli
#include "dictdatum.h"

namespace nest
{

class WeightOptimizer;

/**
 * Base class for common properties for eprop optimizers.
 *
 * The CommonProperties of eprop synapse models own an object of this class hierarchy.
 * The values in these objects are used by the synapse-specific optimizer object.
 * Change of optimizer type is only possible before synapses of the model have been created.
 */
class WeightOptimizerCommonProperties
{
public:
  WeightOptimizerCommonProperties();
  virtual ~WeightOptimizerCommonProperties()
  {
  }
  WeightOptimizerCommonProperties( const WeightOptimizerCommonProperties& );
  WeightOptimizer& operator=( const WeightOptimizer& ) = delete;

  virtual void get_status( DictionaryDatum& d ) const;
  virtual void set_status( const DictionaryDatum& d );

  virtual WeightOptimizerCommonProperties* clone() const = 0;
  virtual WeightOptimizer* get_optimizer() const = 0;

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

public:
  size_t batch_size_; //!< size of optimization batches
  double eta_;        //!< learning rate
  double Wmin_;       //!< lower bound for weight
  double Wmax_;       //!< upper bound for weight
};

/**
 * Base class for e-prop optimizers.
 *
 * An optimizer is used by an e-prop synapse to optimize the weight.
 *
 * An optimizer may have internal state which is maintained from call to call of the `optimized_weight()` method.
 * Each optimized object belongs to exactly one e-prop synapse.
 */
class WeightOptimizer
{
public:
  WeightOptimizer();
  virtual ~WeightOptimizer()
  {
  }
  WeightOptimizer( const WeightOptimizer& ) = default;
  WeightOptimizer& operator=( const WeightOptimizer& ) = delete;

  virtual void get_status( DictionaryDatum& d ) const;
  virtual void set_status( const DictionaryDatum& d );

  //! Return optimized weight based on current weight
  double optimized_weight( const WeightOptimizerCommonProperties& cp,
    const size_t idx_current_update,
    const double gradient_change,
    double weight );

protected:
  //! Actually perform specific optimization, called by optimized_weight()
  virtual double optimize_( const WeightOptimizerCommonProperties& cp, double weight, size_t current_opt_step ) = 0;

  double sum_gradients_;     //!< Sum of gradients accumulated in current batch
  size_t optimization_step_; //!< Current optimization step; optimization happens evert batch_size_ steps.
};


class WeightOptimizerGradientDescent : public WeightOptimizer
{
public:
  WeightOptimizerGradientDescent();
  WeightOptimizerGradientDescent( const WeightOptimizerGradientDescent& ) = default;
  WeightOptimizerGradientDescent& operator=( const WeightOptimizerGradientDescent& ) = delete;

private:
  double optimize_( const WeightOptimizerCommonProperties& cp, double weight, size_t current_opt_step ) override;
};

class WeightOptimizerCommonPropertiesGradientDescent : public WeightOptimizerCommonProperties
{
  friend class WeightOptimizerGradientDescent;

public:
  WeightOptimizerCommonPropertiesGradientDescent& operator=(
    const WeightOptimizerCommonPropertiesGradientDescent& ) = delete;

  WeightOptimizerCommonProperties* clone() const override;
  WeightOptimizer* get_optimizer() const override;

  std::string
  get_name() const override
  {
    return "gradient_descent";
  }
};

class WeightOptimizerAdam : public WeightOptimizer
{
public:
  WeightOptimizerAdam();
  WeightOptimizerAdam( const WeightOptimizerAdam& ) = default;
  WeightOptimizerAdam& operator=( const WeightOptimizerAdam& ) = delete;

  void get_status( DictionaryDatum& d ) const override;
  void set_status( const DictionaryDatum& d ) override;

private:
  //! Return optimized weight based on current weight
  double optimize_( const WeightOptimizerCommonProperties& cp, double weight, size_t current_opt_step ) override;

  double m_;
  double v_;
};


class WeightOptimizerCommonPropertiesAdam : public WeightOptimizerCommonProperties
{
  friend class WeightOptimizerAdam;

public:
  WeightOptimizerCommonPropertiesAdam();
  WeightOptimizerCommonPropertiesAdam& operator=( const WeightOptimizerCommonPropertiesAdam& ) = delete;

  WeightOptimizerCommonProperties* clone() const override;
  WeightOptimizer* get_optimizer() const override;

  void get_status( DictionaryDatum& d ) const override;
  void set_status( const DictionaryDatum& d ) override;

  std::string
  get_name() const override
  {
    return "adam";
  }

private:
  double beta1_;   //!< Exponential decay rate for first moment estimate of Adam optimizer.
  double beta2_;   //!< Exponential decay rate for second moment estimate of Adam optimizer.
  double epsilon_; //!< Small constant for numerical stability of Adam optimizer.
};

} // namespace nest

#endif // WEIGHT_OPTIMIZER_H
