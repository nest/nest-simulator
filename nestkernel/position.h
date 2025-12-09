/*
 *  position.h
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

#ifndef POSITION_H
#define POSITION_H

// C++ includes:
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// Includes from libnestutil:
#include "compose.hpp"

// Includes from nestkernel:
#include "exceptions.h"
#include "nest_types.h"

// Includes from sli:
#include "token.h"

namespace nest
{

// It is necessary to declare the template for operator<< first in order
// to get the friend declaration to work
template < int D, class T >
class Position;

template < int D, class T >
std::ostream& operator<<( std::ostream& os, const Position< D, T >& pos );

template < int D, class T = double >
class Position
{
public:
  template < int OD, class OT >
  friend class Position;

  /**
   * Default constructor, initializing all coordinates to zero.
   */
  Position();

  /**
   * 2D Constructor.
   */
  Position( const T&, const T& );

  /**
   * 3D Constructor.
   */
  Position( const T&, const T&, const T& );

  /**
   * Constructor initializing a Position from an array.
   */
  Position( const T* const y );

  /**
   * Constructor initializing a Position from a std::vector.
   */
  Position( const std::vector< T >& y );

  Position( const Position& other );

  template < class U >
  Position( const Position< D, U >& other );

  /**
   * Move constructor.
   */
  Position( Position&& other );

  /**
   * Assignment constructor.
   */
  Position& operator=( const Position& other ) = default;
  Position& operator=( const std::vector< T >& y );

  /**
   * Move assignment constructor.
   */
  Position& operator=( Position&& other ) = default;

  /**
   * @returns an element (coordinate) of the Position
   */
  T& operator[]( int i );

  /**
   * @returns an element (coordinate) of the Position
   */
  const T& operator[]( int i ) const;

  /**
   * Moves Position variables into an array.
   *
   * @returns array of positions stored as a token object.
   */
  Token getToken() const;

  const std::vector< T > get_vector() const;
  void get_vector( std::vector< T >& vector ) const;

  /**
   * Elementwise addition.
   *
   * @returns elementwise sum of coordinates.
   */
  template < class OT >
  Position operator+( const Position< D, OT >& other ) const;

  /**
   * Elementwise subtraction.
   *
   * @returns elementwise difference of coordinates.
   */
  template < class OT >
  Position operator-( const Position< D, OT >& other ) const;

  /**
   * Unary minus.
   *
   * @returns opposite vector.
   */
  Position operator-() const;

  /**
   * Elementwise multiplication.
   *
   * @returns elementwise product of coordinates.
   */
  template < class OT >
  Position operator*( const Position< D, OT >& other ) const;

  /**
   * Elementwise division.
   *
   * @returns elementwise quotient of coordinates.
   */
  template < class OT >
  Position operator/( const Position< D, OT >& other ) const;

  /**
   * Elementwise addition with scalar
   *
   * @returns position vector with scalar added to all coordinates
   */
  Position operator+( const T& ) const;

  /**
   * Elementwise subtraction with scalar
   *
   * @returns position vector with scalar subtracted from all coordinates
   */
  Position operator-( const T& ) const;

  /**
   * Multiplication with scalar
   *
   * @returns position vector multiplied with the scalar.
   */
  Position operator*( const T& ) const;

  /**
   * Division with scalar
   *
   * @returns position vector divided by the scalar.
   */
  Position operator/( const T& ) const;

  /**
   * In-place elementwise addition.
   *
   * @returns the Position itself after adding the other Position
   * elementwise.
   */
  template < class OT >
  Position& operator+=( const Position< D, OT >& );

  /**
   * In-place elementwise subtraction.
   *
   * @returns the Position itself after subtracting the other Position
   * elementwise.
   */
  template < class OT >
  Position& operator-=( const Position< D, OT >& );

  /**
   * In-place elementwise multiplication.
   *
   * @returns the Position itself after multiplying with the other
   * Position elementwise.
   */
  template < class OT >
  Position& operator*=( const Position< D, OT >& );

  /**
   * In-place elementwise division.
   *
   * @returns the Position itself after dividing by the other Position
   * elementwise.
   */
  template < class OT >
  Position& operator/=( const Position< D, OT >& );

  /**
   * In-place elementwise addition with scalar.
   *
   * @returns the Position itself after adding the scalar to all coordinates.
   */
  Position& operator+=( const T& );

  /**
   * In-place elementwise subtraction with scalar.
   *
   * @returns the Position itself after subtracting the scalar from all
   * coordinates.
   */
  Position& operator-=( const T& );

  /**
   * In-place multiplication by scalar.
   *
   * @returns the Position itself after multiplying with the scalar.
   */
  Position& operator*=( const T& );

  /**
   * In-place elementwise division.
   *
   * @returns the Position itself after dividing by the scalar.
   */
  Position& operator/=( const T& );

  /**
   * @returns true if all coordinates are equal
   */
  bool operator==( const Position& y ) const;

  /**
   * @returns true if not all coordinates are equal
   */
  bool operator!=( const Position& y ) const;

  /**
   * @returns true if all coordinates are less
   */
  bool operator<( const Position& y ) const;

  /**
   * @returns true if all coordinates are greater
   */
  bool operator>( const Position& y ) const;

  /**
   * @returns true if all coordinates are less or equal
   */
  bool operator<=( const Position& y ) const;

  /**
   * @returns true if all coordinates are greater or equal
   */
  bool operator>=( const Position& y ) const;

  /**
   * Length of Position vector.
   *
   * @returns Euclidian norm of the vector.
   */
  T length() const;

  /**
   * @returns string representation of Position
   */
  operator std::string() const;

  /**
   * Print position to output stream.
   *
   * Format: Only as many coordinates as dimensions,
   *         separated by spaces [default], no trailing space.
   *
   * @param out output stream
   * @param sep separator character
   */
  void print( std::ostream& out, char sep = ' ' ) const;

  /**
   * Output the Position to an ostream.
   */
  friend std::ostream& operator<< <>( std::ostream& os, const Position< D, T >& pos );

