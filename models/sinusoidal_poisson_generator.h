/*
 *  sinusoidal_poisson_generator.h
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

#ifndef SINUSOIDAL_POISSON_GENERATOR_H
#define SINUSOIDAL_POISSON_GENERATOR_H

// Includes from nestkernel:
#include "connection.h"
#include "device_node.h"
#include "event.h"
#include "nest_types.h"
#include "random_generators.h"
#include "stimulation_device.h"
#include "universal_data_logger.h"

namespace nest
{

/* BeginUserDocs: device, generator

Short description
+++++++++++++++++

Generate sinusoidally modulated Poisson spike trains

Description
+++++++++++

sinusoidal_poisson_generator generates sinusoidally modulated Poisson spike
trains. By default, each target of the generator will receive a different
spike train.

The instantaneous rate of the process is given by

.. math::

  f(t) = max(0, rate + amplitude \sin ( 2 \pi frequency t + phase
     * \pi/180 )) >= 0

Remarks
+++++++

- If amplitude > rate, firing rate is cut off at zero. In this case, the mean
  firing rate will be less than rate.
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

.. include:: ../models/stimulation_device.rst

rate
    Mean firing rate in spikes/second, default: 0 s^-1

amplitude
    Firing rate modulation amplitude in spikes/second, default: 0 s^-1

frequency
    Modulation frequency, default: 0 Hz

phase
    Modulation phase in degree [0-360], default: 0

individual_spike_trains
    See note above, default: true

Set parameters from a stimulation backend
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The parameters in this stimulation device can be updated with input
coming from a stimulation backend. The data structure used for the
update holds one value for each of the parameters mentioned above.
The indexing is as follows:

 0. rate
 1. frequency
 2. phase
 3. amplitude
 4. individual_spike_trains

Receives
++++++++

DataLoggingRequest

Sends
+++++

SpikeEvent

See also
++++++++

poisson_generator, sinusoidal_gamma_generator

EndUserDocs */

class sinusoidal_poisson_generator : public StimulationDevice
{

public:
  sinusoidal_poisson_generator();
  sinusoidal_poisson_generator( const sinusoidal_poisson_generator& );

  port send_test_event( Node&, rport, synindex, bool ) override;

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;
  using Node::event_hook;

  void handle( DataLoggingRequest& ) override;

  port handles_test_event( DataLoggingRequest&, rport ) override;

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

  //! Model can be switched between proxies (single spike train) and not
  bool
  has_proxies() const override
  {
    return not P_.individual_spike_trains_;
  }

  //! Allow multimeter to connect to local instances
  bool
  local_receiver() const override
  {
    return true;
  }

  Name
  get_element_type() const override
  {
    return names::stimulator;
  }

  StimulationDevice::Type
  get_type() const override
  {
    return StimulationDevice::Type::CURRENT_GENERATOR;
  };

  void set_data_from_stimulation_backend( std::vector< double >& input_param ) override;

private:
  void init_state_() override;
  void init_buffers_() override;
  void calibrate() override;
  void event_hook( DSSpikeEvent& ) override;

  void update( Time const&, const long, const long ) override;

  struct Parameters_
  {
    /** Temporal frequency in radian/ms */
    double om_;

    /** Phase in radian */
    double phi_;

    /** Mean firing rate in spikes/ms */
    double rate_;

    /** Firing rate modulation amplitude in spikes/ms */
    double amplitude_;

    /** Emit individual spike trains for each target, or same for all? */
    bool individual_spike_trains_;

    Parameters_(); //!< Sets default parameter values
    Parameters_( const Parameters_& );
    Parameters_& operator=( const Parameters_& p );

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary

    /**
     * Set values from dictionary.
     * @note State is passed so that the position can be reset if the
     *       spike_times_ vector has been filled with new data.
     */
    void set( const DictionaryDatum&, const sinusoidal_poisson_generator&, Node* );
  };

  struct State_
  {
    //! Two-component oscillator state vector, see Rotter&Diesmann
    double y_0_;
    double y_1_;

    double rate_; //!< current rate, kept for recording

    State_(); //!< Sets default state value

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
  };

  // ------------------------------------------------------------

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< sinusoidal_poisson_generator >;
  friend class UniversalDataLogger< sinusoidal_poisson_generator >;

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    explicit Buffers_( sinusoidal_poisson_generator& );
    Buffers_( const Buffers_&, sinusoidal_poisson_generator& );
    UniversalDataLogger< sinusoidal_poisson_generator > logger_;
  };

  // ------------------------------------------------------------

  struct Variables_
  {
    poisson_distribution poisson_dist_; //!< poisson distribution

    double h_;   //! time resolution (ms)
    double sin_; //!< sin(h om) in propagator
    double cos_; //!< cos(h om) in propagator
  };

  double
  get_rate_() const
  {
    return 1000.0 * S_.rate_;
  }

  // ------------------------------------------------------------

  static RecordablesMap< sinusoidal_poisson_generator > recordablesMap_;

  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;
};

inline port
sinusoidal_poisson_generator::send_test_event( Node& target, rport receptor_type, synindex syn_id, bool dummy_target )
{
  StimulationDevice::enforce_single_syn_type( syn_id );

  // to ensure correct overloading resolution, we need explicit event types
  // therefore, we need to duplicate the code here
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
    return target.handles_test_event( e, receptor_type );
  }
}

inline port
sinusoidal_poisson_generator::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
sinusoidal_poisson_generator::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
  StimulationDevice::get_status( d );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
sinusoidal_poisson_generator::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors

  ptmp.set( d, *this, this ); // throws if BadProperty
  // We now know that ptmp is consistent. We do not write it back
  // to P_ before we are also sure that the properties to be set
  // in the parent class are internally consistent.
  StimulationDevice::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}

} // namespace

#endif // SINUSOIDAL_POISSON_GENERATOR_H
