#ifndef POSITION_H
#define POSITION_H

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

/*
    This file is part of the NEST topology module.
    Author: Kittel Austvoll
*/

#include "nest.h"
#include "token.h"
#include "exceptions.h"
#include "numerics.h"

#include <cstdlib>
#include <cassert>
#include <string>
#include <iostream>

namespace nest
{

  class DimensionalityMismatch;

  /**
   * Template class that stores data pairs giving the coordinates of a 
   * point in a topological grid/space. The class can be used for both
   * discrete and continuous spatial coordinates. It is adviced to
   * keep the class datatype as signed integers or doubles, but any 
   * datatype is allowed.
   */
  template <class DataType>
    class Position
    {
    private:
      // In most cases variable x would represent position along the 
      // horizontal axis and y position along the vertical axis.
      // The variable z is added for future use.
      DataType x;
      DataType y;
      DataType z;
      int dim_;

    public:
      
      Position():
	x(0), y(0), z(0), dim_(0)
      {}
      
      Position(const DataType a):
	x(a), y(0), z(0), dim_(1)
      {}

      Position(const DataType a, const DataType b):
	x(a), y(b), z(0), dim_(2)
      {}

      Position(const DataType a, const DataType b, const DataType c):
	x(a), y(b), z(c), dim_(3)
      {}

      Position(const Position<DataType>& a):
	x(a.x), y(a.y), z(a.z), dim_(a.dim_)
      {}
      
      Position(const std::vector<DataType>& a):
	x(0), y(0), z(0), dim_(a.size())
      {
	assert(a.size() > 1);

	x = a[0];
	if (a.size() > 1) { y = a[1]; }
	if (a.size() > 2) { z = a[2]; }
      }
      
      //! Return dimension of position
      int get_dim() const { return dim_; }

      /**
       * Wrap a displacement with regard to periodic boundary conditions.
       * This function should be called on Position objects representing
       * a displacement. Assuming periodic boundary conditions with respect
       * to the given box, each dimension will be wrapped so that it fulfills
       * -box.x/2 <= x <= box.x/2, and correspondingly for the other dimensions.
       * Thus it returns the shortest displacement given boundary conditions.
       *
       * @param box wrap into this box
       * @note This function does NOT wrap a position to its corresponding location
       *       inside the box. It should be applied to displacements only.
       */
      void wrap_displacement_max_half(const Position<DataType>& box)
      {
	/* x' = x - X round(x/X) will map x -> [-X/2, X/2]
	   thus enforcing periodic boundary conditions. */
	if ( dim_ != box.dim_ )
	  throw DimensionalityMismatch();

	if ( dim_ >= 1 )
	  x -= box.x * dround(x / box.x);
	if ( dim_ >= 2 )
	  y -= box.y * dround(y / box.y);
	if ( dim_ >= 3 )
	  z -= box.z * dround(z / box.z);

	assert(dim_ < 4);
      }

      /**
       * Print position to output stream.
       *
       * Format: Only as many coordinates as dimensions,
       *         separated by spaces [default], no trailing space.
       *
       * @param out output stream
       * @param sep separator character
       */
      void print(std::ostream& out, char sep = ' ') const;

      /**
       *Various operators for the Position class.
       */

      Position operator=(const Position<int_t>& a)
      {
	if(dim_ != a.dim_) { throw DimensionalityMismatch(); }
	x = a.x;
	y = a.y;
	z = a.z;

	return *this;
      }

      bool operator==(const Position<DataType>& a) const
      {
        if ( dim_ != a.dim_ ) throw DimensionalityMismatch();

        // non-existing dims always give true
        return   ( dim_ < 1 || x == a.x )
              && ( dim_ < 2 || y == a.y )
              && ( dim_ < 3 || z == a.z );
      }

      bool operator!=(const Position<DataType>& a) const
      {
	return !(*this == a);
      }

      void operator+=(const Position<DataType>& a)
      {
	if(dim_ != a.dim_) { throw DimensionalityMismatch(); }
	x += a.x;
	y += a.y;
	z += a.z;
      }

