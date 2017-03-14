/*
 *  sinusoidal_gamma_generator.h
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

#ifndef SINUSOIDAL_GAMMA_GENERATOR_H
#define SINUSOIDAL_GAMMA_GENERATOR_H

// Generated includes:
#include "config.h"

#ifdef HAVE_GSL

// C++ includes:
#include <vector>

// Includes from librandom:
#include "randomgen.h"

// Includes from nestkernel:
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "node.h"
#include "stimulating_device.h"
#include "universal_data_logger.h"

namespace nest
{
/* BeginDocumentation
   Name: sinusoidal_gamma_generator - Generates sinusoidally modulated gamma
                                      spike trains.

   Description:
   sinusoidal_gamma_generator generates sinusoidally modulated gamma spike
   trains. By default, each target of the generator will receive a different
   spike train.

   The instantaneous rate of the process is given by

       f(t) = rate + amplitude sin ( 2 pi frequency t + phase * pi/180 )

   Parameters:
   The following parameters can be set in the status dictionary:

   rate       double - Mean firing rate in spikes/second, default: 0 s^-1
   amplitude  double - Firing rate modulation amplitude in spikes/second,
                       default: 0 s^-1
   frequency  double - Modulation frequency in Hz, default: 0 Hz
   phase      double - Modulation phase in degree [0-360], default: 0
   order      double - Gamma order (>= 1), default: 1

   individual_spike_trains   bool - See note below, default: true

   Remarks:
   - The gamma generator requires 0 <= amplitude <= rate.
   - The state of the generator is reset on calibration.
   - The generator does not support precise spike timing.
   - You can use the multimeter to sample the rate of the generator.
   - The generator will create different trains if run at different
     temporal resolutions.

   - Individual spike trains vs single spike train:
     By default, the generator sends a different spike train to each of its
     targets. If /individual_spike_trains is set to false using either
     SetDefaults or CopyModel before a generator node is created, the generator
     will send the same spike train to all of its targets.

   Receives: DataLoggingRequest

   Sends: SpikeEvent

   References: Barbieri et al, J Neurosci Methods 105:25-37 (2001)
   FirstVersion: October 2007, May 2013
   Author: Hans E Plesser, Thomas Heiberg

   SeeAlso: sinusoidal_poisson_generator, gamma_sup_generator
*/


/**
 * AC Gamma Generator.
 * Generates AC-modulated inhomogeneous gamma process.
 * @todo The implementation is very quick and dirty and not tuned for
 * performance at all.
 * @note  The simulator works by calculating the hazard h(t) for each time step
 * and comparing h(t) dt to a [0,1)-uniform number. The hazard is given by
 * $[
 *     h(t) = \frac{a \lambda(t) \Lambda(t)^{a-1} e^{-\Lambda(t)}}{\Gamma(a,
 *                                                                  \Lambda(t))}
 * $]
 * with
 * $[  \lambda(t) = dc + ac \sin ( 2 \pi f t + \phi ) $]
 * $[  \Lambda(t) = a \int_{t_0}^t \lambda(s) ds $]
 * and the incomplete Gamma function $\Gamma(a,z)$; $a$ is the order of the
 * gamma function and $t_0$ the time of the most recent spike.
 *
 * @note This implementation includes an additional $a$ factor in the
 * calculation of $\Lambda(t)$ and $h(t)$ in order to keep the mean rate
 * constant with varying $a$
 *
 * @note Let $t_0$ be the time of the most recent spike. If stimulus parameters
 * are changed at
 *       $t_c > t_0$, then $\Lambda(t)$ is integrated piecewise for $t>t_c$ as
 *       $[ \Lambda(t) = \a_{old} \int_{t_0}^{t_c]} \lambda_{old}(s) ds
 *                      + \a_{new} \int_{t_c}^{t]} \lambda_{new}(s) ds $]
 *       where "old" and "new" indicate old an new parameter values,
 *       respectively.
 *
 * @todo This implementation assumes that outgoing connections are all made from
 *       the same synapse type, see #737. Once #681 is fixed, we need to add a
         check that his assumption holds.
 */
class sinusoidal_gamma_generator : public Node
{

public:
  sinusoidal_gamma_generator();
  sinusoidal_gamma_generator( const sinusoidal_gamma_generator& );

  port send_test_event( Node&, rport, synindex, bool );

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;
  using Node::event_hook;

  void handle( DataLoggingRequest& );

  port handles_test_event( DataLoggingRequest&, rport );

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

  //! Model can be switched between proxies (single spike train) and not
  bool
  has_proxies() const
  {
    return not P_.individual_spike_trains_;
  }

  //! Allow multimeter to connect to local instances
  bool
  local_receiver() const
  {
    return true;
  }

private:
  void init_state_( const Node& );
  void init_buffers_();
  void calibrate();
  void event_hook( DSSpikeEvent& );

