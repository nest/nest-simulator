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

/* BeginDocumentation

   Name: stdp_dopamine_synapse - Synapse type for dopamine-modulated spike-timing dependent plasticity.

   Description:
   stdp_dopamine_synapse is a connection to create synapses with
   dopamine-modulated spike-timing dependent plasticity (used as a
   benchmark model in [1], based on [2]). The dopaminergic signal is a
   low-pass filtered version of the spike rate of a user-specific pool
   of neurons. The spikes emitted by the pool of dopamine neurons are
   delivered to the synapse via the assigned volume transmitter. The
   dopaminergic dynamics is calculated in the synapse itself.

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
   tau_c     double - Time constant of eligibility trace in ms
   tau_n     double - Time constant of dopaminergic trace in ms
   b         double - Dopaminergic baseline concentration
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

   Author: Susanne Kunkel
   Remarks:
   - based on an earlier version by Wiebke Potjans
   - major changes to code after code revision in Apr 2013

   SeeAlso: volume_transmitter
*/

#include "connection_het_wd.h"
#include "archiving_node.h"
#include "volume_transmitter.h"
#include "spikecounter.h"

#include "numerics.h"

namespace nest
{

  /**
   * Class containing the common properties for all synapses of type dopamine connection.
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
    void get_status(DictionaryDatum& d) const;

    /**
     * Set properties from the values given in dictionary.
     */
    void set_status(const DictionaryDatum& d, ConnectorModel& cm);

    // overloaded for all supported event types
    void check_event(SpikeEvent&) {}

    Node* get_node();

  private:

