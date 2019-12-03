/*
 *  test_streamers.h
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

#ifndef TEST_STEAMERS_H
#define TEST_STEAMERS_H

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

// C++ includes:
#include <vector>
#include <sstream>

// Includes from libnestutil:
#include "streamers.h"

namespace nest
{

BOOST_AUTO_TEST_SUITE( test_streamers )

/**
 * Tests whether an example array with some numbers is streamed
 * correctly by a single call to the operator<<().
 */
BOOST_AUTO_TEST_CASE( test_int )
{
  std::ostringstream s;
  const std::vector< int > x = { 2, 3, 4, 5 };

  s << x;

  BOOST_REQUIRE( s.str() == "vector[2, 3, 4, 5]" );
}

BOOST_AUTO_TEST_SUITE_END()

} // of namespace nest

#endif /* TEST_STEAMERS_H */
