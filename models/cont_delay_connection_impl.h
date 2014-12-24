/*
 *  cont_delay_connection_impl.h
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

#include "network.h"
#include "dictdatum.h"
#include "connector_model.h"
#include "common_synapse_properties.h"
#include "cont_delay_connection.h"
#include "event.h"


namespace nest
{

  template<typename targetidentifierT>
  ContDelayConnection<targetidentifierT>::ContDelayConnection() :
    ConnectionBase(),
    weight_(1.0),
    delay_offset_(0.0)
  { }

  template<typename targetidentifierT>
  ContDelayConnection<targetidentifierT>::ContDelayConnection(const ContDelayConnection &rhs) :
    ConnectionBase(rhs),
    weight_(rhs.weight_),
    delay_offset_(rhs.delay_offset_)
  { }

  template<typename targetidentifierT>
  void ContDelayConnection<targetidentifierT>::get_status(DictionaryDatum & d) const
  {
    ConnectionBase::get_status(d);

    def<double_t>(d, names::weight, weight_);
    def<double_t>(d, names::delay, Time(Time::step(get_delay_steps())).get_ms()-delay_offset_);     
  }

  template<typename targetidentifierT>
  void ContDelayConnection<targetidentifierT>::set_status(const DictionaryDatum & d, ConnectorModel &cm)
  {
    ConnectionBase::set_status(d, cm);

    updateValue<double_t>(d, names::weight, weight_);

    //set delay if mentioned
    double_t delay;

    if (updateValue<double_t>(d, names::delay, delay))
      {

	const double_t h = Time::get_resolution().get_ms();

	double_t int_delay;
	const double_t frac_delay = std::modf(delay/h, &int_delay);

	if (frac_delay == 0)
	  {
	    cm.assert_valid_delay_ms(delay);
	    set_delay_steps( Time::delay_ms_to_steps(delay) );
	    delay_offset_ = 0.0;
	  }
	else
	  {
	    const long_t lowerbound = static_cast<long_t>(int_delay);
	    cm.assert_two_valid_delays_steps(lowerbound, lowerbound + 1);
	    set_delay_steps(lowerbound + 1);
	    delay_offset_ = h * (1.0 - frac_delay);
	  }
      }
  }

} // of namespace nest
