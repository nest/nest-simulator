/*
 *  test_target_fields.h
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

#ifndef TEST_TARGET_FIELDS_H
#define TEST_TARGET_FIELDS_H

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

// C++ includes:
#include <cmath>

// Includes from nestkernel:
#include "nest_types.h"
#include "target.h"

namespace nest
{

BOOST_AUTO_TEST_SUITE( test_target_fields )

/**
 * Tests whether values stored in Target objects via bitmasks are read
 * correctly.
 */
BOOST_AUTO_TEST_CASE( test_write_read )
{
  Target target;

  for ( size_t trial = 0; trial < 50; ++trial )
  {
    const thread tid = std::rand() % 1024;
    const thread rank = std::rand() % 1048576;
    const synindex syn_id = std::rand() % 64;
    const index lcid = std::rand() % 134217728;
    const bool is_processed = static_cast< bool >( std::rand() % 2 );

    target.set_tid( tid );
    target.set_rank( rank );
    target.set_syn_id( syn_id );
    target.set_lcid( lcid );
    target.set_is_processed( is_processed );

    BOOST_REQUIRE( target.get_tid() == tid );
    BOOST_REQUIRE( target.get_rank() == rank );
    BOOST_REQUIRE( target.get_syn_id() == syn_id );
    BOOST_REQUIRE( target.get_lcid() == lcid );
    BOOST_REQUIRE( target.is_processed() == is_processed );
  }
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace nest

#endif /* TEST_TARGET_FIELDS_H */
