
/*
 *  connector_model.cpp
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
 * date: april 2007
 * author: Moritz Helias, Jochen Martin Eppler
 */

#include "network.h"
#include "connector_model.h"


namespace nest
{

ConnectorModel::ConnectorModel(Network &net, std::string name) :
    net_(net),
    min_delay_(Time::pos_inf()),
    max_delay_(Time::neg_inf()),
    num_connections_(0),
    default_delay_needs_check_(true),
    user_set_delay_extrema_(false),
    name_(name)
{ }

ConnectorModel::ConnectorModel(const ConnectorModel &cm, std::string name) :
      net_(cm.net_),
      min_delay_(cm.min_delay_),
      max_delay_(cm.max_delay_),
      num_connections_(0),
      num_connectors_(0),
      default_delay_needs_check_(true),
      user_set_delay_extrema_(cm.user_set_delay_extrema_),
      name_(name)
{
  min_delay_.calibrate();  // in case of change in resolution
  max_delay_.calibrate();
}

bool ConnectorModel::check_delay(double_t new_delay)
{

  if (new_delay < Time::get_resolution().get_ms())
  {
    net_.message(SLIInterpreter::M_ERROR, "check_delay()", "Delay must be greater than or equal to resolution");
    return false;
  }

  // if already simulated, the new delay has to be checked against the
  // min_delay and the max_delay which have been used during simulation
  if (net_.get_simulated())
  {
    Time sim_min_delay = Time::step(net_.get_min_delay());
    Time sim_max_delay = Time::step(net_.get_max_delay());
    bool bad_min_delay = new_delay < sim_min_delay.get_ms();
    bool bad_max_delay = new_delay > sim_max_delay.get_ms();
 
    if (bad_min_delay || bad_max_delay)
      {
	net_.message(SLIInterpreter::M_ERROR, "check_delay()", "Delay cannot be set: Simulate has been called already");
	return false;
      }
  }
  
  bool bad_min_delay = new_delay < min_delay_.get_ms();
  bool bad_max_delay = new_delay > max_delay_.get_ms();

  if (!bad_min_delay && !bad_max_delay)
    return true;

  if (user_set_delay_extrema_)
    {
      if (bad_min_delay)
	net_.message(SLIInterpreter::M_ERROR, "check_delay()", "Delay must be greater than or equal to min_delay");
      if (bad_max_delay)
	net_.message(SLIInterpreter::M_ERROR, "check_delay()", "Delay must be smaller than or equal to max_delay");
      return false;    
    }

  if (bad_min_delay)
    update_delay_extrema(new_delay, max_delay_.get_ms());

  if (bad_max_delay)
    update_delay_extrema(min_delay_.get_ms(), new_delay);

  return true;
}

bool ConnectorModel::check_delays(double_t new_delay1, double_t new_delay2)
{
  double_t ldelay = new_delay1 < new_delay2 ? new_delay1 : new_delay2;
  double_t hdelay = new_delay1 < new_delay2 ? new_delay2 : new_delay1;

  if (ldelay < Time::get_resolution().get_ms())
  {
    net_.message(SLIInterpreter::M_ERROR, "check_delay()", "Delay must be greater than or equal to resolution");
    return false;
  }
 
  if (net_.get_simulated())
  {
    Time sim_min_delay = Time::step(net_.get_min_delay());
    Time sim_max_delay = Time::step(net_.get_max_delay());
    bool bad_min_delay = ldelay < sim_min_delay.get_ms();
    bool bad_max_delay = hdelay > sim_max_delay.get_ms();

    if (bad_min_delay || bad_max_delay)
      {
	net_.message(SLIInterpreter::M_ERROR, "check_delay()", "Delay cannot be set: Simulate has been called already");
	return false;
      }
  }

  bool bad_min_delay = ldelay < min_delay_.get_ms();
  bool bad_max_delay = hdelay > max_delay_.get_ms();

  if (!bad_min_delay && !bad_max_delay)
    return true;
  
  if (user_set_delay_extrema_)
    {
      if (bad_min_delay)
	net_.message(SLIInterpreter::M_ERROR, "check_delay()", "Delay must be greater than or equal to min_delay");
      if (bad_max_delay)
	net_.message(SLIInterpreter::M_ERROR, "check_delay()", "Delay must be smaller than or equal to max_delay");
      return false;    
    }

  if (bad_min_delay)
    update_delay_extrema(ldelay, max_delay_.get_ms());
  
  if (bad_max_delay)
    update_delay_extrema(min_delay_.get_ms(), hdelay);

  return true;

}

void ConnectorModel::update_delay_extrema(const double_t mindelay_cand, 
					  const double_t maxdelay_cand )
{ 
  if (mindelay_cand < min_delay_.get_ms())
    min_delay_ = Time(Time::ms(mindelay_cand));
  
  if (maxdelay_cand > max_delay_.get_ms())
    max_delay_ = Time(Time::ms(maxdelay_cand));
}

size_t ConnectorModel::get_num_connections() const
{
  net_.count_connections();
  return num_connections_;
}

} // namespace nest
