/*
 *  multimeter.h
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

#ifndef MULTIMETER_H
#define MULTIMETER_H

// C++ includes:
#include <vector>

// Includes from nestkernel:
#include "connection.h"
#include "device_node.h"
#include "exceptions.h"
#include "kernel_manager.h"
#include "nest_timeconverter.h"
#include "recording_device.h"

// Includes from sli:
#include "dictutils.h"
#include "name.h"

/* BeginUserDocs: device, recorder

Short description
+++++++++++++++++

Sampling continuous quantities from neurons

Description
+++++++++++

Most sampling use cases are covered by the ``multimeter``, which
allows to record analog values from neurons. Models which have such
values expose a ``recordables`` property that lists all recordable
quantities. This property can be inspected using ``GetDefaults`` on
the model class or ``GetStatus`` on a model instance. It cannot be
changed by the user.

::

   >>> nest.GetDefaults('iaf_cond_alpha')['recordables']
   ['g_ex', 'g_in', 't_ref_remaining', 'V_m']

The ``record_from`` property of a ``multimeter`` (a list, empty by
default) can be set to contain the name(s) of one or more of these
recordables to have them sampled during simulation.

::

   mm = nest.Create('multimeter', 1, {'record_from': ['V_m', 'g_ex']})

The sampling interval for recordings (given in ms) can be controlled
using the ``multimeter`` parameter `interval`. The default value of
1.0 ms can be changed by supplying a new value either in the call to
``Create`` or by using ``SetStatus`` on the model instance.

::

   nest.SetStatus(mm, 'interval': 0.1})

The recording interval must be greater than or equal to the
:doc:`simulation resolution <running_simulations>`, which defaults to
0.1 ms.

.. warning::

   The set of variables to record from and the recording interval must
   be set **before** the ``multimeter`` is connected to any neuron.
   These properties cannot be changed afterwards.

After configuration, a ``multimeter`` can be connected to the neurons
it should record from by using the standard ``Connect`` routine.

::

    neurons = nest.Create('iaf_psc_alpha', 100)
    nest.Connect(mm, neurons)

To learn more about possible connection patterns and additional
options when using ``Connect``, see the guide on :doc:`connection
management <connection_management>`.

The above call to ``Connect`` would fail if the neurons would not
support the sampling of the values *V_m* and *g_ex*. It would also
fail if carried out in the wrong direction, i.e., trying to connect the
*neurons* to *mm*.

.. note::

   A pre-configured  ``multimeter`` is available under the name ``voltmeter``.  Its
   ``record_from`` property is already set to record the variable ``V_m``
   from the neurons it is connected to.

EndUserDocs */

namespace nest
{

class multimeter : public RecordingDevice
{

public:
  multimeter();
  multimeter( const multimeter& );

  /**
   * @note multimeters never have proxies, since they must
   *       sample their targets through local communication.
   */
  bool
  has_proxies() const
  {
    return false;
  }

  Name
  get_element_type() const
  {
    return names::recorder;
  }

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;
  using Node::sends_signal;

  port send_test_event( Node&, rport, synindex, bool );

  void handle( DataLoggingReply& );

  SignalType sends_signal() const;

  Type get_type() const;
  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

  void calibrate_time( const TimeConverter& tc );

protected:
  void calibrate();

  /**
   * Collect and output membrane potential information.
   * This function pages all its targets at all pertinent sample
   * points for membrane potential information and then outputs
   * that information. The sampled nodes must provide data from
   * the previous time slice.
   */
  void update( Time const&, const long, const long );

private:
  struct Buffers_;

  struct Parameters_
  {
    Time interval_;                   //!< recording interval, in ms
    Time offset_;                     //!< offset relative to 0, in ms
    std::vector< Name > record_from_; //!< which data to record

    Parameters_();
    Parameters_( const Parameters_& );
    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum&, const Buffers_&, Node* node );
  };

  // ------------------------------------------------------------


  struct Buffers_
  {
    /** Does this multimeter have targets?
     * Placed here since it is implementation detail.
     * @todo Ideally, one should be able to ask ConnectionManager.
     */
    Buffers_();

    bool has_targets_;
  };

  // ------------------------------------------------------------


  Parameters_ P_;
  Buffers_ B_;
};


inline void
nest::multimeter::get_status( DictionaryDatum& d ) const
{
  RecordingDevice::get_status( d );
  P_.get( d );

  if ( is_model_prototype() )
  {
    return; // no data to collect
  }

  // if we are the device on thread 0, also get the data from the
  // siblings on other threads
  if ( get_thread() == 0 )
  {
    const std::vector< Node* > siblings = kernel().node_manager.get_thread_siblings( get_node_id() );
    std::vector< Node* >::const_iterator s;
    for ( s = siblings.begin() + 1; s != siblings.end(); ++s )
    {
      ( *s )->get_status( d );
    }
  }
}

inline void
nest::multimeter::set_status( const DictionaryDatum& d )
{
  // protect multimeter from being frozen
  bool freeze = false;
  if ( updateValue< bool >( d, names::frozen, freeze ) && freeze )
  {
    throw BadProperty( "multimeter cannot be frozen." );
  }

  Parameters_ ptmp = P_;
  ptmp.set( d, B_, this );

  RecordingDevice::set_status( d );
  P_ = ptmp;
}

inline SignalType
nest::multimeter::sends_signal() const
{
  return ALL;
}

inline void
nest::multimeter::calibrate_time( const TimeConverter& tc )
{
  P_.interval_ = tc.from_old_tics( P_.interval_.get_tics() );
  P_.offset_ = tc.from_old_tics( P_.offset_.get_tics() );
}


//
// Declaration of voltmeter subclass
//

class voltmeter : public multimeter
{
public:
  voltmeter();
  voltmeter( const voltmeter& );
};

} // namespace nest

#endif
