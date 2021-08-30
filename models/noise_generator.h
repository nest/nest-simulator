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

// C++ includes:
#include <vector>

// Includes from nestkernel:
#include "connection.h"
#include "device_node.h"
#include "event.h"
#include "nest_timeconverter.h"
#include "nest_types.h"
#include "random_generators.h"
#include "stimulation_device.h"
#include "universal_data_logger.h"

namespace nest
{

/* BeginUserDocs: device, generator

Short description
+++++++++++++++++

Generate a Gaussian white noise current

Description
+++++++++++

This device can be used to inject a Gaussian "white" noise current into a node.

The current is not really white, but a piecewise constant current with Gaussian
distributed amplitude. The current changes at intervals of dt. dt must be a
multiple of the simulation step size, the default is 1.0 ms,
corresponding to a 1 kHz cut-off.
Additionally a second sinusodial modulated term can be added to the standard
deviation of the noise.

The current generated is given by

.. math::

  I(t) = mean + std * N_j  \text{ for } t_0 + j dt \leq t < t_0 + (j-1) dt

where :math:`N_j` are Gaussian random numbers with unit standard deviation and
:math:`t_0` is the device onset time.
If the modulation is added the current is given by

.. math::

   I(t) = mean + \sqrt(std^2 + std_{mod}^2 * \sin(\omega * t + phase)) * N_j \\
                              \text{ for } t_0 + j dt \leq t < t_0 + (j-1) dt

For a detailed discussion of the properties of the noise generator, please see
`noise_generator <../model_details/noise_generator.ipynb>`_
notebook included in the NEST source code.

Remarks
+++++++

 - All targets receive different currents.

 - The currents for all targets change at the same points in time.

 - The interval between changes, dt, must be a multiple of the time step.

 - The effect of this noise current on a neuron depends on dt. Consider
   the membrane potential fluctuations evoked when a noise current is
   injected into a neuron. The standard deviation of these fluctuations
   across an ensemble will increase with dt for a given value of std.
   For the leaky integrate-and-fire neuron with time constant :math:`\tau_m` and
   capacity :math:`C_m`, membrane potential fluctuations Sigma at time
   :math:`t_j+delay` are given by

   .. math::

      \Sigma = std * \tau_m / C_m * \sqrt( (1-x) / (1+x) )  \\
                             \text{where } x = exp(-dt/\tau_m)

   for large :math:`t_j`. In the white noise limit, :math:`dt \rightarrow 0`, one has

   .. math::

      \Sigma \rightarrow std / C_m * \sqrt(dt * \tau / 2).

To obtain comparable results for different values of dt, you must adapt std.

As the noise generator provides a different current for each of its targets,
the current recorded represents the instantaneous average of all the
currents computed. When there exists only a single target, this would be
equivalent to the actual current provided to that target.

.. include:: ../models/stimulation_device.rst

mean
    The mean value of the noise current (pA)

std
    The standard deviation of noise current (pA)

dt
    The interval between changes in current in ms (default: 1.0)

std_mod
    The modulated standard deviation of noise current (pA)

phase
    The phase of sine modulation (0-360 deg)

frequency
    The frequency of the sine modulation

Set parameters from a stimulation backend
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The parameters in this stimulation device can be updated with input
coming from a stimulation backend. The data structure used for the
update holds one value for each of the parameters mentioned above.
The indexing is as follows:

 0. mean
 1. std
 2. std_mod
 3. frequency
 4. phase

Sends
+++++

CurrentEvent

See also
++++++++

step_current_generator

EndUserDocs */

class noise_generator : public StimulationDevice
{

public:
  noise_generator();
  noise_generator( const noise_generator& );


  //! Allow multimeter to connect to local instances
  bool local_receiver() const override;

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;
  using Node::event_hook;
  using Node::sends_signal;

  port send_test_event( Node&, rport, synindex, bool ) override;

  SignalType sends_signal() const override;

  void handle( DataLoggingRequest& ) override;

