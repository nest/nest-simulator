/*
 *  connection_manager.h
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

#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include "dictdatum.h"


namespace nest
{



/**
 * Manages the available connection prototypes and connections. It provides
 * the interface to establish and modify connections between nodes.
 */
class ConnectionManager
{

public:
  ConnectionManager();
  ~ConnectionManager();

  void init();
  void reset();

  /**
   * Add ConnectionManager specific stuff to the root status dictionary
   */
  void get_status( DictionaryDatum& d ) const;

  
private:


  void init_();




};

} // namespace


#endif /* #ifndef CONNECTION_MANAGER_H */
