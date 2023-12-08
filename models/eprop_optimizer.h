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

class EpropOptimizer;

/**
 * Base class for common properties for eprop optimizers.
 *
 * The CommonProperties of eprop synapse models own an object of this class hierarchy.
 * The values in these objects are used by the synapse-specific optimizer object.
 * Change of optimizer type is only possible before synapses of the model have been created.
 */
class EpropOptimizerCommonProperties
{
public:
  EpropOptimizerCommonProperties();
  virtual ~EpropOptimizerCommonProperties()
  {
  }
  EpropOptimizerCommonProperties( const EpropOptimizerCommonProperties& );
  EpropOptimizer& operator=( const EpropOptimizer& ) = delete;

  virtual void get_status( DictionaryDatum& d ) const;
  virtual void set_status( const DictionaryDatum& d );

  virtual EpropOptimizerCommonProperties* clone() const = 0;
  virtual EpropOptimizer* get_optimizer() const = 0;

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
class EpropOptimizer
{
public:
  EpropOptimizer();
  virtual ~EpropOptimizer()
  {
    // std::cout << "Deleting optimizer " << this << std::endl;
  }
  EpropOptimizer( const EpropOptimizer& ) = default;
  EpropOptimizer& operator=( const EpropOptimizer& ) = delete;

  //! Return optimized weight based on current weight
  double optimized_weight( const EpropOptimizerCommonProperties& cp,
    const size_t idx_current_update,
    const double gradient_change,
    double weight );

protected:
  //! Actually perform specific optimization, called by optimized_weight()
  virtual double do_optimize_( const EpropOptimizerCommonProperties& cp, double weight, size_t current_opt_step ) = 0;

  double sum_gradients_;     //!< Sum of gradients accumulated in current batch
  size_t optimization_step_; //!< Current optimization step; optimization happens evert batch_size_ steps.
};


class EpropOptimizerGradientDescent : public EpropOptimizer
{
public:
  EpropOptimizerGradientDescent();
  EpropOptimizerGradientDescent( const EpropOptimizerGradientDescent& ) = default;
  EpropOptimizerGradientDescent& operator=( const EpropOptimizerGradientDescent& ) = delete;

private:
  double do_optimize_( const EpropOptimizerCommonProperties& cp, double weight, size_t current_opt_step ) override;
};

class EpropOptimizerCommonPropertiesGradientDescent : public EpropOptimizerCommonProperties
{
  friend class EpropOptimizerGradientDescent;

public:
  EpropOptimizerCommonPropertiesGradientDescent& operator=(
    const EpropOptimizerCommonPropertiesGradientDescent& ) = delete;

  EpropOptimizerCommonProperties* clone() const override;
  EpropOptimizer* get_optimizer() const override;

  std::string
  get_name() const override
  {
    return "gradient_descent";
  }
};

class EpropOptimizerAdam : public EpropOptimizer
{
public:
  EpropOptimizerAdam();
  EpropOptimizerAdam( const EpropOptimizerAdam& ) = default;
  EpropOptimizerAdam& operator=( const EpropOptimizerAdam& ) = delete;

private:
  //! Return optimized weight based on current weight
  double do_optimize_( const EpropOptimizerCommonProperties& cp, double weight, size_t current_opt_step ) override;

  double adam_m_;
  double adam_v_;
};


class EpropOptimizerCommonPropertiesAdam : public EpropOptimizerCommonProperties
{
  friend class EpropOptimizerAdam;

public:
  EpropOptimizerCommonPropertiesAdam();
  EpropOptimizerCommonPropertiesAdam& operator=( const EpropOptimizerCommonPropertiesAdam& ) = delete;

  EpropOptimizerCommonProperties* clone() const override;
  EpropOptimizer* get_optimizer() const override;

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

#endif // EPROP_OPTIMIZER_H
