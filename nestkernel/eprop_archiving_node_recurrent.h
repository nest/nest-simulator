/*
 *  eprop_archiving_node_recurrent.h
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

#ifndef EPROP_ARCHIVING_NODE_RECURRENT_H
#define EPROP_ARCHIVING_NODE_RECURRENT_H

// models
#include "eprop_archiving_node_impl.h"

// nestkernel
#include "histentry.h"

// sli
#include "dictdatum.h"

namespace nest
{

/**
 * Class implementing an intermediate archiving node model for recurrent node models supporting e-prop plasticity.
 */
template < bool hist_shift_required >
class EpropArchivingNodeRecurrent : public EpropArchivingNode< HistEntryEpropRecurrent >
{

public:
  /**
   * Constructs a new EpropArchivingNodeRecurrent object.
   */
  EpropArchivingNodeRecurrent();

  /**
   * Constructs an EpropArchivingNodeRecurrent object by copying another EpropArchivingNodeRecurrent object.
   *
   * @param other The EpropArchivingNodeRecurrent object to copy.
   */
  EpropArchivingNodeRecurrent( const EpropArchivingNodeRecurrent& other );

  /**
   * Defines the pointer-to-member function type for the surrogate gradient function.
   *
   * @note The typename is `surrogate_gradient_function`. All parentheses in the expression are required.
   */
  typedef double (
    EpropArchivingNodeRecurrent::*surrogate_gradient_function )( double, double, double, double, double );

  /**
   * Validates and finds surrogate gradient function based on the specified name.
   *
   * @param surrogate_gradient_function_name The name of the surrogate gradient function.
   * @return The selected surrogate gradient function.
   */
  surrogate_gradient_function find_surrogate_gradient( const std::string& surrogate_gradient_function_name );

  /**
   * @brief Computes the surrogate gradient with a piecewise linear function around the spike time.
   *
   * The piecewise linear surrogate function is used, for example, in Bellec et al. (2020).
   *
   * @param r The number of remaining refractory steps.
   * @param v_m The membrane voltage.
   * @param v_th The spike threshold voltage. For adaptive neurons, the adaptive spike threshold voltage.
   * @param beta The width scaling of the surrogate gradient function.
   * @param gamma The height scaling of the surrogate gradient function.
   * @return The surrogate gradient of the membrane voltage.
   */
  double compute_piecewise_linear_surrogate_gradient( const double r,
    const double v_m,
    const double v_th,
    const double beta,
    const double gamma );

  /**
   * @brief Computes the surrogate gradient with an exponentially decaying function around the spike time.
   *
   * The exponential surrogate function is used, for example, in Shrestha and Orchard (2018).
   *
   * @param r The number of remaining refractory steps.
   * @param v_m The membrane voltage.
   * @param v_th The threshold membrane voltage. For adaptive neurons, this is the adaptive threshold.
   * @param v_th The spike threshold voltage. For adaptive neurons, the adaptive spike threshold voltage.
   * @param beta The width scaling of the surrogate gradient function.
   * @param gamma The height scaling of the surrogate gradient function.
   *
   * @return The surrogate gradient of the membrane voltage.
   */
  double compute_exponential_surrogate_gradient( const double r,
    const double v_m,
    const double v_th,
    const double beta,
    const double gamma );

  /**
   * @brief Computes the surrogate gradient with a function reflecting the derivative of a fast sigmoid around the spike
   * time.
   *
   * The derivative of fast sigmoid surrogate function is used, for example, in Zenke and Ganguli (2018).
   *
   * @param r The number of remaining refractory steps.
   * @param v_m The membrane voltage.
   * @param v_th The spike threshold voltage. For adaptive neurons, the adaptive spike threshold voltage.
   * @param beta The width scaling of the surrogate gradient function.
   * @param gamma The height scaling of the surrogate gradient function.
   *
   * @return The surrogate gradient of the membrane voltage.
   */
  double compute_fast_sigmoid_derivative_surrogate_gradient( const double r,
    const double v_m,
    const double v_th,
    const double beta,
    const double gamma );

