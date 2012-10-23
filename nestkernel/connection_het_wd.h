/*
 *  connection_het_wd.h
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

#ifndef CONNECTION_HET_WD_H
#define CONNECTION_HET_WD_H

#include "connection.h"

namespace nest
{

/**
 * Class representing a variable connection that has the properties
 * weight and delay in addition to the inherited target and receiver port.
 * This class serves as the base class for dynamic synapses
 * (like TsodyksConnection, STDPConnection).
 * A suitale Connector containing these connections
 * can be obtained from the template GenericConnector.
 */
class ConnectionHetWD : public Connection
{
  public:

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  ConnectionHetWD();

  /**
   * Copy Constructor.
   */
  ConnectionHetWD(const ConnectionHetWD& c);

  /**
   * Get all properties of this connection and put them into a dictionary.
   */
  void get_status(DictionaryDatum & d) const;

  /**
   * Set properties of this connection from the values given in dictionary.
   */
  void set_status(const DictionaryDatum & d, ConnectorModel& cm);

  /**
   * Set properties of this connection from position p in the properties
   * array given in dictionary.
   */
  void set_status(const DictionaryDatum & d, index p, ConnectorModel& cm);

  /**
   * Create new empty arrays for the properties of this connection in the given
   * dictionary. It is assumed that they are not existing before.
   */
  void initialize_property_arrays(DictionaryDatum & d) const;

  /**
   * Append properties of this connection to the given dictionary. It is
   * assumed that the arrays were created by initialize_property_arrays()
   */
  void append_properties(DictionaryDatum & d) const;

  /**
   * Send an event to the receiver of this connection.
   * \param e The event to send
   * \param t_lastspike The time of the last spike.
   * \param cp The common property object containing properties which are the same for all synpses.
   */
  void send(Event& e, double_t t_lastspike, const CommonSynapseProperties &cp);

  /**
   * Calibrate the delay of this connection to the desired resolution.
   */
  void calibrate(const TimeConverter &);

  /**
   * Return the delay of the connection
   */
  double_t get_delay() const;

  /**
   * Set the delay of the connection
   */
  void set_delay(const double_t);

  /**
   * Set the weight of the connection
   */
  void set_weight(const double_t);

  protected:

  double_t weight_;    //!< Synaptic weight of this connection
  long_t delay_;       //!< Delay in timesteps of this connection

};

inline
double_t ConnectionHetWD::get_delay() const
{
  return Time(Time::step(delay_)).get_ms();
}

inline
void ConnectionHetWD::set_delay(const double_t delay)
{
  delay_ = Time(Time::ms(delay)).get_steps();
}

inline
void ConnectionHetWD::set_weight(const double_t weight)
{
  weight_ = weight;
}

inline
void ConnectionHetWD::send(Event& e, double_t, const CommonSynapseProperties &)
{
  e.set_weight(weight_);
  e.set_delay(delay_);
  e.set_receiver(*target_);
  e.set_rport(rport_);
  e();
}

inline
void ConnectionHetWD::calibrate(const TimeConverter &tc)
{
  Time t = tc.from_old_steps(delay_);
  delay_ = t.get_steps();

  if (delay_ == 0)
    delay_ = 1;
}

} // namespace

#endif // CONNECTION_HET_WD_H