protected:
  std::array< T, D > x_;
};

/**
 * A box is defined by the lower left corner (minimum coordinates) and
 * the upper right corner (maximum coordinates).
 */
template < int D >
struct Box
{
  Box()
  {
  }
  Box( const Position< D >& lower_left, const Position< D >& upper_right )
    : lower_left( lower_left )
    , upper_right( upper_right )
  {
  }

  Position< D > lower_left;
  Position< D > upper_right;
};

/**
 * An index into a multidimensional array.
 */
template < int D >
class MultiIndex : public Position< D, int >
{
public:
  MultiIndex()
    : Position< D, int >()
    , lower_left_()
    , upper_right_()
  {
  }

  explicit MultiIndex( const Position< D, int >& ur )
    : Position< D, int >()
    , lower_left_()
    , upper_right_( ur )
  {
  }

  MultiIndex( const Position< D, int >& lower_left, const Position< D, int >& upper_right )
    : Position< D, int >( lower_left )
    , lower_left_( lower_left )
    , upper_right_( upper_right )
  {
  }

  MultiIndex&
  operator++()
  {
    // Try increasing the first coordinate first, resetting it and
    // continuing with the next if the first one overflows, and so on
    for ( int i = 0; i < D; ++i )
    {
      this->x_[ i ]++;
      if ( this->x_[ i ] < upper_right_[ i ] )
      {
        return *this;
      }
      this->x_[ i ] = lower_left_[ i ];
    }
    // If we reach this point, we are outside of bounds. The upper
    // right point is used as a marker to show that we have reached the end.
    for ( int i = 0; i < D; ++i )
    {
      this->x_[ i ] = upper_right_[ i ];
    }
    return *this;
  }

  MultiIndex
  operator++( int )
  {
    MultiIndex tmp = *this;
    ++*this;
    return tmp;
  }

  Position< D, int >
  get_lower_left() const
  {
    return lower_left_;
  }
  Position< D, int >
  get_upper_right() const
  {
    return upper_right_;
  }

private:
  Position< D, int > lower_left_;
  Position< D, int > upper_right_;
};

} // namespace nest

#endif
