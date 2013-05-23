/*
 *  connector_model.h
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

/*
 * first version
 * date: march 2006
 * author: Moritz Helias
 */

#ifndef CONNECTORMODEL_H
#define CONNECTORMODEL_H

#include "nest_time.h"
#include "nest_timeconverter.h"

namespace nest
{
  class Connector;

/**
 * Defines abstract base class for ConnectorModel.
 * These virtual functions constitute the interface between ConnectionManager
 * and the ConnectorModel.
 * The interface between ConnectModel and Connector does not enter here,
 * since by using templates both of them have fixed type.
 */
class ConnectorModel
{

 public:
  ConnectorModel(Network& net, std::string);
  ConnectorModel(const ConnectorModel &, std::string);
  virtual ~ConnectorModel() {}

  virtual ConnectorModel* clone(std::string) const = 0;
  virtual void get_status(DictionaryDatum& d) const = 0;
  virtual void set_status(const DictionaryDatum& d) = 0;
  virtual Connector* get_connector() = 0;
  virtual void calibrate(const TimeConverter &) = 0;
  virtual void reset() = 0;

  const Time get_min_delay() const;
  const Time get_max_delay() const;

  void set_min_delay(const Time &min_delay);
  void set_max_delay(const Time &max_delay);

  Network & network() const;

  /**
   * Update min_delay and max_delay based on the delay given as argument
   * \param delay The delay to update min_delay and max_delay
   */
  void update_delay_extrema(const double_t mindelay, const double_t maxdelay);

  /**
   * Check, if delay is in agreement with min_delay, max_delay and resolution.
   */
  bool check_delay(double_t new_delay);
  bool check_delays(double_t delay1, double_t delay2);

  void set_num_connections(size_t);
  size_t get_num_connections() const;

  size_t get_num_connectors() const;

  std::string get_name() const;

  bool get_user_set_delay_extrema() const;

 protected:
  Network &net_;                    //!< The Network instance.
  Time min_delay_;                  //!< Minimal delay of all created synapses.
  Time max_delay_;                  //!< Maximal delay of all created synapses.
  size_t num_connections_;          //!< The number of connections registered with this type
  size_t num_connectors_;           //!< The number of connectors registered with this type
  bool default_delay_needs_check_;  //!< Flag indicating, that the default delay must be checked
  bool user_set_delay_extrema_;     //!< Flag indicating if the user set the delay extrema.

 private:
  std::string name_;
};


inline
Network& ConnectorModel::network() const
{
  return net_;
}

inline 
const Time ConnectorModel::get_min_delay() const
{
  return min_delay_;
}

inline
const Time ConnectorModel::get_max_delay() const
{
  return max_delay_;
}

inline 
void ConnectorModel::set_min_delay(const Time &min_delay)
{
  min_delay_ = min_delay;
}

inline 
void ConnectorModel::set_max_delay(const Time &max_delay)
{
  max_delay_ = max_delay;
}

inline
void ConnectorModel::set_num_connections(size_t num_connections)
{
  num_connections_ = num_connections;
}

inline
size_t ConnectorModel::get_num_connectors() const
{
  return num_connectors_;
}

inline
std::string ConnectorModel::get_name() const
{
  return name_;
}

inline
bool ConnectorModel::get_user_set_delay_extrema() const
{
  return user_set_delay_extrema_;
}


} // namespace

#endif /* #ifndef CONNECTORMODEL_H */
