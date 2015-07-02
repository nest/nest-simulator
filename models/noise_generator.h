/*
 *  noise_generator.h
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

#ifndef NOISE_GENERATOR_H
#define NOISE_GENERATOR_H


#include <vector>
#include "nest.h"
#include "event.h"
#include "node.h"
#include "stimulating_device.h"
#include "connection.h"
#include "normal_randomdev.h"

namespace nest
{

/* BeginDocumentation
Name: noise_generator - Device to generate Gaussian white noise current.
Description:
This device can be used to inject a Gaussian "white" noise current into a node.
The current is not really white, but a piecewise constant current with Gaussian
distributed amplitude. The current changes at intervals of dt. dt must be a
multiple of the simulation step size, the default is 1.0ms,
corresponding to a 1kHz cut-off.
Additionally a second sinusodial modulated term can be added to the standard deviation of the noise.

The current generated is given by

  I(t) = mean + std * N_j  for t_0 + j dt <= t < t_0 + (j-1) dt

where N_j are Gaussian random numbers with unit standard deviation and t_0 is
the device onset time.
If the modulation is added the current is given by

  I(t) = mean + sqrt(std^2 + std_mod^2 * sin(omega * t + phase)) * N_j  for t_0 + j dt <= t < t_0 +
(j-1) dt

For a detailed discussion of the properties of the noise generator, please see
the noise_generator.ipynb notebook included in the NEST source code (docs/model_details).

Parameters:
The following parameters can be set in the status dictionary:

mean      double - mean value of the noise current in pA
std       double - standard deviation of noise current in pA
dt        double - interval between changes in current in ms, default 1.0ms
std_mod   double - modulated standard deviation of noise current in pA
phase     double - Phase of sine modulation (0-360 deg)
frequency double - Frequency of sine modulation in Hz

Remarks:
 - All targets receive different currents.
 - The currents for all targets change at the same points in time.
 - The interval between changes, dt, must be a multiple of the time step.
 - The effect of this noise current on a neuron DEPENDS ON DT. Consider
   the membrane potential fluctuations evoked when a noise current is
   injected into a neuron. The standard deviation of these fluctuations
   across an ensemble will increase with dt for a given value of std.
   For the leaky integrate-and-fire neuron with time constant tau_m and
   capacity C_m, membrane potential fluctuations Sigma at times t_j+delay are given by

     Sigma = std * tau_m / C_m * sqrt( (1-x) / (1+x) ) where x = exp(-dt/tau_m)

   for large t_j. In the white noise limit, dt -> 0, one has

     Sigma -> std / C_m * sqrt(dt * tau / 2).

   To obtain comparable results for different values of dt, you must
   adapt std.

Sends: CurrentEvent

SeeAlso: Device

Author: Ported to NEST2 API 08/2007 by Jochen Eppler, updated 07/2008 by HEP
*/

/**
 * Gaussian white noise generator.
 * Provide Gaussian "white" noise input current
 */
class noise_generator : public Node
{

public:
  noise_generator();
  noise_generator( const noise_generator& );

  bool
  has_proxies() const
  {
    return false;
  }

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and Hiding
   */
  using Node::event_hook;

  port send_test_event( Node&, rport, synindex, bool );

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  void init_state_( const Node& );
  void init_buffers_();

  /**
   * Recalculates parameters and forces reinitialization
   * of amplitudes if number of targets has changed.
   */
  void calibrate();

  void update( Time const&, const long_t, const long_t );
  void event_hook( DSCurrentEvent& );

  // ------------------------------------------------------------

  typedef std::vector< double_t > AmpVec_;

  /**
   * Store independent parameters of the model.
   */
  struct Parameters_
  {
    double_t mean_;    //!< mean current, in pA
    double_t std_;     //!< standard deviation of current, in pA
    double_t std_mod_; //!< standard deviation of current modulation, in pA
    double_t freq_;    //!< Standard frequency in Hz
    double_t phi_deg_; //!< Phase of sinusodial noise modulation (0-360 deg)
    Time dt_;          //!< time interval between updates

    /**
     * Number of targets.
     * This is a hidden parameter; must be placed in parameters,
     * even though it is an implementation detail, since it
     * concerns the connections and must not be affected by resets.
     */
    size_t num_targets_;

    Parameters_(); //!< Sets default parameter values
    Parameters_( const Parameters_& );

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
    void set( const DictionaryDatum&, const noise_generator& ); //!< Set values from dicitonary
  };

  // ------------------------------------------------------------

  struct State_
  {
    double_t y_0_;
    double_t y_1_;

    State_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
  };

  // ------------------------------------------------------------

  struct Buffers_
  {
    long_t next_step_; //!< time step of next change in current
    AmpVec_ amps_;     //!< amplitudes, one per target
  };

  // ------------------------------------------------------------

  struct Variables_
  {
    long_t dt_steps_;                       //!< update interval in steps
    librandom::NormalRandomDev normal_dev_; //!< random deviate generator
    double_t omega_;                        //!< Angelfrequency i rad/s
    double_t phi_rad_;                      //!< Phase of sine current (0-2Pi rad)

    // The exact integration matrix
    double_t A_00_;
    double_t A_01_;
    double_t A_10_;
    double_t A_11_;
  };

  // ------------------------------------------------------------

  StimulatingDevice< CurrentEvent > device_;
  Parameters_ P_;
  Variables_ V_;
  Buffers_ B_;
  State_ S_;
};


inline void
noise_generator::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
  device_.get_status( d );
}

inline void
noise_generator::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;               // temporary copy in case of errors
  ptmp.num_targets_ = P_.num_targets_; // Copy Constr. does not copy connections
  ptmp.set( d, *this );                // throws if BadProperty

  // We now know that ptmp is consistent. We do not write it back
  // to P_ before we are also sure that the properties to be set
  // in the parent class are internally consistent.
  device_.set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  P_.num_targets_ = ptmp.num_targets_;
}
}
#endif // NOISE_GENERATOR_H
