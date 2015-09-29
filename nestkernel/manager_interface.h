/*
 *  manager_interface.h
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

#ifndef MANAGER_INTERFACE_H
#define MANAGER_INTERFACE_H

class Dictionary;

namespace nest
{

class ManagerInterface
{
private:
  ManagerInterface( ManagerInterface const& ); // Don't Implement
  void operator=( ManagerInterface const& );   // Don't Implement

public:
  ManagerInterface()
  {
  }
  ~ManagerInterface()
  {
  }
  virtual void init() = 0;
  virtual void reset() = 0;

  virtual void set_status( const Dictionary& ) = 0;
  virtual void get_status( Dictionary& ) = 0;
};
}

#endif /* MANAGER_INTERFACE_H */