      void operator-=(const Position<DataType>& a)
      {
	if(dim_ != a.dim_) { throw DimensionalityMismatch(); }
	x -= a.x;
	y -= a.y;
	z -= a.z;
      }

      //! Elementwise vector multiplication
      void operator*=(const Position<DataType>& a)
      {
	if(dim_ != a.dim_) { throw DimensionalityMismatch(); }
	x *= a.x;
	y *= a.y;
	z *= a.z;
      }

      //! Elementwise vector division
      void operator/=(const Position<DataType>& a)
      {
	if(dim_ != a.dim_) { throw DimensionalityMismatch(); }

	if(dim_>=1)
	  x /= a.x;

	if(dim_>=2)
	  y /= a.y;

	if(dim_>=3)
	  z /= a.z;
      }
      

      void operator*=(const DataType& a)
      {
	x *= a;
	y *= a;
	z *= a;
      }

      void operator/=(const DataType& a)
      {
	x /= a;
	y /= a;
	z /= a;
      }


      /**
       * Checks if a point is in the upper-left quadrant of a 2D coordinate 
       * system with center in the input parameters position. 
       * Please note dubious meaning of <= (only true if both x and y is
       * less than input positions x and y). <= is not inverse of >.
       * @param a  center of quadrant structure
       */
      bool operator<=(const Position<DataType>& a ) const
      {
        if ( dim_ != a.dim_ ) throw DimensionalityMismatch();

        // non-existing dims always give true
	return   ( dim_ < 1 || x <= a.x )
	      && ( dim_ < 2 || y <= a.y )
	      && ( dim_ < 3 || z <= a.z );
      }


      bool operator<(const Position<DataType>& a ) const
      {
        if ( dim_ != a.dim_ ) throw DimensionalityMismatch();

        // non-existing dims always give true
        return   ( dim_ < 1 || x < a.x )
              && ( dim_ < 2 || y < a.y )
              && ( dim_ < 3 || z < a.z );
      }

      /**
       * Checks if a point is in the lower-right quadrant of a system 
       * with center in the input parameters position.
       * Please see notes for <= operator.
       * @param a  center of quadrant structure
       */
      bool operator>=(const Position<DataType>& a) const
      {
        if ( dim_ != a.dim_ ) throw DimensionalityMismatch();

        // non-existing dims always give true
        return   ( dim_ < 1 || x >= a.x )
              && ( dim_ < 2 || y >= a.y )
              && ( dim_ < 3 || z >= a.z );
      }
      
      bool operator>(const Position<DataType>& a) const
      {
        if ( dim_ != a.dim_ ) throw DimensionalityMismatch();

        // non-existing dims always give true
        return   ( dim_ < 1 || x > a.x )
              && ( dim_ < 2 || y > a.y )
              && ( dim_ < 3 || z > a.z );
      }

      operator Position<double_t>() const
      {
	switch(dim_) {
	case 0: return Position<double_t>();
	case 1: return Position<double_t>(x);
	case 2: return Position<double_t>(x,y);
	case 3: return Position<double_t>(x,y,z);
	default: assert(0);
	}
      }
      
      /**
       * Rounds of Position variables to nearest whole number.
       *
       * @note [-1/2, 1/2) -> 0 and in general [ (2n-1)/2, (2n+1)/2 ) -> n
       */
      Position<int_t> to_nearest_int() const
	{
	  switch(dim_) {
	  case 0:
	    return Position<int_t>();
	  case 1:
	    return Position<int_t>(static_cast<int_t>(dround(x)));
	  case 2:
	    return Position<int_t>(static_cast<int_t>(dround(x)),
				   static_cast<int_t>(dround(y)));
	  case 3:
	    return Position<int_t>(static_cast<int_t>(dround(x)), 
				   static_cast<int_t>(dround(y)), 
				   static_cast<int_t>(dround(z)));
	  default:
	    assert(false);
	  }
	}

