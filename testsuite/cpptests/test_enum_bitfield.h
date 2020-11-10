/*
 *  test_enum_bitfield.h
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

#ifndef TEST_ENUM_BITFIELD_H
#define TEST_ENUM_BITFIELD_H

// Includes from libnestutil:
#include "enum_bitfield.h"

namespace nest
{

enum class MyFlags : unsigned int
{
  FIRST_FLAG = 1 << 0,
  SECOND_FLAG = 1 << 1,
  THIRD_FLAG = 1 << 2,
  FOURTH_FLAG = 1 << 3
};
}

template <>
struct nest::EnableBitMaskOperators< nest::MyFlags >
{
  static const bool enable = true;
};

namespace nest
{

BOOST_AUTO_TEST_SUITE( test_enum_bitfield )

BOOST_AUTO_TEST_CASE( test_enum_bitfield_ops )
{
  MyFlags my_flags = MyFlags::FIRST_FLAG | MyFlags::FOURTH_FLAG;

  BOOST_REQUIRE( enumFlagSet( my_flags, MyFlags::FIRST_FLAG ) );
  BOOST_REQUIRE( not enumFlagSet( my_flags, MyFlags::SECOND_FLAG ) );
  BOOST_REQUIRE( not enumFlagSet( my_flags, MyFlags::THIRD_FLAG ) );
  BOOST_REQUIRE( enumFlagSet( my_flags, MyFlags::FOURTH_FLAG ) );

  my_flags ^= MyFlags::FIRST_FLAG;
  my_flags ^= MyFlags::SECOND_FLAG;
  my_flags ^= MyFlags::THIRD_FLAG;
  my_flags ^= MyFlags::FOURTH_FLAG;

  BOOST_REQUIRE( not enumFlagSet( my_flags, MyFlags::FIRST_FLAG ) );
  BOOST_REQUIRE( enumFlagSet( my_flags, MyFlags::SECOND_FLAG ) );
  BOOST_REQUIRE( enumFlagSet( my_flags, MyFlags::THIRD_FLAG ) );
  BOOST_REQUIRE( not enumFlagSet( my_flags, MyFlags::FOURTH_FLAG ) );

  my_flags |= MyFlags::FIRST_FLAG;
  my_flags |= MyFlags::FOURTH_FLAG;

  BOOST_REQUIRE( enumFlagSet( my_flags, MyFlags::FIRST_FLAG ) );
  BOOST_REQUIRE( enumFlagSet( my_flags, MyFlags::SECOND_FLAG ) );
  BOOST_REQUIRE( enumFlagSet( my_flags, MyFlags::THIRD_FLAG ) );
  BOOST_REQUIRE( enumFlagSet( my_flags, MyFlags::FOURTH_FLAG ) );

  my_flags &= MyFlags::FIRST_FLAG | MyFlags::SECOND_FLAG;

  BOOST_REQUIRE( enumFlagSet( my_flags, MyFlags::FIRST_FLAG ) );
  BOOST_REQUIRE( enumFlagSet( my_flags, MyFlags::SECOND_FLAG ) );
  BOOST_REQUIRE( not enumFlagSet( my_flags, MyFlags::THIRD_FLAG ) );
  BOOST_REQUIRE( not enumFlagSet( my_flags, MyFlags::FOURTH_FLAG ) );

  my_flags = MyFlags::FIRST_FLAG;

  BOOST_REQUIRE( enumFlagSet( my_flags, MyFlags::FIRST_FLAG ) );
  BOOST_REQUIRE( not enumFlagSet( my_flags, MyFlags::SECOND_FLAG ) );
  BOOST_REQUIRE( not enumFlagSet( my_flags, MyFlags::THIRD_FLAG ) );
  BOOST_REQUIRE( not enumFlagSet( my_flags, MyFlags::FOURTH_FLAG ) );

  BOOST_REQUIRE( not enumFlagSet( my_flags ^ MyFlags::FIRST_FLAG, MyFlags::FIRST_FLAG ) );
  BOOST_REQUIRE( enumFlagSet( my_flags ^ MyFlags::SECOND_FLAG, MyFlags::SECOND_FLAG ) );

  BOOST_REQUIRE( enumFlagSet( my_flags | MyFlags::FIRST_FLAG, MyFlags::FIRST_FLAG ) );
  BOOST_REQUIRE( enumFlagSet( my_flags | MyFlags::SECOND_FLAG, MyFlags::SECOND_FLAG ) );

  BOOST_REQUIRE( enumFlagSet( my_flags & MyFlags::FIRST_FLAG, MyFlags::FIRST_FLAG ) );
  BOOST_REQUIRE( not enumFlagSet( my_flags & MyFlags::SECOND_FLAG, MyFlags::FIRST_FLAG ) );
}

BOOST_AUTO_TEST_SUITE_END()
}

#endif /* TEST_SORT_H */
