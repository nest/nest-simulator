/*
 *  ac_generator.h
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

#ifndef AC_GENERATOR_H
#define AC_GENERATOR_H

// Includes from nestkernel:
#include "connection.h"
#include "device_node.h"
#include "event.h"
#include "nest_types.h"
#include "stimulation_device.h"
#include "universal_data_logger.h"

/* BeginUserDocs: device, generator

Short description
+++++++++++++++++

Produce an alternating current (AC) input

Description
+++++++++++

This device produces an AC input sent by CurrentEvents. The
current is given by

.. math::

        I(t) = \mathrm{offset} + \mathrm{amplitude} \cdot \sin ( \omega t + \phi )

where

.. math::

    \omega  = 2 \pi \cdot \mathrm{frequency} \\
    \phi = \frac{\mathrm{phase}}{180} \cdot \pi

.. include:: ../models/stimulation_device.rst

amplitude
    Amplitude of sine current (pA)

offset
    Constant amplitude offset (pA)

frequency
    Frequency (Hz)

phase
    Phase of sine current (0-360 deg)

Setting `start` and `stop` only windows the current as defined above. It
does not shift the time axis.

Set parameters from a stimulation backend
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The parameters in this stimulation device can be updated with input
coming from a stimulation backend. The data structure used for the
update holds one value for each of the parameters mentioned above.
The indexing is as follows:

 0. amplitude
 1. offset
 2. frequency
 3. phase

References
++++++++++

.. [1] Rotter S and Diesmann M (1999). Exact digital simulation of time-
       invariant linear systems with applications to neuronal modeling,
       Biol. Cybern. 81, 381-402. DOI: https://doi.org/10.1007/s004220050570

Sends
+++++

CurrentEvent

See also
++++++++

dc_generator, noise_generator, step_current_generator, StimulationDevice,
Device

EndUserDocs */

namespace nest
{
class ac_generator : public StimulationDevice
{

public:
  ac_generator();
  ac_generator( const ac_generator& );

  //! Allow multimeter to connect to local instances
  bool local_receiver() const override;

  port send_test_event( Node&, rport, synindex, bool ) override;

  using Node::handle;
  using Node::handles_test_event;

  void handle( DataLoggingRequest& ) override;

  port handles_test_event( DataLoggingRequest&, rport ) override;

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

  StimulationDevice::Type get_type() const override;
  void set_data_from_stimulation_backend( std::vector< double >& input_param ) override;

private:
  void init_state_() override;
  void init_buffers_() override;
  void calibrate() override;

  void update( Time const&, const long, const long ) override;


  // ------------------------------------------------------------

  struct Parameters_
  {
    double amp_;     //!< Amplitude of sine-current
    double offset_;  //!< Offset of sine-current
    double freq_;    //!< Standard frequency in Hz
    double phi_deg_; //!< Phase of sine current (0-360 deg)

    Parameters_(); //!< Sets default parameter values
    Parameters_( const Parameters_& );
    Parameters_& operator=( const Parameters_& p );

    void get( DictionaryDatum& ) const;             //!< Store current values in dictionary
    void set( const DictionaryDatum&, Node* node ); //!< Set values from dictionary
  };

  // ------------------------------------------------------------

  struct State_
  {
    double y_0_;
    double y_1_;
    double I_; //!< Instantaneous current value; used for recording current
               //!< Required to handle current values when device is inactive

    State_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
  };

  // ------------------------------------------------------------

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< ac_generator >;
  friend class UniversalDataLogger< ac_generator >;

  // ------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    explicit Buffers_( ac_generator& );
    Buffers_( const Buffers_&, ac_generator& );
    UniversalDataLogger< ac_generator > logger_;
  };

  // ------------------------------------------------------------

  struct Variables_
  {
    // The exact integration matrix
    double A_00_;
    double A_01_;
    double A_10_;
    double A_11_;
  };

  double
  get_I_() const
  {
    return S_.I_;
  }

  // ------------------------------------------------------------

  static RecordablesMap< ac_generator > recordablesMap_;
  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;
};

inline port
ac_generator::send_test_event( Node& target, rport receptor_type, synindex syn_id, bool )
{
  StimulationDevice::enforce_single_syn_type( syn_id );

  CurrentEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}

inline port
ac_generator::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
ac_generator::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
  StimulationDevice::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
ac_generator::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d, this );   // throws if BadProperty

  // State_ is read-only

  // We now know that ptmp is consistent. We do not write it back
  // to P_ before we are also sure that the properties to be set
  // in the parent class are internally consistent.
  StimulationDevice::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}

inline bool
ac_generator::local_receiver() const
{
  return true;
}

inline StimulationDevice::Type
ac_generator::get_type() const
{
  return StimulationDevice::Type::CURRENT_GENERATOR;
}

} // namespace

#endif // AC_GENERATOR_H
