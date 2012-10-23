
/*
 *  cont_delay_connection.cpp
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

  ContDelayConnection::ContDelayConnection() :
    ConnectionHetWD(),
    delay_offset_(0.0)
  { }


  ContDelayConnection::ContDelayConnection(const ContDelayConnection &rhs) :
    ConnectionHetWD(rhs)
  {
    delay_offset_ = rhs.delay_offset_;
  }

  void ContDelayConnection::get_status(DictionaryDatum & d) const
  {
    def<double_t>(d, names::weight, weight_);
    def<double_t>(d, names::delay, Time(Time::step(delay_)).get_ms()-delay_offset_);

    if (target_ != 0)
      {
	def<long>(d, names::rport, rport_);
	def<long>(d, names::target, target_->get_gid());
      }
    
  }
  
  void ContDelayConnection::set_status(const DictionaryDatum & d, ConnectorModel &cm)
  {
     //set delay if mentioned
    double_t delay;
    if (updateValue<double_t>(d, names::delay, delay))
      {

	double int_delay;     
	long_t lowerbound; 
	double_t h = Time::get_resolution().get_ms();
	double_t frac_delay = std::modf(delay/h, &int_delay);
	if (frac_delay == 0)
	  {
	    if (!cm.check_delay(delay))
	      throw BadDelay(delay);
	    delay_ = Time(Time::ms(delay)).get_steps();
	    delay_offset_ = 0.0;
	  }
	else
	  {
	    lowerbound = (long_t)(int_delay);
	    if (!cm.check_delays(Time(Time::step(lowerbound)).get_ms(),
				 Time(Time::step(lowerbound + 1)).get_ms()))
	      throw BadDelay(lowerbound);
	    delay_ = lowerbound + 1; 
	    delay_offset_ = h * (1.0 - frac_delay);
	  }
      }
    updateValue<double_t>(d, names::weight, weight_);
  }

   /**
   * Set properties of this connection from position p in the properties
   * array given in dictionary.
   */  
  void ContDelayConnection::set_status(const DictionaryDatum & d, index p, ConnectorModel &cm)
  {

     //set delay if mentioned
    double_t delay;
    if ( set_property<double_t>(d, names::delays, p, delay) )
      {
	double int_delay;     
	long_t lowerbound; 
	double_t h = Time::get_resolution().get_ms();
	double_t frac_delay = std::modf(delay/h, &int_delay);
	if (frac_delay == 0)
	  {
	    if (!cm.check_delay(delay))
	      throw BadDelay(delay);
	    delay_ = Time(Time::ms(delay)).get_steps();
	    delay_offset_ = 0.0;
	  }
	else
	  {
	    lowerbound = (long_t)(int_delay);
	    if (!cm.check_delays(Time(Time::step(lowerbound)).get_ms(),
				 Time(Time::step(lowerbound + 1)).get_ms()))
	      throw BadDelay(lowerbound);
	    delay_ = lowerbound + 1; 
	    delay_offset_ = h * (1.0 - frac_delay);
	  }
      }
    set_property<double_t>(d, names::weights, p, weight_); 
  }
 
  /**
   * Append properties of this connection to the given dictionary. If the
   * dictionary is empty, new arrays are created first.
   */
  void ContDelayConnection::append_properties(DictionaryDatum & d) const
  {
    append_property<index>(d, names::targets, target_->get_gid()); 
    append_property<double_t>(d, names::weights, weight_); 
    append_property<double_t>(d, names::delays, Time(Time::step(delay_)).get_ms()-delay_offset_); 
    append_property<long_t>(d, names::rports, rport_); 
  }

} // of namespace nest