      /**
       * Checks if position is within the spatial square formed by the 
       * input positions min and max.
       *
       * @param min  The position with the lowest x and(!) y values 
       *             in the region (i.e. the lower_left corner for
       *             continuous layers and the upper_left corner 
       *             (origo) for discrete layers).
       * @param max  The position with the highest x and(!) y values 
       *             in the region (i.e. the upper_right corner for
       *             continuous layers and the lower_right corner 
       *             for discrete layers).
       * @returns true if position is within spatial region.
       */
      bool within_range(const Position<DataType>& min, 
			const Position<DataType>& max) const
      {
	// A point is within bounds if its topological position is 
	// lower right of the upper-left bound range and upper left 
	// of the lower right bound range.
	return (*this) >= min && (*this) <= max;
      }

      /**
       * Converts a 2-dimensional layer position (discrete) to a 
       * 1-dimensional local id position.
       * @param height  number of rows in a topological layer
       * @param depth   layer depth
       * @returns local id of node at position
       */
      int_t pos2lid(const int_t rows) const
	{
	  if( dim_ != 2 ) { throw DimensionalityMismatch(); }
	  return x*rows + y;
	}

      int_t pos2lid(const int_t rows,
			  const int_t depth) const
	{
	  if( dim_ != 3 ) { throw DimensionalityMismatch(); }
	  return x*rows*depth + y*depth + z;
	}

      /**
       * Calculates the absolute distance of a position vector.
       * @returns distance of vector
       */
      double_t length() const
	{
	  // Standard pythagoras.
	  return std::sqrt(x*x+y*y+z*z);
	}

      /**
       * Converts the Position variables to their absolute value.
       * @returns new position object
       */
      Position<DataType> absolute() const
	{
	  switch(dim_) {
	  case 0:
	    return Position<DataType>();
	  case 1:
	    return Position<DataType>(std::abs(x));
	  case 2:
	    return Position<DataType>(std::abs(x), std::abs(y));
	  case 3:
	    return Position<DataType>(std::abs(x), std::abs(y), std::abs(z));
	  default:
	    assert(false);
	  }
	}

      /**
       * The following description is partly based on the description for 
       * SLI function arr::IndexWrap
       *
       * This function projects a cyclic integer index in the range (-oo,oo),
       * of periodicy N, onto its norm interval [0,N).
       *
       * This function can be used to "wrap around" array indices in order to
       * index an array
       *
       * @param N  Peroidicity of the cyclic index (i.e. the length of the 
       *           layer along the selected dimension).
       * @param index  value that should be projected on the half-open 
       *               interval [0,N).
       * @returns The cyclic equivalent of the given index, regarding period N.
       *
       * Examples:
       * index_wrap(-6, 3) -> 0
       * index_wrap(-5, 3) -> 1
       * index_wrap(-4, 3) -> 2
       * index_wrap(-3, 3) -> 0
       * index_wrap(-2, 3) -> 1
       * index_wrap(-1, 3) -> 2
       * index_wrap( 0, 3) -> 0
       * index_wrap( 1, 3) -> 1
       * index_wrap( 2, 3) -> 2
       * index_wrap( 3, 3) -> 0
       * index_wrap( 4, 3) -> 1
       * index_wrap( 5, 3) -> 2
       * index_wrap( 6, 3) -> 0
       */
      size_t index_wrap(const int_t index, const size_t N)
      {
	assert( N != 0);
	
	return (((std::abs(index)/N) + 1)*N+index) % N;
      }

      /**
       * The following description is partly based on the description for 
       * SLI function arr::EdgeWrap
       *
       * This function checks if the position lie inside the
       * range [0,columns] and [0,rows].
       *
       * x and y are modified according to the following rules:
       *
       * 1. If both indices lie inside [0,height) and [0,width), respectively,
       * they are left untouched.
       * 2. If the row    index lies outside [0,height) it is cyclicly wrapped
       * around. That is, a suitable multiple of "height" is added or
       * substracted, that makes the index fall inside [0,height).
       * 3. If the column index lies outside [0,width) it is cyclicly wrapped
       * around. That is, a suitable multiple of "width" is added or
       * substracted, that makes the index fall inside [0,width).
       *
       * @param columns
       * @param rows
       */
      void edge_wrap(const size_t columns, const size_t rows)
      {
	  x = index_wrap(x, columns);
	  y = index_wrap(y, rows);

	  assert(static_cast<size_t>(x)<columns);
	  assert(static_cast<size_t>(y)<rows);
	}

