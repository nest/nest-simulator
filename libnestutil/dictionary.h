/*
 *  dictionary.h
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

#ifndef DICTIONARY_H_
#define DICTIONARY_H_

#include <map>
#include <string>
#include <vector>
#include <boost/any.hpp>

using dictionary = std::map< std::string, boost::any >;

// int
inline bool
is_int( const boost::any& operand )
{
  return operand.type() == typeid( int );
}

// double
inline bool
is_double( const boost::any& operand )
{
  return operand.type() == typeid( double );
}

// string
inline bool
is_string( const boost::any& operand )
{
  return operand.type() == typeid( std::string );
}

// vector of ints
inline bool
is_int_vector( const boost::any& operand )
{
  return operand.type() == typeid( std::vector< int > );
}

// vector of doubles
inline bool
is_double_vector( const boost::any& operand )
{
  return operand.type() == typeid( std::vector< double > );
}

// dict
inline bool
is_dict( const boost::any& operand )
{
  return operand.type() == typeid( dictionary );
}


#endif /* DICTIONARY_H_ */
