/*
 *  dictionary.cpp
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

#include <map>
#include <string>
#include <vector>
#include <boost/any.hpp>
#include <iostream>

#include "dictionary.h"


// debug
std::string
debug_type( const boost::any& operand )
{
  return operand.type().name();
}

// int
bool
is_int( const boost::any& operand )
{
  return operand.type() == typeid( int );
}

// long
bool
is_long( const boost::any& operand )
{
  return operand.type() == typeid( long );
}

bool
is_size_t( const boost::any& operand )
{
  return operand.type() == typeid( size_t );
}


// double
bool
is_double( const boost::any& operand )
{
  return operand.type() == typeid( double );
}

// bool
bool
is_bool( const boost::any& operand )
{
  return operand.type() == typeid( bool );
}

// string
bool
is_string( const boost::any& operand )
{
  return operand.type() == typeid( std::string );
}

// vector of ints
bool
is_int_vector( const boost::any& operand )
{
  return operand.type() == typeid( std::vector< int > );
}

// vector of doubles
bool
is_double_vector( const boost::any& operand )
{
  return operand.type() == typeid( std::vector< double > );
}

// vector of strings
bool
is_string_vector( const boost::any& operand )
{
  return operand.type() == typeid( std::vector< std::string > );
}

// dict
bool
is_dict( const boost::any& operand )
{
  return operand.type() == typeid( dictionary );
}