    volume_transmitter* vt_;
    double_t A_plus_;
    double_t A_minus_;
    double_t tau_plus_;
    double_t tau_c_;
    double_t tau_n_;
    double_t b_;
    double_t Wmin_;
    double_t Wmax_;
  };


  /**
   * Class representing an STDPDopaConnection with homogeneous parameters,
   * i.e. parameters are the same for all synapses.
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
    STDPDopaConnection(const STDPDopaConnection&);

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
    void check_connection(Node& s, Node& r, rport receptor_type, double_t t_lastspike);

    /**
     * Get all properties of this connection and put them into a dictionary.
     */
    void get_status(DictionaryDatum& d) const;

    /**
     * Set properties of this connection from the values given in dictionary.
     */
    void set_status(const DictionaryDatum& d, ConnectorModel& cm);

    /**
     * Set properties of this connection from position p in the properties
     * array given in dictionary.
     */
    void set_status(const DictionaryDatum& d, index p, ConnectorModel& cm);

    /**
     * Create new empty arrays for the properties of this connection in the given
     * dictionary. It is assumed that they are not existing before.
     */
    void initialize_property_arrays(DictionaryDatum& d) const;

    /**
     * Append properties of this connection to the given dictionary. If the
     * dictionary is empty, new arrays are created first.
     */
    void append_properties(DictionaryDatum& d) const;

    // overloaded for all supported event types
    void check_event(SpikeEvent&) {}

    /**
     * Send an event to the receiver of this connection.
     * \param e The event to send
     */
    void send(Event& e, double_t, const STDPDopaCommonProperties& cp);

    void trigger_update_weight(const vector<spikecounter>& dopa_spikes, double_t t_trig, const STDPDopaCommonProperties& cp);

  private:

    // update dopamine trace from last to current dopamine spike and increment index
    void update_dopamine_(const vector<spikecounter>& dopa_spikes, const STDPDopaCommonProperties& cp);

    void update_weight_(double_t c0, double_t n0, double_t minus_dt, const STDPDopaCommonProperties& cp);

    void process_dopa_spikes_(const vector<spikecounter>& dopa_spikes, double_t t0, double_t t1, const STDPDopaCommonProperties& cp);
    void facilitate_(double_t kplus, const STDPDopaCommonProperties& cp);
    void depress_(double_t kminus, const STDPDopaCommonProperties& cp);

    double_t Kplus_;
    double_t c_;
    double_t n_;

    // dopa_spikes_idx_ refers to the dopamine spike that has just been processes
    // after trigger_update_weight a pseudo dopamine spike at t_trig is stored at index 0 and dopa_spike_idx_ = 0
    index dopa_spikes_idx_;

    // time of last update, which is either time of last presyn. spike or time-driven update
    double_t t_last_update_;
  };


  inline
  void STDPDopaConnection::update_dopamine_(const vector<spikecounter>& dopa_spikes, const STDPDopaCommonProperties& cp)
  {
    double_t minus_dt = dopa_spikes[dopa_spikes_idx_].spike_time_ - dopa_spikes[dopa_spikes_idx_+1].spike_time_;
    ++dopa_spikes_idx_;
    n_ = n_ * std::exp( minus_dt / cp.tau_n_ ) + dopa_spikes[dopa_spikes_idx_].multiplicity_ / cp.tau_n_;
  }

  inline
  void STDPDopaConnection::update_weight_(double_t c0, double_t n0, double_t minus_dt, const STDPDopaCommonProperties& cp)
  {
    double_t taus_ = ( cp.tau_c_ + cp.tau_n_ ) / ( cp.tau_c_ * cp.tau_n_ );
    weight_ = weight_ - c0 * ( n0 / taus_ * numerics::expm1( taus_ * minus_dt )
			     - cp.b_ * cp.tau_c_ * numerics::expm1( minus_dt / cp.tau_c_ ) );
    if ( weight_ < cp.Wmin_ )
      weight_ = cp.Wmin_;
    if ( weight_ > cp.Wmax_ )
      weight_ = cp.Wmax_;
  }

  inline
  void STDPDopaConnection::process_dopa_spikes_(const vector<spikecounter>& dopa_spikes,
                                                double_t t0, double_t t1, const STDPDopaCommonProperties& cp)
  {
    // process dopa spikes in (t0, t1]
    // propagate weight from t0 to t1
    if ( ( dopa_spikes.size() > dopa_spikes_idx_+1 ) && ( dopa_spikes[dopa_spikes_idx_+1].spike_time_ <= t1 ) )
    {
      // there is at least 1 dopa spike in (t0, t1]
      // propagate weight up to first dopa spike and update dopamine trace
      // weight and eligibility c are at time t0 but dopamine trace n is at time of last dopa spike
      double_t n0 = n_ * std::exp( ( dopa_spikes[dopa_spikes_idx_].spike_time_ - t0 ) / cp.tau_n_ );  // dopamine trace n at time t0
      update_weight_(c_, n0, t0 - dopa_spikes[dopa_spikes_idx_+1].spike_time_, cp);
      update_dopamine_(dopa_spikes, cp);

      // process remaining dopa spikes in (t0, t1]
      double_t cd;
      while ( ( dopa_spikes.size() > dopa_spikes_idx_+1 ) && ( dopa_spikes[dopa_spikes_idx_+1].spike_time_ <= t1 ) )
      {
	// propagate weight up to next dopa spike and update dopamine trace
	// weight and dopamine trace n are at time of last dopa spike td but eligibility c is at time t0
	cd = c_ * std::exp( ( t0 - dopa_spikes[dopa_spikes_idx_].spike_time_ ) / cp.tau_c_ );  // eligibility c at time of td
	update_weight_(cd, n_, dopa_spikes[dopa_spikes_idx_].spike_time_ - dopa_spikes[dopa_spikes_idx_+1].spike_time_, cp);
	update_dopamine_(dopa_spikes, cp);
      }

      // propagate weight up to t1
      // weight and dopamine trace n are at time of last dopa spike td but eligibility c is at time t0
      cd = c_ * std::exp( ( t0 - dopa_spikes[dopa_spikes_idx_].spike_time_ ) / cp.tau_c_ );  // eligibility c at time td
      update_weight_(cd, n_, dopa_spikes[dopa_spikes_idx_].spike_time_ - t1, cp);
    }
    else
    {
      // no dopamine spikes in (t0, t1]
      // weight and eligibility c are at time t0 but dopamine trace n is at time of last dopa spike
      double_t n0 = n_ * std::exp( ( dopa_spikes[dopa_spikes_idx_].spike_time_ - t0 ) / cp.tau_n_ );  // dopamine trace n at time t0
      update_weight_(c_, n0, t0 - t1, cp);
    }

    // update eligibility trace c for interval (t0, t1]
    c_ = c_ * std::exp( ( t0 - t1 ) / cp.tau_c_ );
  }

  inline
  void STDPDopaConnection::facilitate_(double_t kplus, const STDPDopaCommonProperties& cp)
  {
    c_ += cp.A_plus_ * kplus;
  }

  inline 
  void STDPDopaConnection::depress_(double_t kminus, const STDPDopaCommonProperties& cp)
  {
    c_ -= cp.A_minus_ * kminus;
  }

  inline
  void STDPDopaConnection::check_connection(Node& s, Node& r, rport receptor_type, double_t t_lastspike)
  {
    ConnectionHetWD::check_connection(s, r, receptor_type, t_lastspike);
    r.register_stdp_connection(t_lastspike - Time(Time::step(delay_)).get_ms());
  }

  inline
  void STDPDopaConnection::send(Event& e, double_t, const STDPDopaCommonProperties& cp)
  {
    // t_lastspike_ = 0 initially

    // purely dendritic delay
    double_t dendritic_delay = Time(Time::step(delay_)).get_ms();

    double_t t_spike = e.get_stamp().get_ms();

    // get history of dopamine spikes
    const vector<spikecounter>& dopa_spikes = cp.vt_->deliver_spikes();

    // get spike history in relevant range (t_last_update, t_spike] from post-synaptic neuron
    std::deque<histentry>::iterator start;
    std::deque<histentry>::iterator finish;
    target_->get_history(t_last_update_ - dendritic_delay, t_spike - dendritic_delay, &start, &finish);

    // facilitation due to post-synaptic spikes since last update
    double_t t0 = t_last_update_;
    double_t minus_dt;
    while ( start != finish )
    {
      process_dopa_spikes_(dopa_spikes, t0, start->t_ + dendritic_delay, cp);
      t0 = start->t_ + dendritic_delay;
      minus_dt = t_last_update_ - t0;
      if ( start->t_ < t_spike )  // only depression if pre- and postsyn. spike occur at the same time
	facilitate_(Kplus_ * std::exp( minus_dt / cp.tau_plus_ ), cp);
      ++start;
    }

    // depression due to new pre-synaptic spike
    process_dopa_spikes_(dopa_spikes, t0, t_spike, cp);
    depress_(target_->get_K_value(t_spike - dendritic_delay), cp);

    e.set_receiver(*target_);
    e.set_weight(weight_);
    e.set_delay(delay_);
    e.set_rport(rport_);
    e();

    Kplus_ = Kplus_ * std::exp( ( t_last_update_ - t_spike ) / cp.tau_plus_) + 1.0;
    t_last_update_ = t_spike;
  }

  inline
  void STDPDopaConnection::trigger_update_weight(const vector<spikecounter>& dopa_spikes, const double_t t_trig,
						 const STDPDopaCommonProperties& cp)
  {
    // propagate all state variables to time t_trig
    // this does not include the depression trace K_minus, which is updated in the postsyn.neuron

    // purely dendritic delay
    double_t dendritic_delay = Time(Time::step(delay_)).get_ms();

    // get spike history in relevant range (t_last_update, t_trig] from postsyn. neuron
    std::deque<histentry>::iterator start;
    std::deque<histentry>::iterator finish;
    target_->get_history(t_last_update_ - dendritic_delay, t_trig - dendritic_delay, &start, &finish);

    // facilitation due to postsyn. spikes since last update
    double_t t0 = t_last_update_;
    double_t minus_dt;
    while ( start != finish )
    {
      process_dopa_spikes_(dopa_spikes, t0, start->t_ + dendritic_delay, cp);
      t0 = start->t_ + dendritic_delay;
      minus_dt = t_last_update_ - t0;
      facilitate_(Kplus_ * std::exp( minus_dt / cp.tau_plus_ ), cp);
      ++start;
    }
    
    // propagate weight, eligibility trace c, dopamine trace n and facilitation trace K_plus to time t_trig
    // but do increment/decrement as there are no spikes to be handled at t_trig
    process_dopa_spikes_(dopa_spikes, t0, t_trig, cp);
    n_ = n_ * std::exp( ( dopa_spikes[dopa_spikes_idx_].spike_time_ - t_trig ) / cp.tau_n_ );
    Kplus_ = Kplus_ * std::exp( ( t_last_update_ - t_trig ) / cp.tau_plus_);

    t_last_update_ = t_trig;
    dopa_spikes_idx_ = 0;
  }

} // of namespace nest

#endif // of #ifndef STDP_DOPA_CONNECTION_H
