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

/* BeginUserDocs: e-prop plasticity, synapse

Short description
+++++++++++++++++

Selection of weight optimizers

Description
+++++++++++
A weight optimizer is an algorithm that adjusts the synaptic weights in a
network during training to minimize the loss function and thus improve the
network's performance on a given task.

This method is an essential part of plasticity rules like e-prop plasticity.

Currently two weight optimizers are implemented: gradient descent and the Adam optimizer.

In gradient descent [1]_ the weights are optimized via:

.. math::
  W_t = W_{t-1} - \eta g_t \,, \\

where :math:`\eta` denotes the learning rate and :math:`g_t` the gradient of the current
time step :math:`t`.

In the Adam scheme [2]_ the weights are optimized via:

.. math::
  m_0 &= 0, v_0 = 0, t = 1 \,, \\
  m_t &= \beta_1 m_{t-1} + \left( 1- \beta_1 \right) g_t \,, \\
  v_t &= \beta_2 v_{t-1} + \left( 1 - \beta_2 \right) g_t^2 \,, \\
  \alpha_t &= \eta \frac{ \sqrt{ 1- \beta_2^t } }{ 1 - \beta_1^t } \,, \\
  W_t &= W_{t-1} - \alpha_t \frac{ m_t }{ \sqrt{v_t} + \hat{\epsilon} } \,. \\

Note that the implementation follows the implementation in TensorFlow [3]_ for comparability.
The TensorFlow implementation deviates from [1]_ in that it assumes
:math:`\hat{\epsilon} = \epsilon \sqrt{ 1 - \beta_2^t }` to be constant, whereas [1]_
assumes :math:`\epsilon = \hat{\epsilon} \sqrt{ 1 - \beta_2^t }` to be constant.

When `optimize_each_step` is set to `True`, the weights are optimized at every
time step. If set to `False`, optimization occurs once per spike, resulting in a
significant speed-up. For gradient descent, both settings yield the same
results under exact arithmetic; however, small numerical differences may be
observed due to floating point precision. For the Adam optimizer, only setting
`optimize_each_step` to `True` precisely implements the algorithm as described
in [2]_. The impact of this setting on learning performance may vary depending
on the task.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

====================== ==== ========================= ========= =================================
**Common optimizer parameters**
-------------------------------------------------------------------------------------------------
Parameter              Unit Math equivalent           Default   Description
====================== ==== ========================= ========= =================================
``batch_size``                                              1   Size of batch
``eta``                     :math:`\eta`                 1e-4   Learning rate
``optimize_each_step``                                 ``True``
``Wmax``                pA  :math:`W_{ji}^\text{max}`   100.0   Maximal value for synaptic weight
``Wmin``                pA  :math:`W_{ji}^\text{min}`  -100.0   Minimal value for synaptic weight
====================== ==== ========================= ========= =================================

========= ==== =============== ================== ==============
**Gradient descent parameters (default optimizer)**
----------------------------------------------------------------
Parameter Unit Math equivalent Default            Description
========= ==== =============== ================== ==============
``type``                       "gradient_descent" Optimizer type
========= ==== =============== ================== ==============

=========== ==== ================ ======= =================================================
**Adam optimizer parameters**
-------------------------------------------------------------------------------------------
Parameter   Unit Math equivalent  Default Description
=========== ==== ================ ======= =================================================
``type``                           "adam" Optimizer type
``beta_1``       :math:`\beta_1`      0.9 Exponential decay rate for first moment estimate
``beta_2``       :math:`\beta_2`    0.999 Exponential decay rate for second moment estimate
``epsilon``      :math:`\epsilon`    1e-7 Small constant for numerical stability
=========== ==== ================ ======= =================================================

The following state variables evolve during simulation.

============== ==== =============== ============= ==========================
**Adam optimizer state variables for individual synapses**
----------------------------------------------------------------------------
State variable Unit Math equivalent Initial value Description
============== ==== =============== ============= ==========================
``m``               :math:`m`                 0.0 First moment estimate
``v``               :math:`v`                 0.0 Second moment raw estimate
============== ==== =============== ============= ==========================


References
++++++++++

.. [1] Huh D, Sejnowski TJ (2018). Gradient descent for spiking neural networks.
       Advances in Neural Information Processing Systems, 31:1433-1443.
       https://proceedings.neurips.cc/paper_files/paper/2018/hash/185e65bc40581880c4f2c82958de8cfe-Abstract.html

.. [2] Kingma DP, Ba JL (2015). Adam: A method for stochastic optimization.
       Proceedings of 3rd International Conference for Learning Representations (ICLR).
       https://doi.org/10.48550/arXiv.1412.6980

.. [3] https://github.com/keras-team/keras/blob/v2.15.0/keras/optimizers/adam.py#L26-L220

See also
++++++++

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: eprop_synapse_bsshslm_2020

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
  //! Default constructor.
  WeightOptimizerCommonProperties();

  //! Destructor.
  virtual ~WeightOptimizerCommonProperties()
  {
  }

  //! Copy constructor.
  WeightOptimizerCommonProperties( const WeightOptimizerCommonProperties& );

  //! Assignment operator.
  WeightOptimizer& operator=( const WeightOptimizer& ) = delete;

  //! Get parameter dictionary.
  virtual void get_status( DictionaryDatum& d ) const;

  //! Update parameters in parameter dictionary.
  virtual void set_status( const DictionaryDatum& d );

  //! Clone constructor.
  virtual WeightOptimizerCommonProperties* clone() const = 0;

  //! Get optimizer.
  virtual WeightOptimizer* get_optimizer() const = 0;

  //! Get minimal value for synaptic weight.
  double
  get_Wmin() const
  {
    return Wmin_;
  }

  //! Get maximal value for synaptic weight.
  double
  get_Wmax() const
  {
    return Wmax_;
  }

  //! Get optimizer name.
  virtual std::string get_name() const = 0;

public:
  //! Size of an optimization batch.
  size_t batch_size_;

  //! Learning rate common to all synapses.
  double eta_;

  //! First learning rate that differs from the default.
  double eta_first_;

  //! Number of changes to the learning rate.
  long n_eta_change_;

  //! Minimal value for synaptic weight.
  double Wmin_;

  //! Maximal value for synaptic weight.
  double Wmax_;

  //! If true, optimize each step, else once per spike.
  bool optimize_each_step_;
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
  //! Default constructor.
  WeightOptimizer();

  //! Destructor.
  virtual ~WeightOptimizer()
  {
  }

  //! Copy constructor.
  WeightOptimizer( const WeightOptimizer& ) = default;

  //! Assignment operator.
  WeightOptimizer& operator=( const WeightOptimizer& ) = delete;

  //! Get parameter dictionary.
  virtual void get_status( DictionaryDatum& d ) const;

  //! Update values in parameter dictionary.
  virtual void set_status( const DictionaryDatum& d );

  //! Return optimized weight based on current weight.
  double optimized_weight( WeightOptimizerCommonProperties& cp,
    const size_t idx_current_update,
    const double gradient,
    double weight );

protected:
  //! Perform specific optimization.
  virtual double optimize_( const WeightOptimizerCommonProperties& cp, double weight, size_t current_opt_step ) = 0;

  //! Sum of gradients accumulated in current batch.
  double sum_gradients_;

  //! Current optimization step, whereby optimization happens every batch_size_ steps.
  size_t optimization_step_;

  //! Learning rate private to the synapse.
  double eta_;

  //! Number of optimizations.
  long n_optimize_;
};

/**
 * Base class implementing a gradient descent weight optimizer model.
 */
