/*
 *  stdp_dopa_connection.h
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

#ifndef STDP_DOPA_CONNECTION_H
#define STDP_DOPA_CONNECTION_H

/*BeginDocumentation

   Name: stdp_dopamine_synapse - Synapse type for dopamine-modulated spike-timing dependent plasticity.

   Description:
   stdp_dopamine_synapse is a connector to create synapses with
   dopamine-modulated spike-timing dependent plasticity (used as a
   benchmark model in [1], based on [2]). The dopaminergic signal is a
   low-pass filtered version of the spike rate of a user-specific pool
   of neurons. The spikes emitted by the pool of dopamine neurons are
   delivered to the synapse via the assigned volume transmitter. The
   dopaminergic dynamics is calculated in the synapses itself.
   
   Examples:
   /volume_transmitter Create /vol Set
   /iaf_neuron Create /pre_neuron Set
   /iaf_neuron Create /post_neuron Set
   /iaf_neuron Create /neuromod_neuron Set
   /stdp_dopamine_synapse  << /vt vol >>  SetDefaults
   neuromod_neuron vol Connect
   pre_neuron post_neuron /stdp_dopamine_synapse Connect
    
   Parameters:
   vt        long   - ID of volume_transmitter collecting the spikes from the pool of
                      dopamine releasing neurons and transmitting the spikes
                      to the synapse. If no volume transmitter has been
                      assigned, a value of -1 is returned when the synapse is
                      asked for its defaults.             
   A_plus    double - Amplitude of weight change for facilitation
   A_minus   double - Amplitude of weight change for depression
   tau_plus  double - STDP time constant for facilitation in ms
   tau_minus double - STDP time constant for depression in ms
   tau_d     double - Time constant of dopaminergic trace in ms 
   dopa_base double - Dopaminergic baseline concentration
   tau_e     double - Time constant of eligibility trace in ms
   Wmin      double - Minimal synaptic weight
   Wmax      double - Maximal synaptic weight
  
   References:
   [1] Potjans W, Morrison A and Diesmann M (2010). Enabling
       functional neural circuit simulations with distributed
       computing of neuromodulated plasticity.
       Front. Comput. Neurosci. 4:141. doi:10.3389/fncom.2010.00141
   [2] Izhikevich, E.M. (2007). Solving the distal reward problem
       through linkage of STDP and dopamine signaling. Cereb. Cortex,
       17(10), 2443-2452.

   Transmits: SpikeEvent
  
   Author: Wiebke Potjans
   SeeAlso: volume_transmitter
*/

#include "connection_het_wd.h"
#include "archiving_node.h"  
#include <cmath>
#include "volume_transmitter.h"
#include "spikecounter.h"

namespace nest
{

  /**
   * Class containing the common properties for all synapses of type DOPAMINEConnection.
   */
  class STDPDopaCommonProperties : public CommonSynapseProperties
    {
      friend class STDPDopaConnection;

    public:
      /**
       * Default constructor.
       * Sets all property values to defaults.
       */
      STDPDopaCommonProperties();
   
      /**
       * Get all properties and put them into a dictionary.
       */
      void get_status(DictionaryDatum & d) const;
  
      /**
       * Set properties from the values given in dictionary.
       */
      void set_status(const DictionaryDatum & d, ConnectorModel& cm);

      // overloaded for all supported event types
      void check_event(SpikeEvent&) {}

      Node* get_node();

 
    private:
      volume_transmitter * vt_; 
      double_t tau_d_;     
      double_t tau_e_;     
      double_t A_plus_;    
      double_t tau_plus_;  
      double_t A_minus_;  
      double_t tau_minus_; 
      double_t dopa_base_; 
      double_t Wmin_;      
      double_t Wmax_;      
    };



  /**
   * Class representing an STDPDopa connection with homogeneous parameters, i.e. parameters are the same for all synapses.
   */
  class STDPDopaConnection : public ConnectionHetWD
  {

