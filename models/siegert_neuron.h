/*
 *  siegert_neuron.h
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

#ifndef SIEGERT_NEURON_H
#define SIEGERT_NEURON_H

#include "config.h"

#ifdef HAVE_GSL

// C includes:
#include <gsl/gsl_integration.h>

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "node.h"
#include "normal_randomdev.h"
#include "poisson_randomdev.h"
#include "ring_buffer.h"
#include "recordables_map.h"
#include "universal_data_logger.h"

namespace nest
{

/** @BeginDocumentation
Name: siegert_neuron

Description:

siegert_neuron is an implementation of a rate model with the
non-linearity given by the gain function of the
leaky-integrate-and-fire neuron with delta or exponentially decaying
synapses [2] and [3, their eq. 25]. The model can be used for a
mean-field analysis of spiking networks.

The model supports connections to other rate models with zero
delay, and uses the secondary_event concept introduced with the
gap-junction framework.

Parameters:

The following parameters can be set in the status dictionary.

rate                double - Rate (1/s)
tau                 double - Time constant in ms.
mean                double - Additional constant input

The following parameters can be set in the status directory and are
used in the evaluation of the gain function. Parameters as in
iaf_psc_exp/delta.

tau_m               double - Membrane time constant in ms.
tau_syn             double - Time constant of postsynaptic currents in ms.
t_ref               double - Duration of refractory period in ms.
theta               double - Threshold relative to resting potential in mV.
V_reset             double - Reset relative to resting membrane potential in
                             mV.

References:

[1] Hahne, J., Dahmen, D., Schuecker, J., Frommer, A.,
    Bolten, M., Helias, M. and Diesmann, M. (2017).
    Integration of Continuous-Time Dynamics in a
    Spiking Neural Network Simulator.
    Front. Neuroinform. 11:34. doi: 10.3389/fninf.2017.00034

[2] Fourcaud, N and Brunel, N. (2002). Dynamics of the firing
    probability of noisy integrate-and-fire neurons, Neural computation,
    14:9, pp 2057--2110

[3] Schuecker, J., Diesmann, M. and Helias, M. (2015).
    Modulated escape from a metastable state driven by colored noise.
    Physical Review E 92:052119

[4] Hahne, J., Helias, M., Kunkel, S., Igarashi, J.,
    Bolten, M., Frommer, A. and Diesmann, M. (2015).
    A unified framework for spiking and gap-junction interactions
    in distributed neuronal network simulations.
    Front. Neuroinform. 9:22. doi: 10.3389/fninf.2015.00022

Sends: DiffusionConnectionEvent

Receives: DiffusionConnectionEvent, DataLoggingRequest

Author: Jannis Schuecker, David Dahmen, Jan Hahne

SeeAlso: diffusion_connection
*/
class siegert_neuron : public Archiving_Node
{

public:
  typedef Node base;

  siegert_neuron();
  siegert_neuron( const siegert_neuron& );

  ~siegert_neuron();

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::sends_secondary_event;

  void handle( DiffusionConnectionEvent& );
  void handle( DataLoggingRequest& );

  port handles_test_event( DiffusionConnectionEvent&, rport );
  port handles_test_event( DataLoggingRequest&, rport );

  void
  sends_secondary_event( DiffusionConnectionEvent& )
  {
  }

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  void init_state_( const Node& proto );
  void init_buffers_();
  void calibrate();

  /** This is the actual update function. The additional boolean parameter
   * determines if the function is called by update (false) or wfr_update (true)
   */
  bool update_( Time const&, const long, const long, const bool );

  void update( Time const&, const long, const long );
  bool wfr_update( Time const&, const long, const long );

  // siegert functions and helpers
  double siegert( double, double );
  double siegert1( double, double, double, double );
  double siegert2( double, double, double, double );

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< siegert_neuron >;
  friend class UniversalDataLogger< siegert_neuron >;

  // ----------------------------------------------------------------

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {
    /** Time constant in ms. */
    double tau_;

    /** Membrane time constant in ms. */
    double tau_m_;

    /** Synaptic time constant in ms. */
    double tau_syn_;

    /** Refractory period in ms. */
    double t_ref_;

    /** Constant input in Hz. */
    double mean_;

    /** Threshold in mV. */
    double theta_;

    /** reset value in mV. */
    double V_reset_;

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary

    void set( const DictionaryDatum& );
  };

  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   */
  struct State_
  {
    double r_; //!< Rate

    State_(); //!< Default initialization

    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum& );
  };

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( siegert_neuron& );
    Buffers_( const Buffers_&, siegert_neuron& );

    std::vector< double >
      drift_input_; //!< buffer for drift term received by DiffusionConnection
    std::vector< double > diffusion_input_; //!< buffer for diffusion term
    // received by DiffusionConnection
    std::vector< double >
      last_y_values; //!< remembers y_values from last wfr_update
    UniversalDataLogger< siegert_neuron >
      logger_; //!< Logger for all analog data
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {

    // propagators
    double P1_;
    double P2_;
  };

  //! Read out the rate
  double
  get_rate_() const
  {
    return S_.r_;
  }

  // ----------------------------------------------------------------

  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;

  gsl_integration_workspace* gsl_w_;

  //! Mapping of recordables names to access functions
  static RecordablesMap< siegert_neuron > recordablesMap_;
};

inline void
siegert_neuron::update( Time const& origin, const long from, const long to )
{
  update_( origin, from, to, false );
}

inline bool
siegert_neuron::wfr_update( Time const& origin, const long from, const long to )
{
  State_ old_state = S_; // save state before wfr update
  const bool wfr_tol_exceeded = update_( origin, from, to, true );
  S_ = old_state; // restore old state

  return not wfr_tol_exceeded;
}

inline port
siegert_neuron::handles_test_event( DiffusionConnectionEvent&,
  rport receptor_type )
{
  if ( receptor_type == 0 )
  {
    return 0;
  }
  else if ( receptor_type == 1 )
  {
    return 1;
  }
  else
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
}

inline port
siegert_neuron::handles_test_event( DataLoggingRequest& dlr,
  rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
siegert_neuron::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
  Archiving_Node::get_status( d );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
siegert_neuron::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d );         // throws if BadProperty
  State_ stmp = S_;      // temporary copy in case of errors
  stmp.set( d );         // throws if BadProperty

  // We now know that (ptmp, stmp) are consistent. We do not
  // write them back to (P_, S_) before we are also sure that
  // the properties to be set in the parent class are internally
  // consistent.
  Archiving_Node::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

} // namespace

#endif // HAVE_GSL
#endif // SIEGERT_NEURON_H
