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
/* BeginUserDocs:  e-prop plasticity

Short description
+++++++++++++++++
Archiving node for the history of e-prop variables.

Description
+++++++++++
The archiving node comprises a set of functions needed for writing the e-prop
values of the e-prop variables to history and retrieving them, as well as
functions to compute, for example, the firing rate regularization and the
surrogate gradient.

Surrogate gradient functions
++++++++++++++++++++++++++++

Surrogate gradients help overcome the challenge of the spiking function's
non-differentiability, facilitating the use of gradient-based learning
techniques such as e-prop. The non-existent derivative of the spiking
variable with respect to the membrane voltage,
:math:`\frac{\partial z^t_j}{ \partial v^t_j}`, can be effectively
replaced with a variety of surrogate gradient functions, as detailed in
various studies (see, e.g., [1]_). Here, there are four to
choose from:

1. A piecewise linear function used among others in [2]_:

.. math::
  \psi_j^t = \frac{ \gamma }{ v_\text{th} } \text{max}
    \left( 0, 1-\beta \left| \frac{ v_j^t - v_\text{th} }{ v_\text{th} }\right| \right) \,. \\

2. An exponential function used in [3]_:

.. math::
  \psi_j^t = \gamma \exp \left( -\beta \left| v_j^t - v_\text{th} \right| \right) \,. \\

3. The derivative of a fast sigmoid function used in [4]_:

.. math::
  \psi_j^t = \gamma \left( 1 + \beta \left| v_j^t - v_\text{th} \right| \right)^2 \,. \\

4. An arctan function used in [5]_:

.. math::
  \psi_j^t = \frac{\gamma}{\pi} \frac{1}{ 1 + \left( \beta \pi \left( v_j^t - v_\text{th} \right) \right)^2 } \,. \\


References
++++++++++

.. [1] Neftci EO, Mostafa H, Zenke F (2019). Surrogate Gradient Learning in
       Spiking Neural Networks. IEEE Signal Processing Magazine, 36(6), 51-63.
       https://doi.org/10.1109/MSP.2019.2931595

.. [2] Bellec G, Scherr F, Subramoney F, Hajek E, Salaj D, Legenstein R,
       Maass W (2020). A solution to the learning dilemma for recurrent
       networks of spiking neurons. Nature Communications, 11:3625.
       https://doi.org/10.1038/s41467-020-17236-y

.. [3] Shrestha SB, Orchard G (2018). SLAYER: Spike Layer Error Reassignment in
       Time. Advances in Neural Information Processing Systems, 31:1412-1421.
       https://proceedings.neurips.cc/paper_files/paper/2018/hash/82f2b308c3b01637c607ce05f52a2fed-Abstract.html

.. [4] Zenke F, Ganguli S (2018). SuperSpike: Supervised Learning in Multilayer
       Spiking Neural Networks. Neural Computation, 30:1514–1541.
       https://doi.org/10.1162/neco_a_01086

.. [5] Fang W, Yu Z, Chen Y, Huang T, Masquelier T, Tian Y (2021). Deep residual
       learning in spiking neural networks. Advances in Neural Information
       Processing Systems, 34:21056–21069.
       https://proceedings.neurips.cc/paper/2021/hash/afe434653a898da20044041262b3ac74-Abstract.html

EndUserDocs */

/**
 * Base class implementing an intermediate archiving node model for node models supporting e-prop plasticity
 * according to Bellec et al. (2020) and supporting additional biological features described in Korcsak-Gorzo,
 * Stapmanns, and Espinoza Valverde et al. (in preparation).
 *
 * A node which archives the history of dynamic variables, the firing rate
 * regularization, and update times needed to calculate the weight updates for
 * e-prop plasticity. It further provides a set of get, write, and set functions
 * for these histories and the hardcoded shifts to synchronize the factors of
 * the plasticity rule.
 */
template < typename HistEntryT >
class EpropArchivingNode : public Node
{

public:
  //! Default constructor.
  EpropArchivingNode();

  //! Copy constructor.
  EpropArchivingNode( const EpropArchivingNode& );

  //! Initialize the update history and register the eprop synapse.
  void register_eprop_connection( const bool is_bsshslm_2020_model = true ) override;

  //! Register current update in the update history and deregister previous update.
  void write_update_to_history( const long t_previous_update,
    const long t_current_update,
    const long eprop_isi_trace_cutoff = 0,
    const bool erase = false ) override;

  //! Get an iterator pointing to the update history entry of the given time step.
  std::vector< HistEntryEpropUpdate >::iterator get_update_history( const long time_step );

  //! Get an iterator pointing to the eprop history entry of the given time step.
  typename std::vector< HistEntryT >::iterator get_eprop_history( const long time_step );

  //! Erase update history parts for which the access counter has decreased to zero since no synapse needs them
  //! any longer.
  void erase_used_update_history();

  //! Erase update intervals from the e-prop history in which each synapse has either not transmitted a spike or has
  //! transmitted a spike in a more recent update interval.
  void erase_used_eprop_history();

  //! Erase entries from the e-prop history between the last trace cutoff and the last update, as well as the ones up to
  //! the first update.
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

  //! Connection delay from input to recurrent neurons.
  const long delay_in_rec_ = 1;

  //! Connection delay from recurrent to output neurons.
  const long delay_rec_out_ = 1;

  //! Connection delay between output neurons for normalization.
  const long delay_out_norm_ = 1;