  public:
  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  STDPDopaConnection();
  
  /**
   * Copy constructor from a property object.
   * Needs to be defined properly in order for GenericConnector to work.
   */
  STDPDopaConnection(const STDPDopaConnection &);

  /**
   * Default Destructor.
   */
  virtual ~STDPDopaConnection() {}

  // Import overloaded virtual function set to local scope. 
  using Connection::check_event;

  /*
   * This function calls check_connection on the sender and checks if the receiver
   * accepts the event type and receptor type requested by the sender.
   * Node::check_connection() will either confirm the receiver port by returning
   * true or false if the connection should be ignored.
   * We have to override the base class' implementation, since for STDPDopa
   * connections we have to call register_dopamine_connection on the target neuron
   * to inform the Archiver to collect spikes for this connection.
   *
   * \param s The source node
   * \param r The target node
   * \param receptor_type The ID of the requested receptor type
   */
  void check_connection(Node & s, Node & r, rport receptor_type, double_t t_lastspike);

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
   */
  void send(Event& e, double_t t_lastspike, const STDPDopaCommonProperties &);


  // overloaded for all supported event types
  void check_event(SpikeEvent&) {}
  
  void trigger_update_weight(const vector<spikecounter> &dopa_spikes, const STDPDopaCommonProperties &cp);
  void update_weight(double_t this_update, const STDPDopaCommonProperties &cp);
    
  private:
 
 // data members of each connection
  double_t last_update_;
  double_t last_post_spike_;
  double_t last_e_update_;
  double_t eligibility_;
  double_t last_dopa_spike_;
  double_t dopa_trace_;
  double_t last_spike_;  
 
  };


  inline
    void STDPDopaConnection::trigger_update_weight(const vector<spikecounter> &dopa_spikes, const STDPDopaCommonProperties &cp)
  {

    double_t dendritic_delay = Time(Time::step(delay_)).get_ms();
    //get spike history of postsynaptic neuron in range (t1,t2]
    std::deque<histentry>::iterator start;
    std::deque<histentry>::iterator finish;
    target_->get_history(last_update_ - dendritic_delay, dopa_spikes.back().spike_time_ - dendritic_delay, &start, &finish);
    for (uint_t i=0; i<dopa_spikes.size();i++)
      {
	if (dopa_spikes[i].spike_time_ >= last_update_)
	  {
	    double_t this_dopa_spike = dopa_spikes[i].spike_time_;
	    while((start->t_ + dendritic_delay <= this_dopa_spike)&&(start!=finish))
	      {
		update_weight(start->t_ + dendritic_delay, cp);
		eligibility_= eligibility_*std::exp((last_e_update_ -(start->t_+dendritic_delay))/cp.tau_e_)+cp.A_plus_*std::exp((last_spike_ - (start->t_+dendritic_delay))/cp.tau_plus_);
		last_e_update_ = start->t_ + dendritic_delay;
		last_post_spike_ = start->t_;
		start++;
	      } 
	    update_weight(this_dopa_spike, cp);
	    dopa_trace_ = dopa_trace_*std::exp((last_dopa_spike_-this_dopa_spike)/cp.tau_d_)+dopa_spikes[i].multiplicity_/cp.tau_d_;
	    last_dopa_spike_ = this_dopa_spike;
		
	  }
      }
      
  }


      
  
