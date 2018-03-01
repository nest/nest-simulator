/*
 *  string_utils.h
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

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <algorithm>
#include <string>

/**
 * This header is supposed to group string related utility functions.
 */

/**
 * Returns true, if the string `value` ends with the string `ending`.
 */
inline bool
ends_with( std::string const& value, std::string const& ending )
{
  if ( ending.size() > value.size() )
  {
    return false;
  }
  return std::equal( ending.rbegin(), ending.rend(), value.rbegin() );
}

#endif /* STRING_UTILS_H */