  //! Connection delay from output neurons to recurrent neurons.
  const long delay_out_rec_ = 1;
};

/**
 * Class implementing an intermediate archiving node model for recurrent node models supporting e-prop plasticity.
 */
class EpropArchivingNodeRecurrent : public EpropArchivingNode< HistEntryEpropRecurrent >
{

public:
  //! Default constructor.
  EpropArchivingNodeRecurrent();

  //! Copy constructor.
  EpropArchivingNodeRecurrent( const EpropArchivingNodeRecurrent& );

  //! Select the surrogate gradient function.
  double ( EpropArchivingNodeRecurrent::*select_surrogate_gradient(
    std::string surrogate_gradient_function ) )( double, double, double, double, double, double )
  {
    const std::map< std::string,
      double ( EpropArchivingNodeRecurrent::* )( double, double, double, double, double, double ) >
      surrogate_gradient_funcs = { { "piecewise_linear",
                                     &EpropArchivingNodeRecurrent::compute_piecewise_linear_surrogate_gradient },
        { "exponential", &EpropArchivingNodeRecurrent::compute_exponential_surrogate_gradient },
        { "fast_sigmoid_derivative", &EpropArchivingNodeRecurrent::compute_fast_sigmoid_derivative_surrogate_gradient },
        { "arctan", &EpropArchivingNodeRecurrent::compute_arctan_surrogate_gradient } };

    auto found_entry_it = surrogate_gradient_funcs.find( surrogate_gradient_function );
    if ( found_entry_it == surrogate_gradient_funcs.end() )
    {
      std::string error_message = "Surrogate gradient / pseudo-derivate function surrogate_gradient_function from [";
      for ( const auto& surrogate_gradient_func : surrogate_gradient_funcs )
      {
        error_message += " \"" + surrogate_gradient_func.first + "\",";
      }
      error_message.pop_back();
      error_message += " ] required.";

      throw BadProperty( error_message );
    }
    else
    {
      return found_entry_it->second;
    }
  }

  //! Compute the surrogate gradient with a piecewise linear function around the spike time (used, e.g., in Bellec et
  //! al., 2020).
  double compute_piecewise_linear_surrogate_gradient( const double r,
    const double v_m,
    const double v_th_adapt,
    const double V_th,
    const double beta,
    const double gamma );

  //! Compute the surrogate gradient with an exponentially decaying function around the spike time (used, e.g., in
  //! Shrestha and Orchard, 2018).
  double compute_exponential_surrogate_gradient( const double r,
    const double v_m,
    const double v_th_adapt,
    const double V_th,
    const double beta,
    const double gamma );

  //! Compute the surrogate gradient with a function corresponding to the derivative of a fast sigmoid around the spike
  //! (used, e.g., in Zenke and Ganguli, 2018).
  double compute_fast_sigmoid_derivative_surrogate_gradient( const double r,
    const double v_m,
    const double v_th_adapt,
    const double V_th,
    const double beta,
    const double gamma );

  //! Compute the surrogate gradient with an arctan function around the spike time (used, e.g., in Fang et al., 2021).
  double compute_arctan_surrogate_gradient( const double r,
    const double v_m,
    const double v_th_adapt,
    const double V_th,
    const double beta,
    const double gamma );

  //! Create an entry for the given time step at the end of the eprop history.
  void emplace_new_eprop_history_entry( const long time_step );

  //! Write the given surrogate gradient value to the history at the given time step.
  void write_surrogate_gradient_to_history( const long time_step, const double surrogate_gradient );

  //! Update the learning signal in the eprop history entry of the given time step by writing the value of the incoming
  //! learning signal to the history or adding it to the existing value in case of multiple readout neurons.
  void write_learning_signal_to_history( const long time_step,
    const double learning_signal,
    const bool has_norm_step = true );

  //! Create an entry in the firing rate regularization history for the current update.
  void write_firing_rate_reg_to_history( const long t_current_update, const double f_target, const double c_reg );

  //! Calculate the current firing rate regularization and add the value to the learning signal.
  void write_firing_rate_reg_to_history( const long t,
    const long interval_step,
    const double z,
    const double f_target,
    const double kappa,
    const double c_reg );

  //! Get an iterator pointing to the firing rate regularization history of the given time step.
  std::vector< HistEntryEpropFiringRateReg >::iterator get_firing_rate_reg_history( const long time_step );

  //! Return learning signal from history for given time step or zero if time step not in history
  double get_learning_signal_from_history( const long time_step, const bool has_norm_step = true );

  //! Erase parts of the firing rate regularization history for which the access counter in the update history has
  //! decreased to zero since no synapse needs them any longer.
  void erase_used_firing_rate_reg_history();

  //! Count emitted spike for the firing rate regularization.
  void count_spike();

  //! Reset spike count for the firing rate regularization.
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
  //! Default constructor.
  EpropArchivingNodeReadout();

  //! Copy constructor.
  EpropArchivingNodeReadout( const EpropArchivingNodeReadout& );

  //! Create an entry for the given time step at the end of the eprop history.
  void emplace_new_eprop_history_entry( const long time_step, const bool has_norm_step = true );

  //! Write the given error signal value to history at the given time step.
  void
  write_error_signal_to_history( const long time_step, const double error_signal, const bool has_norm_step = true );
};

} // namespace nest

#endif // EPROP_ARCHIVING_NODE_H
