/*
 *  event_delivery_manager.h
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

#ifndef EVENT_DELIVERY_MANAGER_H
#define EVENT_DELIVERY_MANAGER_H

#include "manager_interface.h"

#include "dictdatum.h"

namespace nest
{

class EventDeliveryManager : public ManagerInterface
{
public:
  EventDeliveryManager();
  virtual ~EventDeliveryManager();

  virtual void init();
  virtual void reset();

  virtual void set_status( const DictionaryDatum& );
  virtual void get_status( DictionaryDatum& );
};
} // namespace nest

#endif /* EVENT_DELIVERY_MANAGER_H */