  port handles_test_event( DataLoggingRequest&, rport ) override;

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

  void calibrate_time( const TimeConverter& tc ) override;

  StimulationDevice::Type get_type() const override;
  void set_data_from_stimulation_backend( std::vector< double >& input_param ) override;

private:
  void init_state_() override;
  void init_buffers_() override;

  /**
   * Recalculates parameters and forces reinitialization
   * of amplitudes if number of targets has changed.
   */
  void calibrate() override;

  void update( Time const&, const long, const long ) override;
  void event_hook( DSCurrentEvent& ) override;

  // ------------------------------------------------------------

  typedef std::vector< double > AmpVec_;

  /**
   * Store independent parameters of the model.
   */
  struct Parameters_
  {
    double mean_;    //!< mean current, in pA
    double std_;     //!< standard deviation of current, in pA
    double std_mod_; //!< standard deviation of current modulation, in pA
    double freq_;    //!< Standard frequency in Hz
    double phi_deg_; //!< Phase of sinusodial noise modulation (0-360 deg)
    Time dt_;        //!< time interval between updates

    /**
     * Number of targets.
     * This is a hidden parameter; must be placed in parameters,
     * even though it is an implementation detail, since it
     * concerns the connections and must not be affected by resets.
     */
    size_t num_targets_;

    Parameters_(); //!< Sets default parameter values
    Parameters_( const Parameters_& );
    Parameters_& operator=( const Parameters_& p );

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
    //! Set values from dictionary
    void set( const DictionaryDatum&, const noise_generator&, Node* node );
  };

  // ------------------------------------------------------------

  struct State_
  {
    double y_0_;
    double y_1_;
    double I_avg_; //!< Average of instantaneous currents computed
                   //!< Used for recording current

    State_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
  };

  // ------------------------------------------------------------

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< noise_generator >;
  friend class UniversalDataLogger< noise_generator >;

  // ------------------------------------------------------------

  struct Buffers_
  {
    long next_step_; //!< time step of next change in current
    AmpVec_ amps_;   //!< amplitudes, one per target
    explicit Buffers_( noise_generator& );
    Buffers_( const Buffers_&, noise_generator& );
    UniversalDataLogger< noise_generator > logger_;
  };

  // ------------------------------------------------------------

  struct Variables_
  {
    normal_distribution normal_dist_; //!< normal distribution

    long dt_steps_;  //!< update interval in steps
    double omega_;   //!< frequency [radian/s]
    double phi_rad_; //!< phase of sine current (0-2Pi rad)

    // The exact integration matrix
    double A_00_;
    double A_01_;
    double A_10_;
    double A_11_;
  };

  double
  get_I_avg_() const
  {
    return S_.I_avg_;
  }

  // ------------------------------------------------------------

  static RecordablesMap< noise_generator > recordablesMap_;

  Parameters_ P_;
  State_ S_;
  Buffers_ B_;
  Variables_ V_;
};

inline port
noise_generator::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
noise_generator::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
  StimulationDevice::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
noise_generator::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;               // temporary copy in case of errors
  ptmp.num_targets_ = P_.num_targets_; // Copy Constr. does not copy connections
  ptmp.set( d, *this, this );          // throws if BadProperty

  // We now know that ptmp is consistent. We do not write it back
  // to P_ before we are also sure that the properties to be set
  // in the parent class are internally consistent.
  StimulationDevice::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  P_.num_targets_ = ptmp.num_targets_;
}

inline SignalType
noise_generator::sends_signal() const
{
  return ALL;
}

inline void
noise_generator::calibrate_time( const TimeConverter& tc )
{
  P_.dt_ = tc.from_old_tics( P_.dt_.get_tics() );
}

inline bool
noise_generator::local_receiver() const
{
  return true;
}

inline StimulationDevice::Type
noise_generator::get_type() const
{
  return StimulationDevice::Type::CURRENT_GENERATOR;
}

} // namespace

#endif // NOISE_GENERATOR_H
