/*
 *  ht_connection.h
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

#ifndef HT_CONNECTION_H
#define HT_CONNECTION_H

#include "connection_het_wd.h"

/* BeginDocumentation
  Name: ht_synapse - Synapse with depression after Hill & Tononi (2005).

  Description:
  This synapse implements the depression model described in [1, p 1678].
  Synaptic dynamics are given by

  P'(t) = ( 1 - P ) / tau_p 
  P(T+) = (1 - delta_P) P(T-)   for T : time of a spike 
  P(t=0) = 1

  w(t) = w_max * P(t)  is the resulting synaptic weight

  Parameters: 
     The following parameters can be set in the status dictionary:
     tauP     double - synaptic vesicle pool recovery time constant [ms]
     delta_P  double - fractional change in vesicle pool on incoming spikes [unitless]
     P        double - current size of the vesicle pool [unitless, 0 <= P <= 1]

  Warning:
  THIS SYNAPSE MODEL HAS NOT BEEN TESTED EXTENSIVELY!
   
  References:
   [1] S Hill and G Tononi (2005). J Neurophysiol 93:1671-1698.

  Sends: SpikeEvent

  FirstVersion: March 2009
  Author: Hans Ekkehard Plesser, based on markram_synapse
  SeeAlso: ht_neuron, tsodyks_synapse, stdp_synapse, static_synapse
*/

/**
 * Class representing a synapse with Hill short term plasticity.  A
 * suitale Connector containing these connections can be obtained from
 * the template GenericConnector.
 */

namespace nest {

  //class CommonProperties;

class HTConnection : public ConnectionHetWD
{
 public:

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  HTConnection();

  /**
   * Default Destructor.
   */
  virtual ~HTConnection() {}

  // Import overloaded virtual function set to local scope. 
  using Connection::check_event;

  /**
   * Get all properties of this connection and put them into a dictionary.
   */
  virtual void get_status(DictionaryDatum & d) const;
  
  /**
   * Set properties of this connection from the values given in dictionary.
   */
  virtual void set_status(const DictionaryDatum & d, ConnectorModel &cm);

  /**
   * Set properties of this connection from position p in the properties
   * array given in dictionary.
   */  
  virtual void set_status(const DictionaryDatum & d, index p, ConnectorModel &cm);

  /**
   * Create new empty arrays for the properties of this connection in the given
   * dictionary. It is assumed that they are not existing before.
   */
  void initialize_property_arrays(DictionaryDatum & d) const;

  /**
   * Append properties of this connection to the given dictionary. If the
   * dictionary is empty, new arrays are created first.
   */
  virtual void append_properties(DictionaryDatum & d) const;

  /**
   * Send an event to the receiver of this connection.
   * \param e The event to send
   * \param t_lastspike Point in time of last spike sent.
   * \param cp Common properties to all synapses (empty).
   */
  void send(Event& e, double_t t_lastspike, const CommonSynapseProperties &cp);

  // overloaded for all supported event types
  void check_event(SpikeEvent&) {}
 
 private:
  double_t tau_P_;     //!< [ms] time constant for recovery
  double_t delta_P_;   //!< fractional decrease in pool size per spike

  double_t p_;         //!< current pool size
};


/**
 * Send an event to the receiver of this connection.
 * \param e The event to send
 * \param p The port under which this connection is stored in the Connector.
 * \param t_lastspike Time point of last spike emitted
 */
inline
void HTConnection::send(Event& e, double_t t_lastspike, 
			const CommonSynapseProperties &)
{
  double_t h = e.get_stamp().get_ms() - t_lastspike;

  // t_lastspike_ = 0 initially

  // propagation t_lastspike -> t_spike, t_lastspike_ = 0 initially, p_ = 1
  p_ = 1 - ( 1 - p_ ) * std::exp(-h/tau_P_);

  // send the spike to the target
  e.set_receiver(*target_);
  e.set_weight( weight_ * p_ );
  e.set_delay( delay_ );
  e.set_rport( rport_ );
  e();

  // reduce pool after spike is sent
  p_ *= ( 1 - delta_P_ );
}
 
} // namespace

#endif // HT_CONNECTION_H
