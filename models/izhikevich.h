/*
 *  izhikevich.h
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

#ifndef IZHIKEVICH_H
#define IZHIKEVICH_H

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"

namespace nest
{

/** @BeginDocumentation
@ingroup Neurons
@ingroup iaf

Name: izhikevich - Izhikevich neuron model

Description:
Implementation of the simple spiking neuron model introduced by Izhikevich
[1]. The dynamics are given by:
  @f[
  dv/dt = 0.04*v^2 + 5*v + 140 - u + I \\
     du/dt = a*(b*v - u)] @f]

    if  \f$ v >= V_{th} \f$:
      v is set to c
      u is incremented by d

    v jumps on each spike arrival by the weight of the spike.

As published in [1], the numerics differs from the standard forward Euler
technique in two ways:
1) the new value of u is calculated based on the new value of v, rather than
the previous value
2) the variable v is updated using a time step half the size of that used to
update variable u.

This model offers both forms of integration, they can be selected using the
boolean parameter consistent_integration. To reproduce some results published
on the basis of this model, it is necessary to use the published form of the
dynamics. In this case, consistent_integration must be set to false. For all
other purposes, it is recommended to use the standard technique for forward
Euler integration. In this case, consistent_integration must be set to true
(default).


Parameters:
The following parameters can be set in the status dictionary.

\verbatim embed:rst
======================= =======  ==============================================
 V_m                    mV       Membrane potential
 U_m                    mV       Membrane potential recovery variable
 V_th                   mV       Spike threshold
 I_e                    pA       Constant input current (R=1)
 V_min                  mV       Absolute lower value for the membrane potential
 a                      real     Describes time scale of recovery variable
 b                      real     Sensitivity of recovery variable
 c                      mV       After-spike reset value of V_m
 d                      mV       After-spike reset value of U_m
 consistent_integration boolean  Use standard integration technique
======================= =======  ==============================================
\endverbatim

References:

\verbatim embed:rst
.. [1] Izhikevich EM (2003). Simple model of spiking neurons. IEEE Transactions
on
       Neural Networks, 14:1569-1572.
       DOI: https://doi.org/10.1109/TNN.2003.820440
\endverbatim

Sends: SpikeEvent

Receives: SpikeEvent, CurrentEvent, DataLoggingRequest

FirstVersion: 2009

Author: Hanuschkin, Morrison, Kunkel

SeeAlso: iaf_psc_delta, mat2_psc_exp
*/
class izhikevich : public Archiving_Node
{

public:
  izhikevich();
  izhikevich( const izhikevich& );

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;

  void handle( DataLoggingRequest& );
  void handle( SpikeEvent& );
  void handle( CurrentEvent& );

  port handles_test_event( DataLoggingRequest&, rport );
  port handles_test_event( SpikeEvent&, rport );
  port handles_test_event( CurrentEvent&, rport );

  port send_test_event( Node&, rport, synindex, bool );

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  friend class RecordablesMap< izhikevich >;
  friend class UniversalDataLogger< izhikevich >;

  void init_state_( const Node& proto );
  void init_buffers_();
  void calibrate();

  void update( Time const&, const long, const long );

  // ----------------------------------------------------------------

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {
    double a_;
    double b_;
    double c_;
    double d_;

    /** External DC current */
    double I_e_;

    /** Threshold */
    double V_th_;

    /** Lower bound */
    double V_min_;

    /** Use standard integration numerics **/
    bool consistent_integration_;

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const;             //!< Store current values in dictionary
    void set( const DictionaryDatum&, Node* node ); //!< Set values from dicitonary
  };

  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   */
  struct State_
  {
    double v_; // membrane potential
    double u_; // membrane recovery variable
    double I_; // input current


    /** Accumulate spikes arriving during refractory period, discounted for
        decay until end of refractory period.
    */

    State_(); //!< Default initialization

    void get( DictionaryDatum&, const Parameters_& ) const;
    void set( const DictionaryDatum&, const Parameters_&, Node* );
  };

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    /**
     * Buffer for recording
     */
    Buffers_( izhikevich& );
    Buffers_( const Buffers_&, izhikevich& );
    UniversalDataLogger< izhikevich > logger_;

    /** buffers and sums up incoming spikes/currents */
    RingBuffer spikes_;
    RingBuffer currents_;
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {
  };

  // Access functions for UniversalDataLogger -----------------------

  //! Read out the membrane potential
  double
  get_V_m_() const
  {
    return S_.v_;
  }
  //! Read out the recovery variable
  double
  get_U_m_() const
  {
    return S_.u_;
  }

  // ----------------------------------------------------------------

  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;

  //! Mapping of recordables names to access functions
  static RecordablesMap< izhikevich > recordablesMap_;
  /** @} */
};

inline port
izhikevich::send_test_event( Node& target, rport receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}

inline port
izhikevich::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
izhikevich::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
izhikevich::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
izhikevich::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  Archiving_Node::get_status( d );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
izhikevich::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;     // temporary copy in case of errors
  ptmp.set( d, this );       // throws if BadProperty
  State_ stmp = S_;          // temporary copy in case of errors
  stmp.set( d, ptmp, this ); // throws if BadProperty

  // We now know that (ptmp, stmp) are consistent. We do not
  // write them back to (P_, S_) before we are also sure that
  // the properties to be set in the parent class are internally
  // consistent.
  Archiving_Node::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

} // namespace nest

#endif /* #ifndef IZHIKEVICH_H */
