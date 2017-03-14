/*
 *  quantal_stp_connection.h
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

#ifndef QUANTAL_STP_CONNECTION_H
#define QUANTAL_STP_CONNECTION_H

// Includes from librandom:
#include "binomial_randomdev.h"

// Includes from nestkernel:
#include "connection.h"

/* BeginDocumentation
  Name: quantal_stp_synapse - Probabilistic synapse model with short term
  plasticity.

  Description:

   This synapse model implements synaptic short-term depression and
   short-term facilitation according to the quantal release model
   described by Fuhrmann et al. [1] and Loebel et al. [2].

   Each presynaptic spike will stochastically activate a fraction of
   the available release sites.  This fraction is binomialy
   distributed and the release probability per site is governed by the
   Fuhrmann et al. (2002) model. The solution of the differential
   equations is taken from Maass and Markram 2002 [3].

   The connection weight is interpreted as the maximal weight that can
   be obtained if all n release sites are activated.

   Parameters:
     The following parameters can be set in the status dictionary:
     U          double - Maximal fraction of available resources [0,1],
                         default=0.5
     u          double - available fraction of resources [0,1], default=0.5
     p          double - probability that a vesicle is available, default = 1.0
     n          long   - total number of release sites, default = 1
     a          long   - number of available release sites, default = n
     tau_rec    double - time constant for depression in ms, default=800 ms
     tau_rec    double - time constant for facilitation in ms, default=0 (off)


  References:
   [1] Fuhrmann, G., Segev, I., Markram, H., & Tsodyks, M. V. (2002). Coding of
       temporal information by activity-dependent synapses. Journal of
       neurophysiology, 87(1), 140-8.
   [2] Loebel, A., Silberberg, G., Helbig, D., Markram, H., Tsodyks,
       M. V, & Richardson, M. J. E. (2009). Multiquantal release underlies
       the distribution of synaptic efficacies in the neocortex. Frontiers
       in computational neuroscience, 3(November), 27.
       doi:10.3389/neuro.10.027.2009
   [3] Maass, W., & Markram, H. (2002). Synapses as dynamic memory buffers.
       Neural networks, 15(2), 155-61.

  Transmits: SpikeEvent

  FirstVersion: December 2013
  Author: Marc-Oliver Gewaltig, based on tsodyks2_synapse
  SeeAlso: tsodyks2_synapse, synapsedict, stdp_synapse, static_synapse
*/


/**
 * Class representing a synapse with Tsodyks short term plasticity, based on the
 * iterative formula. A suitable Connector containing these connections can be
 * obtained from the template GenericConnector.
 */

namespace nest
{

template < typename targetidentifierT >
class Quantal_StpConnection : public Connection< targetidentifierT >
{
public:
  typedef CommonSynapseProperties CommonPropertiesType;
  typedef Connection< targetidentifierT > ConnectionBase;

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  Quantal_StpConnection();
  /**
   * Copy constructor to propagate common properties.
   */
  Quantal_StpConnection( const Quantal_StpConnection& );

  // Explicitly declare all methods inherited from the dependent base
  // ConnectionBase. This avoids explicit name prefixes in all places these
  // functions are used. Since ConnectionBase depends on the template parameter,
  // they are not automatically found in the base class.
  using ConnectionBase::get_delay_steps;
  using ConnectionBase::get_delay;
  using ConnectionBase::get_rport;
  using ConnectionBase::get_target;

  /**
   * Get all properties of this connection and put them into a dictionary.
   */
  void get_status( DictionaryDatum& d ) const;

  /**
   * Set default properties of this connection from the values given in
   * dictionary.
   */
  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  /**
   * Send an event to the receiver of this connection.
   * \param e The event to send
   * \param t_lastspike Point in time of last spike sent.
   * \param cp Common properties to all synapses (empty).
   */
  void send( Event& e,
    thread t,
    double t_lastspike,
    const CommonSynapseProperties& cp );

  class ConnTestDummyNode : public ConnTestDummyNodeBase
  {
  public:
    // Ensure proper overriding of overloaded virtual functions.
    // Return values from functions are ignored.
    using ConnTestDummyNodeBase::handles_test_event;
    port
    handles_test_event( SpikeEvent&, rport )
    {
      return invalid_port_;
    }
  };

  void
  check_connection( Node& s,
    Node& t,
    rport receptor_type,
    double,
    const CommonPropertiesType& )
  {
    ConnTestDummyNode dummy_target;
    ConnectionBase::check_connection_( dummy_target, s, t, receptor_type );
  }

  void
  set_weight( double w )
  {
    weight_ = w;
  }

private:
  double weight_;  //!< synaptic weight
  double U_;       //!< unit increment of a facilitating synapse (U)
  double u_;       //!< dynamic value of probability of release
  double tau_rec_; //!< [ms] time constant for recovery from depression (D)
  double tau_fac_; //!< [ms] time constant for facilitation (F)
  int n_;          //!< Number of release sites
  int a_;          //!< Number of available release sites
};


/**
 * Send an event to the receiver of this connection.
 * \param e The event to send
 * \param t The thread on which this connection is stored.
 * \param t_lastspike Time point of last spike emitted
 * \param cp Common properties object, containing the quantal_stp parameters.
 */
template < typename targetidentifierT >
inline void
Quantal_StpConnection< targetidentifierT >::send( Event& e,
  thread t,
  double t_lastspike,
  const CommonSynapseProperties& )
{
  const int vp = get_target( t )->get_vp();

  const double h = e.get_stamp().get_ms() - t_lastspike;

  // Compute the decay factors, based on the time since the last spike.
  const double p_decay = std::exp( -h / tau_rec_ );
  const double u_decay =
    ( tau_fac_ < 1.0e-10 ) ? 0.0 : std::exp( -h / tau_fac_ );

  // Compute release probability
  u_ = U_ + u_ * ( 1. - U_ ) * u_decay; // Eq. 4 from [2]

  // Compute number of sites that recovered during the interval.
  for ( int depleted = n_ - a_; depleted > 0; --depleted )
  {
    if ( kernel().rng_manager.get_rng( vp )->drand() < ( 1.0 - p_decay ) )
    {
      ++a_;
    }
  }

  // Compute number of released sites
  int n_release = 0;
  for ( int i = a_; i > 0; --i )
  {
    if ( kernel().rng_manager.get_rng( vp )->drand() < u_ )
    {
      ++n_release;
    }
  }

  if ( n_release > 0 )
  {
    e.set_receiver( *get_target( t ) );
    e.set_weight( n_release * weight_ );
    e.set_delay( get_delay_steps() );
    e.set_rport( get_rport() );
    e();
    a_ -= n_release;
  }
}

} // namespace

#endif // QUANTAL_STP_CONNECTION_H
