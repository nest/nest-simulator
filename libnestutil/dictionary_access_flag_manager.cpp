/*
 *  dictionary_access_flag_manager.cpp
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


#include <algorithm> // std::sort, std::copy, std::set_difference
#include <numeric>   // std::accumulate
#include <string>
#include <vector>

#include "dictionary_access_flag_manager.h"

void
DictionaryAccessFlagManager::all_accessed( const dictionary& dict,
  const std::string where,
  const std::string what ) const
{
  std::vector< key_type_ > keys;              // To hold keys of the dictionary
  std::vector< key_type_ > accessed_keys;     // To hold accessed keys, copied from the unordered_set in access_flags_
  std::vector< key_type_ > not_accessed_keys; // To hold keys in the dictionary that are not accessed

  const auto access_set = access_flags_.at( &dict );

  // Reserve memory for sizes we know
  keys.reserve( dict.size() );
  accessed_keys.reserve( access_set.size() );
  // Copy the keys from the dictionary to the vector
  for ( auto&& kv : dict )
  {
    keys.emplace_back( kv.first );
  }
  // Copy the keys from the set of accessed keys to the vector
  std::copy( access_set.begin(), access_set.end(), std::back_inserter( accessed_keys ) );
  // Sort keys so we can use set_difference to find unaccessed keys
  std::sort( keys.begin(), keys.end() );
  std::sort( accessed_keys.begin(), accessed_keys.end() );
  std::set_difference(
    keys.begin(), keys.end(), accessed_keys.begin(), accessed_keys.end(), std::back_inserter( not_accessed_keys ) );

  if ( not_accessed_keys.size() > 0 )
  {
    const auto missed = std::accumulate(
      not_accessed_keys.begin(), not_accessed_keys.end(), key_type_(), []( const key_type_& a, const key_type_& b ) {
        return a + " " + b;
      } );

    // TODO-PYNEST-NG: special case for blank <what> ("unaccessed elements in function <where>")?

    throw UnaccessedDictionaryEntry( std::string( "unaccessed elements in " ) + what + std::string( ", in function " )
      + where + std::string( ": " ) + missed );
  }
}

bool
DictionaryAccessFlagManager::accessed( const dictionary& dict, const key_type_& key )
{
  return access_flags_.at( &dict ).count( key ) > 0;
}
