/*
 *  eprop_archiving_node.h
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

#ifndef EPROP_ARCHIVING_NODE_H
#define EPROP_ARCHIVING_NODE_H

// nestkernel
#include "histentry.h"
#include "nest_time.h"
#include "nest_types.h"
#include "node.h"

// sli
#include "dictdatum.h"

namespace nest
{
/**
 * @brief Base class implementing archiving for node models supporting e-prop plasticity.
 *
 * Base class implementing an intermediate archiving node model for node models supporting e-prop plasticity
 * according to Bellec et al. (2020) and supporting additional biological features described in Korcsak-Gorzo,
 * Stapmanns, and Espinoza Valverde et al. (in preparation).
 *
 * A node which archives the history of dynamic variables, the firing rate
 * regularization, and update times needed to calculate the weight updates for
 * e-prop plasticity. It further provides a set of get, write, and set functions
 * for these histories and the hardcoded shifts to synchronize the factors of
 * the plasticity rule.
 *
 * @tparam HistEntryT The type of history entry.
 */
template < typename HistEntryT >
class EpropArchivingNode : public Node
{

public:
  /**
   * Constructs a new EpropArchivingNode object.
   */
  EpropArchivingNode();

  /**
   * Constructs a new EpropArchivingNode object by copying another EpropArchivingNode object.
   *
   * @param other The other object to copy.
   */
  EpropArchivingNode( const EpropArchivingNode& other );

  void register_eprop_connection( const bool is_bsshslm_2020_model = true ) override;

  void write_update_to_history( const long t_previous_update,
    const long t_current_update,
    const long eprop_isi_trace_cutoff = 0,
    const bool erase = false ) override;

  /**
   * Retrieves the update history entry for a specific time step.
   *
   * @param time_step The time step.
   * @return An iterator pointing to the update history for the specified time step.
   */
  std::vector< HistEntryEpropUpdate >::iterator get_update_history( const long time_step );

  /**
   * Retrieves the eprop history entry for a specified time step.
   *
   * @param time_step The time step.
   * @return An iterator pointing to the eprop history entry for the specified time step.
   */
  typename std::vector< HistEntryT >::iterator get_eprop_history( const long time_step );

  /**
   * @brief Erases the used eprop history for `bsshslm_2020` models.
   *
   * Erases e-prop history entries for update intervals during which no spikes were sent to the target neuron,
   * and any entries older than the earliest time stamp required by the first update in the history.
   */
  void erase_used_eprop_history();

  /**
   * @brief Erases the used eprop history.
   *
   * Erases e-prop history entries between the last and penultimate updates if they exceed the inter-spike
   * interval trace cutoff and any entries older than the earliest time stamp required by the first update.
   *
   * @param eprop_isi_trace_cutoff The cutoff value for the inter-spike integration of the eprop trace.
   */
  void erase_used_eprop_history( const long eprop_isi_trace_cutoff );

protected:
  //! Number of incoming eprop synapses
  size_t eprop_indegree_;

  //! History of updates still needed by at least one synapse.
  std::vector< HistEntryEpropUpdate > update_history_;

  //! History of dynamic variables needed for e-prop plasticity.
  std::vector< HistEntryT > eprop_history_;

  // The following shifts are, for now, hardcoded to 1 time step since the current
  // implementation only works if all the delays are equal to the simulation resolution.

  //! Offset since generator signals start from time step 1.
  const long offset_gen_ = 1;

  //! Transmission delay from input to recurrent neurons.
  const long delay_in_rec_ = 1;

  //! Transmission delay from recurrent to output neurons.
  const long delay_rec_out_ = 1;

  //! Transmission delay between output neurons for normalization.
  const long delay_out_norm_ = 1;

  //! Transmission delay from output neurons to recurrent neurons.
  const long delay_out_rec_ = 1;
};

/**
 * Class implementing an intermediate archiving node model for recurrent node models supporting e-prop plasticity.
 */
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
   * Selects a surrogate gradient function based on the specified name.
   *
   * @param surrogate_gradient_function_name The name of the surrogate gradient function.
   * @return The selected surrogate gradient function.
   */
  surrogate_gradient_function select_surrogate_gradient( const std::string& surrogate_gradient_function_name );

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
   * @param has_norm_step Flag indicating if an extra time step is used for communication between readout
   * neurons to normalize the readout signal outputs, as for softmax.
   */
  void write_learning_signal_to_history( const long time_step,
    const double learning_signal,
    const bool has_norm_step = true );

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
   * @param has_norm_step Flag indicating if an extra time step is used for communication between readout neurons to
   * normalize the readout signal outputs, as for softmax.
   *
   * @return The learning signal at the specified time step or zero if time step is not in the history.
   */
  double get_learning_signal_from_history( const long time_step, const bool has_norm_step = true );

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

inline void
EpropArchivingNodeRecurrent::count_spike()
{
  ++n_spikes_;
}

inline void
EpropArchivingNodeRecurrent::reset_spike_count()
{
  n_spikes_ = 0;
}

/**
 * Class implementing an intermediate archiving node model for readout node models supporting e-prop plasticity.
 */
class EpropArchivingNodeReadout : public EpropArchivingNode< HistEntryEpropReadout >
{
public:
  /**
   * Constructs a new EpropArchivingNodeReadout object.
   */
  EpropArchivingNodeReadout();

  /**
   * Constructs a new EpropArchivingNodeReadout object by copying another EpropArchivingNodeReadout object.
   *
   * @param other The EpropArchivingNodeReadout object to copy.
   */
  EpropArchivingNodeReadout( const EpropArchivingNodeReadout& other );

  /**
   * Creates an entry for the specified time step at the end of the eprop history.
   *
   * @param time_step The time step.
   * @param has_norm_step Flag indicating if an extra time step is used for communication between readout neurons to
   * normalize the readout signal outputs, as for softmax.
   */
  void append_new_eprop_history_entry( const long time_step, const bool has_norm_step = true );

  /**
   * Writes the error signal to the eprop history at the specified time step.
   *
   * @param time_step The time step.
   * @param error_signal The error signal.
   * @param has_norm_step Flag indicating if an extra time step is used for communication between readout neurons to
   * normalize the readout signal outputs, as for softmax.
   */
  void
  write_error_signal_to_history( const long time_step, const double error_signal, const bool has_norm_step = true );
};

} // namespace nest

#endif // EPROP_ARCHIVING_NODE_H