      void set_x(const DataType& a)
      {
	assert( dim_ >= 1 );
	x=a;
      }

      void set_y(const DataType& a)
      {
	assert( dim_ >= 2 );
	y=a;
      }

      DataType get_x() const
      {
	assert( dim_ >= 1 );
	return x;
      }

      DataType get_y() const
      {
	assert( dim_ >= 2 );
	return y;
      }

      DataType get_z() const
      {
	assert( dim_ >= 3 );
	return z;
      }
      
      /**
       * Moves Position variables into an array.
       * @returns array of positions stored as a token object.
       */
      Token getToken() const
      {
	std::vector<double_t> result = toVector();
	
	return Token(result);
      }
      
      std::vector<double_t> toVector() const
	{
	  std::vector<double_t> result;
	
	  if (dim_ >= 1) { result.push_back(x); }
	  if (dim_ >= 2) { result.push_back(y); }
	  if (dim_ >= 3) { result.push_back(z); }

	  return result;
	}
      
      template <class DT>
      friend std::ostream &operator<<(std::ostream &,const Position<DT>&);
    };

  template <class DataType>
  std::ostream &operator<<(std::ostream &out,const Position<DataType>& pos)
  {
    out << "(";
    if (pos.dim_>=1) out << pos.x;
    if (pos.dim_>=2) out << "," << pos.y;
    if (pos.dim_>=3) out << "," << pos.z;
    out << ")";
    return out;
  }

  template <class DataType>
  const Position<DataType> operator+(const Position<DataType>& a,
				     const Position<DataType>& b)
    {
      Position<DataType> r = a;
      r += b;
      return r;
    }

  template <class DataType>
  const Position<DataType> operator-(const Position<DataType>& a,
				     const Position<DataType>& b)
    {
      Position<DataType> r = a;
      r -= b;
      return r;
    }

  /**
   * Elementwise vector multiplication.
   */
    template <class DataType>
    const Position<DataType> operator*(const Position<DataType>& a,
				       const Position<DataType>& b)
    {
      Position<DataType> r = a;
      r *= b;
      return r;
    }
  
  /**
   * Elementwise vector division.
   */
  template <class DataType>
    const Position<DataType> operator/(const Position<DataType>& a,
				       const Position<DataType>& b)
    {
      Position<DataType> r = a;
      r /= b;
      return r;
    }

  template <class DataType>
    const Position<DataType> operator*(const Position<DataType>& a,
				       const DataType& b)
    {
      Position<DataType> r = a;
      r *= b;
      return r;
    }
  
  template <class DataType>
    const Position<DataType> operator/(const Position<DataType>& a,
				       const DataType& b)
    {
      Position<DataType> r = a;
      r /= b;
      return r;
    }

  template <typename DataType>
  void Position<DataType>::print(std::ostream& out, char sep) const
  {
    if ( dim_ > 0 ) out <<        x;
    if ( dim_ > 1 ) out << sep << y;
    if ( dim_ > 2 ) out << sep << z;
    return;
  }

  /**
   * Exception to be thrown if a Position is compared to another
   * with a different dimensionality.
   * @ingroup KernelExceptions
   */
  class DimensionalityMismatch: public KernelException
  {
    std::string positions_;

  public:
    DimensionalityMismatch()
      : KernelException("TopologyDimensionalityMismatch")  {}
    DimensionalityMismatch(const std::string &p)
      : KernelException("TopologyDimensionalityMismatch"), positions_(p)  {}
    ~DimensionalityMismatch() throw () {}
    
    std::string message() {
      if (positions_.empty()) {
	return std::string("Dimensionality of positions do not match.");
      } else {
	return std::string("Dimensionality of positions " + positions_ +
			   " do not match.");
      }
    }
  };



}

#endif
