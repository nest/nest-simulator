/*
 *  enum_bitfield.h
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

#ifndef ENUM_BITFIELD_H
#define ENUM_BITFIELD_H

namespace nest
{

template < typename Enum >
struct EnableBitMaskOperators
{
  static const bool enable = false;
};

template < typename Enum >
typename std::enable_if< EnableBitMaskOperators< Enum >::enable, Enum >::type
operator|( Enum lhs, Enum rhs )
{
  using underlying = typename std::underlying_type< Enum >::type;
  return static_cast< Enum >(
    static_cast< underlying >( lhs ) | static_cast< underlying >( rhs ) );
}

template < typename Enum >
typename std::enable_if< EnableBitMaskOperators< Enum >::enable, Enum >::type
operator&( Enum lhs, Enum rhs )
{
  using underlying = typename std::underlying_type< Enum >::type;
  return static_cast< Enum >(
    static_cast< underlying >( lhs ) & static_cast< underlying >( rhs ) );
}

template < typename Enum >
typename std::enable_if< EnableBitMaskOperators< Enum >::enable, Enum >::type
operator^( Enum lhs, Enum rhs )
{
  using underlying = typename std::underlying_type< Enum >::type;
  return static_cast< Enum >(
    static_cast< underlying >( lhs ) ^ static_cast< underlying >( rhs ) );
}

template < typename Enum >
typename std::enable_if< EnableBitMaskOperators< Enum >::enable, Enum >::type
operator|=( Enum& lhs, Enum& rhs )
{
  using underlying = typename std::underlying_type< Enum >::type;
  return static_cast< Enum >(
    static_cast< underlying >( lhs ) | static_cast< underlying >( rhs ) );
}

template < typename Enum >
typename std::enable_if< EnableBitMaskOperators< Enum >::enable, Enum >::type
operator&=( Enum& lhs, Enum& rhs )
{
  using underlying = typename std::underlying_type< Enum >::type;
  return static_cast< Enum >(
    static_cast< underlying >( lhs ) & static_cast< underlying >( rhs ) );
}

template < typename Enum >
typename std::enable_if< EnableBitMaskOperators< Enum >::enable, Enum >::type
operator^=( Enum& lhs, Enum& rhs )
{
  using underlying = typename std::underlying_type< Enum >::type;
  return static_cast< Enum >(
    static_cast< underlying >( lhs ) ^ static_cast< underlying >( rhs ) );
}

template < typename Enum >
bool
enumFlagSet( const Enum en, const Enum flag )
{
  using underlying = typename std::underlying_type< Enum >::type;
  return static_cast< underlying >( en & flag ) != 0;
};

}


#endif /* ENUM_BITFIELD_H */
