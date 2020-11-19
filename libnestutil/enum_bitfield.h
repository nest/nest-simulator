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

/**
 * These methods support type-safe bitfields using C++'s enum classes.
 *
 * Define the bitfield flags as follows:
 *
 * .. code-block:: C++
 *
 *    enum class MyFlags : unsigned
 *    {
 *        FIRST_FLAG = 1 << 0,
 *        SECOND_FLAG = 1 << 1,
 *        THIRD_FLAG = 1 << 2,
 *        FOURTH_FLAG = 1 << 3
 *    };
 *
 * To prevent template substitution from enabling bitfield operators on *every* enum class, we use the enable_if
 * mechanism (see https://en.cppreference.com/w/cpp/language/sfinae).
 *
 * A templated struct `EnableBitMaskOperators` is defined, that, regardless of the template type, contains a single
 * static constant member `enable`, set to false. This is the "default", as the template may be specialised with any
 * type. However, we can override this behaviour by explicitly providing a template specialisation of
 * `EnableBitMaskOperators` for a particular type--namely, our bitfield enum class `MyFlags`--with a static constant
 * member by the same name but with value set to true. If we then want to ask during function definition of bitmask
 * operations whether they should be defined for an arbitrary type `T`, all we have to do is specalise
 * `EnableBitMaskOperators` by this type `T`, and check the value of its `enable` member using `enable_if`.
 *
 * To enable the bitfield operators for our `MyFlags` class, we thus specialise the template:
 *
 * .. code-block:: C++
 *
 *    template <>
 *    struct EnableBitMaskOperators< MyFlags >
 *    {
 *        static const bool enable = true;
 *    };
 *
 * Finally, we can instantiate a bitfield and do some operations on it, for example:
 *
 * .. code-block:: C++
 *
 *    MyFlags my_flags = MyFlags::FIRST_FLAG | MyFlags::FOURTH_FLAG;
 *    my_flags |= MyFlags::THIRD_FLAG;
 *    if ( enumFlagSet( my_flags, MyFlags::FOURTH_FLAG ) )
 *    {
 *        std::cout << "Fourth flag is set!" << std::endl;
 *    }
*/

template < typename Enum >
struct EnableBitMaskOperators
{
  static const bool enable = false;
};

template < typename Enum >
typename std::enable_if< EnableBitMaskOperators< Enum >::enable, Enum >::type operator|( Enum lhs, Enum rhs )
{
  using underlying = typename std::underlying_type< Enum >::type;
  return static_cast< Enum >( static_cast< underlying >( lhs ) | static_cast< underlying >( rhs ) );
}

template < typename Enum >
typename std::enable_if< EnableBitMaskOperators< Enum >::enable, Enum >::type operator&( Enum lhs, Enum rhs )
{
  using underlying = typename std::underlying_type< Enum >::type;
  return static_cast< Enum >( static_cast< underlying >( lhs ) & static_cast< underlying >( rhs ) );
}

template < typename Enum >
typename std::enable_if< EnableBitMaskOperators< Enum >::enable, Enum >::type operator^( Enum& lhs, Enum rhs )
{
  using underlying = typename std::underlying_type< Enum >::type;
  return static_cast< Enum >( static_cast< underlying >( lhs ) ^ static_cast< underlying >( rhs ) );
}

template < typename Enum >
typename std::enable_if< EnableBitMaskOperators< Enum >::enable, Enum >::type operator|=( Enum& lhs, Enum rhs )
{
  using underlying = typename std::underlying_type< Enum >::type;
  lhs = static_cast< Enum >( static_cast< underlying >( lhs ) | static_cast< underlying >( rhs ) );
  return lhs;
}

template < typename Enum >
typename std::enable_if< EnableBitMaskOperators< Enum >::enable, Enum >::type operator&=( Enum& lhs, Enum rhs )
{
  using underlying = typename std::underlying_type< Enum >::type;
  lhs = static_cast< Enum >( static_cast< underlying >( lhs ) & static_cast< underlying >( rhs ) );
  return lhs;
}

template < typename Enum >
typename std::enable_if< EnableBitMaskOperators< Enum >::enable, Enum >::type operator^=( Enum& lhs, Enum rhs )
{
  using underlying = typename std::underlying_type< Enum >::type;
  lhs = static_cast< Enum >( static_cast< underlying >( lhs ) ^ static_cast< underlying >( rhs ) );
  return lhs;
}

template < typename Enum >
bool
enumFlagSet( const Enum en, const Enum flag )
{
  using underlying = typename std::underlying_type< Enum >::type;
  return static_cast< underlying >( en & flag ) != 0;
}
}


#endif /* ENUM_BITFIELD_H */
