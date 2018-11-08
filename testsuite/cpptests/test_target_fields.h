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

BOOST_AUTO_TEST_SUITE( test_target_data_type_object )

/**
 * Tests whether values stored in a Target data type object are returned
 * correctly.
 */
BOOST_AUTO_TEST_CASE( test_write_read )
{
  Target target_id;

  BOOST_REQUIRE( sizeof(target_id) == 8U );

  srand(time(0));
  for ( int trial = 0; trial < 50; ++trial )
  {
    const thread tid = std::rand() % (Target::MAX_TID - 1);
    const thread rank = std::rand() % (Target::MAX_RANK - 1);
    const synindex syn_id = std::rand() % (Target::MAX_SYN_ID - 1);
    const index lcid = std::rand() % (Target::MAX_LCID - 1);

    enum_status_target_id status_target_id = TARGET_ID_UNPROCESSED;
    if( static_cast< bool >( std::rand() % 2 ) ) {
      status_target_id = TARGET_ID_PROCESSED;
    }

    target_id.set_tid( tid );
    target_id.set_rank( rank );
    target_id.set_syn_id( syn_id );
    target_id.set_lcid( lcid );
    target_id.set_status( status_target_id );

    BOOST_REQUIRE( target_id.get_tid() == tid );
    BOOST_REQUIRE( target_id.get_rank() == rank );
    BOOST_REQUIRE( target_id.get_syn_id() == syn_id );
    BOOST_REQUIRE( target_id.get_lcid() == lcid );
    BOOST_REQUIRE( target_id.get_status() == status_target_id );
  }
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace nest

#endif /* TEST_TARGET_FIELDS_H */
