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

/* BeginUserDocs: e-prop plasticity

Short description
+++++++++++++++++

Selection of weight optimizers

Description
+++++++++++

Currently two weight optimizers are implemented: gradient descent and the Adam optimizer.

In gradient descent [1]_ the weights are optimized via:

.. math::
  W_t = W_{t-1} - \eta g_t \,,

whereby :math:`\eta` denotes the learning rate and :math:`g_t` the gradient of the current
time step :math:`t`.

In the Adam scheme [2]_ the weights are optimized via:

.. math::
  m_0 &= 0, v_0 = 0, t = 1 \,, \\
  m_t &= \beta_1 \cdot m_{t-1} + \left(1-\beta_1\right) \cdot g_t \,, \\
  v_t &= \beta_2 \cdot v_{t-1} + \left(1-\beta_2\right) \cdot g_t^2 \,, \\
  \hat{m}_t &= \frac{m_t}{1-\beta_1^t} \,, \\
  \hat{v}_t &= \frac{v_t}{1-\beta_2^t} \,, \\
  W_t &= W_{t-1} - \eta\frac{\hat{m_t}}{\sqrt{\hat{v}_t} + \epsilon} \,.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

========== ===== ========================= ================ =========================================================
**General optimizer parameters**
---------------------------------------------------------------------------------------------------------------------
Parameter   Unit  Math equivalent          Default          Description
========== ===== ========================= ================ =========================================================
batch_size                                                1 Size of batch
eta              :math:`\eta`                          1e-4 Learning rate
Wmax          pA :math:`W_{ji}^\text{max}`            100.0 Maximal value for synaptic weight
Wmin          pA :math:`W_{ji}^\text{min}`              0.0 Minimal value for synaptic weight
========== ===== ========================= ================ =========================================================

========== ===== ========================= ================ =========================================================
**Gradient descent parameters (default optimizer)**
---------------------------------------------------------------------------------------------------------------------
Parameter   Unit  Math equivalent          Default          Description
========== ===== ========================= ================ =========================================================
type                                       gradient_descent Optimizer type
========== ===== ========================= ================ =========================================================

========== ===== ========================= ================ =========================================================
**Adam optimizer parameters**
---------------------------------------------------------------------------------------------------------------------
Parameter   Unit  Math equivalent          Default          Description
========== ===== ========================= ================ =========================================================
type                                                   adam Optimizer type
beta_1            :math:`\beta_1`                        0.9 Exponential decay rate for first moment estimate of Adam
                                                            optimizer
beta_2            :math:`\beta_2`                      0.999 Exponential decay rate for second moment estimate of Adam
                                                            optimizer
epsilon          :math:`\epsilon`                      1e-8 Small constant for numerical stability of Adam optimizer
========== ===== ========================= ================ =========================================================

========= ==== =============== =======  ===============================================================
**Adam optimizer parameters for individual synapses**
-------------------------------------------------------------------------------------------------------
Parameter Unit Math equivalent Default  Description
========= ==== =============== =======  ===============================================================
m              :math:`m_0`         0.0  Initial value of first moment estimate m of Adam optimizer
v              :math:`v_0`         0.0  Initial value of second moment raw estimate v of Adam optimizer
========= ==== =============== =======  ===============================================================


References
++++++++++
.. [1] Huh, D. & Sejnowski, T. J. Gradient descent for spiking neural networks. 32nd
       Conference on Neural Information Processing Systems (2018).
.. [2] Kingma DP, Ba JL (2015). Adam: A method for stochastic optimization.
       Proceedings of International Conference on Learning Representations (ICLR).
       https://doi.org/10.48550/arXiv.1412.6980

See also
++++++++

Examples using this model
++++++++++++++++++++++++++

.. listexamples:: eprop_synapse

EndUserDocs */

class WeightOptimizer;

/**
 * Base class implementing common properties of a weight optimizer model.
 *
 * The CommonProperties of synapse models supporting weight optimization own an object of this class hierarchy.
 * The values in these objects are used by the synapse-specific optimizer object.
 * Change of the optimizer type is only possible before synapses of the model have been created.
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
 * Base class implementing a weight optimizer model.
 *
 * An optimizer is used by a synapse that supports this mechanism to optimize the weight.
 *
 * An optimizer may have an internal state which is maintained from call to call of the `optimized_weight()` method.
 * Each optimized object belongs to exactly one synapse.
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

/**
 * Base class implementing a gradient descent weight optimizer model.
 */
class WeightOptimizerGradientDescent : public WeightOptimizer
{
public:
  WeightOptimizerGradientDescent();
  WeightOptimizerGradientDescent( const WeightOptimizerGradientDescent& ) = default;
  WeightOptimizerGradientDescent& operator=( const WeightOptimizerGradientDescent& ) = delete;

private:
  double optimize_( const WeightOptimizerCommonProperties& cp, double weight, size_t current_opt_step ) override;
};

/**
 * Base class implementing common properties of a gradient descent weight optimizer model.
 */
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

/**
 * Base class implementing an Adam weight optimizer model.
 */
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

/**
 * Base class implementing common properties of an Adam weight optimizer model.
 */
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
  double beta_1_;  //!< Exponential decay rate for first moment estimate.
  double beta_2_;  //!< Exponential decay rate for second moment estimate.
  double epsilon_; //!< Small constant for numerical stability.
};

} // namespace nest

#endif // WEIGHT_OPTIMIZER_H