  inline
    void STDPDopaConnection::update_weight(double_t this_update, const STDPDopaCommonProperties &cp)
  {
    
    weight_ = weight_ - (dopa_trace_-cp.dopa_base_)*eligibility_/(1./cp.tau_e_+1./cp.tau_d_)*(std::exp((last_e_update_-this_update)/cp.tau_e_)*std::exp((last_dopa_spike_-this_update)/cp.tau_d_)-std::exp((last_e_update_-last_update_)/cp.tau_e_)*std::exp((last_dopa_spike_-last_update_)/cp.tau_d_));
    last_update_ = this_update;
    
   
    if(weight_ < cp.Wmin_)
      weight_ = cp.Wmin_;
    if(weight_ > cp.Wmax_)
      weight_= cp.Wmax_;
      
  }
  

inline 
  void STDPDopaConnection::check_connection(Node & s, Node & r, rport receptor_type, double_t t_lastspike)
{
  ConnectionHetWD::check_connection(s, r, receptor_type, t_lastspike);
  r.register_stdp_connection(t_lastspike - Time(Time::step(delay_)).get_ms());
  
}
/**
 * Send an event to the receiver of this connection.
 * \param e The event to send
 * \param p The port under which this connection is stored in the Connector.
 * \param t_lastspike Time point of last spike emitted
 */
inline
  void STDPDopaConnection::send(Event& e, double_t t_lastspike, const STDPDopaCommonProperties &cp)
{
  
  // the event needs to be a spike event to be able to 
  // set the multiplicity. See comment below, where multiplicity is set.
  //  SpikeEvent& se=(SpikeEvent&) e;
   
  double_t dendritic_delay = Time(Time::step(delay_)).get_ms();
  double_t t_spike = e.get_stamp().get_ms();
  const vector<spikecounter>& dopa_spikes = cp.vt_->deliver_spikes();
    
  //get spike history of postsynaptic neuron in range (t1,t2]
  std::deque<histentry>::iterator start;
  std::deque<histentry>::iterator finish;
  target_->get_history(last_update_ - dendritic_delay, t_spike - dendritic_delay, &start, &finish);
  for (uint_t i=0; i<dopa_spikes.size(); i++)
    {
      if((dopa_spikes[i].spike_time_ >= last_update_)&&(dopa_spikes[i].spike_time_<t_spike)) 
	{
	  double_t this_dopa_spike = dopa_spikes[i].spike_time_;
	  
	  while((start != finish)&&(start->t_+dendritic_delay <= this_dopa_spike))
	    {
	      update_weight(start->t_ + dendritic_delay, cp);
	      eligibility_= eligibility_*std::exp((last_e_update_ - (start->t_+dendritic_delay))/cp.tau_e_)+cp.A_plus_*std::exp((t_lastspike - (start->t_+dendritic_delay))/cp.tau_plus_);
	      last_e_update_ = start->t_ + dendritic_delay;
	      last_post_spike_ = start->t_;
	      start++;
	    }
	  update_weight(this_dopa_spike, cp);
	   
	  dopa_trace_ = dopa_trace_*std::exp((last_dopa_spike_-this_dopa_spike)/cp.tau_d_)+dopa_spikes[i].multiplicity_/cp.tau_d_;
	  last_dopa_spike_ = this_dopa_spike;
	}
    }
  
  while((start != finish)&&(start->t_ <= t_spike))
    {
      update_weight(start->t_ + dendritic_delay, cp);
      eligibility_= eligibility_*std::exp((last_e_update_ - (start->t_+dendritic_delay))/cp.tau_e_)+cp.A_plus_*std::exp((t_lastspike - (start->t_+dendritic_delay))/cp.tau_plus_);
      last_e_update_ = start->t_ + dendritic_delay;
      last_post_spike_ = start->t_;
      start++;
    }
  update_weight(t_spike, cp);
  eligibility_= eligibility_*std::exp((last_e_update_-t_spike)/cp.tau_e_)-cp.A_minus_*std::exp(((last_post_spike_ + dendritic_delay) - t_spike)/cp.tau_minus_);
  last_e_update_ = t_spike;
  last_spike_ = t_spike; //we need last post spike for trigger_update_weight
  
  e.set_receiver(*target_);
  e.set_weight(weight_);
  e.set_delay(delay_);
  e.set_rport(rport_);
  e();

}
 

} // of namespace nest

#endif // of #ifndef STDPDopa_CONNECTION_H