  void update( Time const&, const long, const long );

  struct Parameters_
  {
    /** Frequency in radian/ms */
    double om_;

    /** Phase in radian */
    double phi_;

    /** gamma order */
    double order_;

    /** Mean firing rate in spikes/ms */
    double rate_;

    /** Firing rate modulation amplitude in spikes/ms */
    double amplitude_;

    /** Emit individual spike trains for each target, or same for all? */
    bool individual_spike_trains_;

    /**
     * Number of targets.
     * This is a hidden parameter; must be placed in parameters,
     * even though it is an implementation detail, since it
     * concerns the connections and must not be affected by resets.
     * @note If individual_spike_trains_ is false, this value fixed at 1.
     *       This way all code using num_trains_ (and thus all the Buffers_
     *       arrays, does not need to check individual_spike_trains_.
     */
    size_t num_trains_;

    Parameters_(); //!< Sets default parameter values
    Parameters_( const Parameters_& );
    Parameters_& operator=( const Parameters_& p ); // Copy constructor EN

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary

    /**
     * Set values from dicitonary.
     * @note State is passed so that the position can be reset if the
     *       spike_times_ vector has been filled with new data.
     */
    void set( const DictionaryDatum&, const sinusoidal_gamma_generator& );
  };

  struct State_
  {

    double rate_; //!< current rate, kept for recording

    State_(); //!< Sets default state value

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
    //! Set values from dictionary
    void set( const DictionaryDatum&, const Parameters_& );
  };

  // ------------------------------------------------------------

  // These friend declarations must be precisely here.
  friend class RecordablesMap< sinusoidal_gamma_generator >;
  friend class UniversalDataLogger< sinusoidal_gamma_generator >;

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( sinusoidal_gamma_generator& );
    Buffers_( const Buffers_&, sinusoidal_gamma_generator& );
    UniversalDataLogger< sinusoidal_gamma_generator > logger_;

    /**
     * Beginning of current integration interval in ms.
     * This is either the most recent spike, or the most recent
     * parameter change, whichever is later. update() must integrate
     * Lambda from t0_ to the current time. The integral from the
     * most recent spike to t0_ is given as Lambda_t0_.
     * Entries are indexed by port, one per target.
     */
    std::vector< double > t0_ms_;

    /**
     * Integral Lambda from most recent spike up to t0_.
     * See t0_ for details.
     */
    std::vector< double > Lambda_t0_;

    Parameters_ P_prev_; //!< parameter values prior to last SetStatus
  };

  // ------------------------------------------------------------

  struct Variables_
  {
    double h_;    //!< time resolution (ms)
    double t_ms_; //!< current time in ms, for communication with event_hook()
    //! current time in steps, for communication with event_hook()
    long t_steps_;
    librandom::RngPtr rng_; //!< thread-specific random generator
  };

  double
  get_rate_() const
  {
    return 1000.0 * S_.rate_;
  }

  //! compute deltaLambda for given parameters from ta to tb
  double deltaLambda_( const Parameters_&, double, double ) const;

  //! compute hazard for given target index, including time-step factor
  double hazard_( port ) const;

  StimulatingDevice< SpikeEvent > device_;
  static RecordablesMap< sinusoidal_gamma_generator > recordablesMap_;

  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;
};

inline port
sinusoidal_gamma_generator::send_test_event( Node& target,
  rport receptor_type,
  synindex syn_id,
  bool dummy_target )
{
  device_.enforce_single_syn_type( syn_id );

  // to ensure correct overloading resolution, we need explicit event types
  // therefore, we need to duplicate the code here
  if ( P_.individual_spike_trains_ )
  {
    if ( dummy_target )
    {
      DSSpikeEvent e;
      e.set_sender( *this );
      return target.handles_test_event( e, receptor_type );
    }
    else
    {
      SpikeEvent e;
      e.set_sender( *this );
      const rport r = target.handles_test_event( e, receptor_type );
      if ( r != invalid_port_ and not is_model_prototype() )
      {
        ++P_.num_trains_;
      }
      return r;
    }
  }
  else
  {
    // We do not count targets here, since connections may be created through
    // proxies. Instead, we set P_.num_trains_ to 1 in Parameters_::set().
    SpikeEvent e;
    e.set_sender( *this );
    return target.handles_test_event( e, receptor_type );
  }
}

inline port
sinusoidal_gamma_generator::handles_test_event( DataLoggingRequest& dlr,
  rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
sinusoidal_gamma_generator::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
  device_.get_status( d );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
sinusoidal_gamma_generator::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors

  ptmp.set( d, *this ); // throws if BadProperty
  // We now know that ptmp is consistent. We do not write it back
  // to P_ before we are also sure that the properties to be set
  // in the parent class are internally consistent.
  device_.set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}

} // namespace

#endif // SINUSOIDAL_GAMMA_GENERATOR_H

#endif // HAVE_GSL
