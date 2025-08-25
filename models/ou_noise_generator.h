/*
 *  ou_noise_generator.h
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

#ifndef OU_NOISE_GENERATOR_H
#define OU_NOISE_GENERATOR_H

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

Generates a temporally correlated noise current based on an Ornstein-Uhlenbeck process.

Description
+++++++++++

The `ou_noise_generator` can be used to inject a temporally correlated noise current into a node.
The current I(t) follows an Ornstein-Uhlenbeck (OU) process, which is described by the following stochastic differential
equation:

.. math::

  dI = \frac{1}{\tau}(\mu - I)dt + \sigma_{stat} \sqrt{\frac{2}{\tau}} dW

where:
 - :math:`\mu` is the long-term mean of the process (`mean` parameter).
 - :math:`\tau` is the time constant of the correlation (`tau` parameter).
 - :math:`\sigma_{stat}` is the stationary standard deviation of the process (`std` parameter).
 - :math:`dW` is a Wiener process (Gaussian white noise).

The generator integrates this process at a user-defined interval `dt` and delivers the resulting current to its targets.
A larger time constant :math:`\tau` results in a more slowly varying noise signal.

All targets of a noise generator receive different, independent noise currents, but the currents for all targets are
updated at the same points in time. The interval `dt` between updates must be a multiple of the simulation time step.

.. admonition:: Recording the generated current

  You can use a :doc:`multimeter <multimeter>` to record the average current sent to all targets for each time step if
  simulating on a single thread; multiple MPI processes with one thread each also work. In this case, the recording
  interval of the multimeter should be equal to the `dt` of the generator to avoid aliasing effects. In multi-threaded
  mode, recording of noise currents is prohibited for technical reasons.


.. include:: ../models/stimulation_device.rst

mean
    The mean value :math:`\mu` to which the process reverts (pA).

std
    The stationary standard deviation :math:`\sigma_{stat}` of the process (pA).

tau
    The correlation time constant :math:`\tau` of the process (ms).

dt
    The interval :math:`\delta` between updates of the noise current (ms).


Setting parameters from a stimulation backend
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The parameters in this stimulation device can be updated with input
coming from a stimulation backend. The data structure used for the
update holds one value for each of the parameters mentioned above.
The indexing is as follows:

 0. mean
 1. std
 2. tau


EndUserDocs */

void register_ou_noise_generator( const std::string& name );

class ou_noise_generator : public StimulationDevice
{

public:
  ou_noise_generator();
  ou_noise_generator( const ou_noise_generator& );


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
    double mean_; //!< mean current, in pA
    double std_;  //!< standard deviation of current, in pA
    double tau_;  //!< Standard frequency in Hz
    Time dt_;     //!< time interval between updates

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
    void set( const DictionaryDatum&, const ou_noise_generator&, Node* node );

    Time get_default_dt();
  };

  // ------------------------------------------------------------

  struct State_
  {
    double I_avg_; //!< Average of instantaneous currents computed
                   //!< Used for recording current

    State_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
  };

  // ------------------------------------------------------------

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< ou_noise_generator >;
  friend class UniversalDataLogger< ou_noise_generator >;

  // ------------------------------------------------------------

  struct Buffers_
  {
    long next_step_; //!< time step of next change in current
    AmpVec_ amps_;   //!< amplitudes, one per target
    explicit Buffers_( ou_noise_generator& );
    Buffers_( const Buffers_&, ou_noise_generator& );
    UniversalDataLogger< ou_noise_generator > logger_;
  };

  // ------------------------------------------------------------

  struct Variables_
  {
    normal_distribution normal_dist_; //!< normal distribution

    long dt_steps_;    //!< update interval in steps
    double prop_;      //!< frequency [radian/s]
    double noise_amp_; //!<
    double tau_inv_;   //!<
  };

  double
  get_I_avg_() const
  {
    return S_.I_avg_;
  }

  // ------------------------------------------------------------

  static RecordablesMap< ou_noise_generator > recordablesMap_;

  Parameters_ P_;
  State_ S_;
  Buffers_ B_;
  Variables_ V_;
};

inline size_t
ou_noise_generator::handles_test_event( DataLoggingRequest& dlr, size_t receptor_type )
{
  if ( kernel().vp_manager.get_num_threads() > 1 )
  {
    throw KernelException( "Recording from a ou_noise_generator is only possible in single-threaded mode." );
  }

  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
ou_noise_generator::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
  StimulationDevice::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
ou_noise_generator::set_status( const DictionaryDatum& d )
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
ou_noise_generator::sends_signal() const
{
  return ALL;
}

inline bool
ou_noise_generator::local_receiver() const
{
  return true;
}

inline StimulationDevice::Type
ou_noise_generator::get_type() const
{
  return StimulationDevice::Type::CURRENT_GENERATOR;
}

inline Time
ou_noise_generator::Parameters_::get_default_dt()
{
  return 10 * Time::get_resolution();
}

} // namespace

#endif // OU_NOISE_GENERATOR_H