class WeightOptimizerGradientDescent : public WeightOptimizer
{
public:
  //! Default constructor.
  WeightOptimizerGradientDescent();

  //! Copy constructor.
  WeightOptimizerGradientDescent( const WeightOptimizerGradientDescent& ) = default;

  //! Assignment operator.
  WeightOptimizerGradientDescent& operator=( const WeightOptimizerGradientDescent& ) = delete;

private:
  double optimize_( const WeightOptimizerCommonProperties& cp, double weight, size_t current_opt_step ) override;
};

/**
 * Class implementing common properties of a gradient descent weight optimizer model.
 */
class WeightOptimizerCommonPropertiesGradientDescent : public WeightOptimizerCommonProperties
{
  //! Friend class for gradient descent weight optimizer model.
  friend class WeightOptimizerGradientDescent;

public:
  //! Assignment operator.
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
  //! Default constructor.
  WeightOptimizerAdam();

  //! Copy constructor.
  WeightOptimizerAdam( const WeightOptimizerAdam& ) = default;

  //! Assignment operator.
  WeightOptimizerAdam& operator=( const WeightOptimizerAdam& ) = delete;

  void get_status( DictionaryDatum& d ) const override;
  void set_status( const DictionaryDatum& d ) override;

private:
  double optimize_( const WeightOptimizerCommonProperties& cp, double weight, size_t current_opt_step ) override;

  //! First moment estimate variable.
  double m_;

  //! Second moment estimate variable.
  double v_;

  //! Power of beta_1 factor.
  double beta_1_power_;

  //! Power of beta_2 factor.
  double beta_2_power_;
};

/**
 * Class implementing common properties of an Adam weight optimizer model.
 */
class WeightOptimizerCommonPropertiesAdam : public WeightOptimizerCommonProperties
{
  //! Friend class for Adam weight optimizer model.
  friend class WeightOptimizerAdam;

public:
  //! Default constructor.
  WeightOptimizerCommonPropertiesAdam();

  //! Assignment operator.
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
  //! Exponential decay rate for first moment estimate.
  double beta_1_;

  //! Exponential decay rate for second moment estimate.
  double beta_2_;

  //! Small constant for numerical stability.
  double epsilon_;
};

} // namespace nest

#endif // WEIGHT_OPTIMIZER_H
