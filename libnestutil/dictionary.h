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

std::string debug_type( const boost::any& operand );

bool is_int( const boost::any& operand );
bool is_long( const boost::any& operand );
bool is_size_t( const boost::any& operand );
bool is_double( const boost::any& operand );
bool is_bool( const boost::any& operand );
bool is_string( const boost::any& operand );
bool is_int_vector( const boost::any& operand );
bool is_double_vector( const boost::any& operand );
bool is_string_vector( const boost::any& operand );
bool is_dict( const boost::any& operand );

#endif /* DICTIONARY_H_ */
