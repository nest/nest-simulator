/*
 *  deprecation_warning.h
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

#ifndef DEPRECATION_WARNING_H
#define DEPRECATION_WARNING_H


namespace nest
{

class DeprecationWarning
{
public:
  DeprecationWarning()
    : deprecated_functions_(0)
  {
  }


private:
  std::map<std::string, bool> deprecated_functions_;
};

}



#endif /* DEPRECATION_WARNING_H */