  /**
   * @brief Computes the surrogate gradient with an inverse tangent function around the spike time.
   *
   * The inverse tangent surrogate gradient function is used, for example, in Fang et al. (2021).
   *
   * @param r The number of remaining refractory steps.
   * @param v_m The membrane voltage.
   * @param v_th The spike threshold voltage. For adaptive neurons, the adaptive spike threshold voltage.
   * @param beta The width scaling of the surrogate gradient function.
   * @param gamma The height scaling of the surrogate gradient function.
   *
   * @return The surrogate gradient of the membrane voltage.
   */
  double compute_arctan_surrogate_gradient( const double r,
    const double v_m,
    const double v_th,
    const double beta,
    const double gamma );

  /**
   * Creates an entry for the specified time step at the end of the eprop history.
   *
   * @param time_step The time step.
   */
  void append_new_eprop_history_entry( const long time_step );

  /**
   * Writes the surrogate gradient to the eprop history entry at the specified time step.
   *
   * @param time_step The time step.
   * @param surrogate_gradient The surrogate gradient.
   */
  void write_surrogate_gradient_to_history( const long time_step, const double surrogate_gradient );

  /**
   * @brief Writes the learning signal to the eprop history entry at the specifed time step.
   *
   * Updates the learning signal in the eprop history entry of the specified time step by writing the value of the
   * incoming learning signal to the history or adding it to the existing value in case of multiple readout
   * neurons.
   *
   * @param time_step The time step.
   * @param learning_signal The learning signal.
   */
  void write_learning_signal_to_history( const long time_step, const double learning_signal );

  /**
   * Calculates the firing rate regularization for the current update and writes it to a new entry in the firing rate
   * regularization history.
   *
   * @param t_current_update The current update time.
   * @param f_target The target firing rate.
   * @param c_reg The firing rate regularization coefficient.
   */
  void write_firing_rate_reg_to_history( const long t_current_update, const double f_target, const double c_reg );

  /**
   * Calculates the current firing rate regularization and writes it to the eprop history at the specified time step.
   *
   * @param time_step The time step.
   * @param z The spike state variable.
   * @param f_target The target firing rate.
   * @param kappa_reg The low-pass filter of the firing rate regularization.
   * @param c_reg The firing rate regularization coefficient.
   */
  void write_firing_rate_reg_to_history( const long time_step,
    const double z,
    const double f_target,
    const double kappa_reg,
    const double c_reg );

  /**
   * Retrieves the firing rate regularization at the specified time step from the firing rate regularization history.
   *
   * @param time_step The time step.
   *
   * @return The firing rate regularization at the specified time step.
   */
  double get_firing_rate_reg_history( const long time_step );

  /**
   * Retrieves the learning signal from the eprop history at the specified time step.
   *
   * @param time_step The time step.
   *
   * @return The learning signal at the specified time step or zero if time step is not in the history.
   */
  double get_learning_signal_from_history( const long time_step );

  /**
   * @brief Erases the history of the used firing rate regularization history.
   *
   * Erases parts of the firing rate regularization history for which the access counter in the update history has
   * decreased to zero since no synapse needs them any longer.
   */
  void erase_used_firing_rate_reg_history();

  /**
   * Counts an emitted spike for the firing rate regularization.
   */
  void count_spike();

  /**
   * Resets the spike count for the firing rate regularization.
   */
  void reset_spike_count();

  //! Firing rate regularization.
  double firing_rate_reg_;

  //! Average firing rate.
  double f_av_;

protected:
  long model_dependent_history_shift_() const override;
  bool history_shift_required_() const override;

  //! Pointer to member function selected for computing the surrogate gradient.
  surrogate_gradient_function compute_surrogate_gradient_ =
    &EpropArchivingNodeRecurrent::compute_piecewise_linear_surrogate_gradient;

private:
  //! Count of the emitted spikes for the firing rate regularization.
  size_t n_spikes_;

  //! History of the firing rate regularization.
  std::vector< HistEntryEpropFiringRateReg > firing_rate_reg_history_;

  /**
   * Maps provided names of surrogate gradients to corresponding pointers to member functions.
   *
   * @todo In the long run, this map should be handled by a manager with proper registration functions,
   * so that external modules can add their own gradient functions.
   */
  static std::map< std::string, surrogate_gradient_function > surrogate_gradient_funcs_;
};

}

#endif
