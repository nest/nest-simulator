/*
 *  static_assert.h
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

#ifndef STATIC_ASSERT
#define STATIC_ASSERT

namespace nest
{

/**
 * Compile time assertions.
 * Usage:
 * \code
 * static const StaticAssert<bool-test>::success unique-var-name;
 * \endcode
 *
 * or
 *
 * \code
 * typedef StaticAssert<bool-test>::sucess unique-type-name;
 * \endcode
 *
 * Allows compilation if the bool-test is true. If bool-test is false,
 * fails to compile because success is not defined.
 */
template < bool >
struct StaticAssert
{
};

template <>
struct StaticAssert< true >
{
  struct success
  {
  };
};
}

#endif // STATIC_ASSERT
