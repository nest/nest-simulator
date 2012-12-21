/*
 *  tsodyks2_connection.h
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

#ifndef TSODYKS2_CONNECTION_H
#define TSODYKS2_CONNECTION_H

#include "connection_het_wd.h"

/* BeginDocumentation
  Name: tsodyks2_synapse - Synapse type with short term plasticity.

  Description:
   This synapse model implements synaptic short-term depression and short-term facilitation
   according to [1] and [2]. It solves Eq (2) from [1] and modulates U according to eq. (2) of [2].

   This connection merely scales the synaptic weight, based on the spike history and the 
   parameters of the kinetic model. Thus, it is suitable for all types of synaptic dynamics,
   that is current or conductance based.

   The parameter A_se from the publications is represented by the
   synaptic weight. The variable x in the synapse properties is the
   factor that scales the synaptic weight.

   Parameters: 
     The following parameters can be set in the status dictionary:
     U          double - probability of release increment (U1) [0,1], default=0.5
     u          double - Maximum probability of release (U_se) [0,1], default=0.5
     x          double - current scaling factor of the weight, default=U 
     tau_rec    double - time constant for depression in ms, default=800 ms
     tau_rec    double - time constant for facilitation in ms, default=0 (off)

  Notes:

     Under identical conditions, the tsodyks2_synapse produces
     slightly lower peak amplitudes than the tsodyks_synapse. However,
     the qualitative behavior is identical. The script
     test_tsodyks2_synapse.py in the examples compares the two synapse
     models.


  References:
   [1] Tsodyks, M. V., & Markram, H. (1997). The neural code between neocortical pyramidal neurons 
       depends on neurotransmitter release probability. PNAS, 94(2), 719-23.
   [2] Fuhrmann, G., Segev, I., Markram, H., & Tsodyks, M. V. (2002). Coding of temporal 
       information by activity-dependent synapses. Journal of neurophysiology, 87(1), 140-8.

  Transmits: SpikeEvent
       
  FirstVersion: October 2011
  Author: Marc-Oliver Gewaltig, based on tsodyks_synapse by Moritz Helias
  SeeAlso: tsodyks_synapse, synapsedict, stdp_synapse, static_synapse
*/


/**
 * Class representing a synapse with Tsodyks short term plasticity, based on the iterative formula
 * A suitable Connector containing these connections can be obtained from the template GenericConnector.
 */

namespace nest {

class Tsodyks2Connection : public ConnectionHetWD
{
 public:

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  Tsodyks2Connection();

  /**
   * Default Destructor.
   */
  ~Tsodyks2Connection() {}

  /**
   * Get all properties of this connection and put them into a dictionary.
   */
  void get_status(DictionaryDatum & d) const;
  
  /**
   * Set properties of this connection from the values given in dictionary.
   */
  void set_status(const DictionaryDatum & d, ConnectorModel &cm);

  /**
   * Set properties of this connection from position p in the properties
   * array given in dictionary.
   */  
  void set_status(const DictionaryDatum & d, index p, ConnectorModel &cm);

  /**
   * Create new empty arrays for the properties of this connection in the given
   * dictionary. It is assumed that they are not existing before.
   */
  void initialize_property_arrays(DictionaryDatum & d) const;

  /**
   * Append properties of this connection to the given dictionary. If the
   * dictionary is empty, new arrays are created first.
   */
  void append_properties(DictionaryDatum & d) const;

  /**
   * Send an event to the receiver of this connection.
   * \param e The event to send
   * \param t_lastspike Point in time of last spike sent.
   * \param cp Common properties to all synapses (empty).
   */
  void send(Event& e, double_t t_lastspike, const CommonSynapseProperties &cp);

  // overloaded for all supported event types
  using Connection::check_event;
  void check_event(SpikeEvent&) {}
  
 private:
  double_t U_;       //!< unit increment of a facilitating synapse (U1)
  double_t u_;       //!< probability of release (Use)
  double_t x_;       //!< current fraction of the synaptic weight 
  double_t tau_rec_; //!< [ms] time constant for recovery
  double_t tau_fac_; //!< [ms] time constant for facilitation
};


/**
 * Send an event to the receiver of this connection.
 * \param e The event to send
 * \param p The port under which this connection is stored in the Connector.
 * \param t_lastspike Time point of last spike emitted
 */
inline
void Tsodyks2Connection::send(Event& e, double_t t_lastspike, const CommonSynapseProperties &)
{
  double_t h = e.get_stamp().get_ms() - t_lastspike;  
  double_t f = std::exp(-h/tau_rec_);
  double_t u_decay = (tau_fac_ < 1.0e-10) ? 0.0 : std::exp(-h/tau_fac_);

  x_= x_*(1.0-u_)*f + u_*(1.0-f); // Eq. 2 from reference [1]
  u_ *= u_decay; 
  u_+= U_*(1.0-u_); // for tau_fac=0 and u_=0, this will render u_==U_

  // send the spike to the target
  e.set_receiver(*target_);
  e.set_weight( x_*weight_ );
  e.set_delay( delay_ );
  e.set_rport( rport_ );
  e();
}
 
} // namespace

#endif // TSODYKS2_CONNECTION_H
