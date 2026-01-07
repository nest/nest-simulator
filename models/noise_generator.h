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
#include "event.h"
#include "nest_types.h"
#include "random_generators.h"
#include "stimulation_device.h"
#include "universal_data_logger_impl.h"

namespace nest
{

/* BeginUserDocs: device, generator

Short description
+++++++++++++++++

Generate a Gaussian white noise current

Description
+++++++++++

The `noise_generator` can be used to inject a Gaussian "white" noise current into a node.

The current is not truly white, but a piecewise constant current with a Gaussian distributed
amplitude with mean :math:`\mu` and standard deviation :math:`\sigma`. The current changes at
a user-defined interval :math:`\delta` and is given by

.. math::

  I(t) = \mu + N_j \sigma \quad \text{for} \quad t_0 + j \delta < t \leq t_0 + (j+1) \delta \;,

where :math:`N_j` are Gaussian random numbers with unit standard deviation and :math:`t_0` is
the device onset time.

Additionally a sinusodially modulated term can be added to the standard
deviation of the noise:

.. math::

   I(t) = \mu + N_j \sqrt{\sigma^2 + \sigma_{\text{mod}}^2 \sin(\omega t + \phi)}
                              \quad \text{for} \quad t_0 + j \delta < t \leq t_0 + (j+1) \delta \;.

The effect of the noise current on a neuron depends on the switching interval :math:`\delta`.
For a leaky integrate-and-fire neuron with time constant :math:`\tau_m` and capacitance
:math:`C_m`, the variance of the membrane potential is given by

.. math::

        \Sigma^2 = \frac{\delta \tau_m \sigma^2}{2 C_m^2}

for :math:`\delta \ll \tau_m`. For details, see the `noise generator notebook
<../model_details/noise_generator.ipynb>`_.

All targets of a noise generator receive different currents, but the currents for all
targets change at the same points in time. The interval :math:`\delta` between
changes must be a multiple of the time step.

.. admonition:: Recording the generated current

   You can use a :doc:`multimeter <multimeter>` to record the average current sent to all targets for each time step
   if simulating on a single thread; multiple MPI processes with one thread each also work. In this case,
   the recording interval of the multimeter should be equal to the simulation resolution to avoid confusing effects
   due to offset or drift between the recording times of the multimeter and the switching times of the
   noise generator. In multi-threaded mode, recording of noise currents is prohibited for technical reasons.


.. include:: ../models/stimulation_device.rst

mean
    The mean value :math:`\mu` of the noise current (pA)

std
    The standard deviation :math:`\sigma` of the noise current (pA)

dt
    The interval :math:`\delta` between changes in current (ms; default: 10 * resolution)

std_mod
    The modulation :math:`\sigma_{\text{mod}}` of the standard deviation of the noise current (pA)

frequency
    The frequency of the sine modulation (Hz)

phase
    The phase of sine modulation (0–360 deg)


Setting parameters from a stimulation backend
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: noise_generator

EndUserDocs */

void register_noise_generator( const std::string& name );

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
  using Node::event_hook;
  using Node::handle;
  using Node::handles_test_event;
  using Node::sends_signal;

  size_t send_test_event( Node&, size_t, synindex, bool ) override;

  SignalType sends_signal() const override;

  void handle( DataLoggingRequest& ) override;

  size_t handles_test_event( DataLoggingRequest&, size_t ) override;

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
  void pre_run_hook() override;

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

    Time get_default_dt();
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

inline size_t
noise_generator::handles_test_event( DataLoggingRequest& dlr, size_t receptor_type )
{
  if ( kernel::manager< VPManager >.get_num_threads() > 1 )
  {
    throw KernelException( "Recording from a noise_generator is only possible in single-threaded mode." );
  }

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

inline Time
noise_generator::Parameters_::get_default_dt()
{
  return 10 * Time::get_resolution();
}

template <>
void RecordablesMap< noise_generator >::create();

} // namespace

#endif // NOISE_GENERATOR_H
