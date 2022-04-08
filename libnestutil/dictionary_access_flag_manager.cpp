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


#include <algorithm> // std::copy_if
#include <numeric>   // std::accumulate
#include <string>
#include <vector>

#include "dictionary_access_flag_manager.h"

#include "exceptions.h"

void
DictionaryAccessFlagManager::all_accessed( const dictionary& dict,
  const std::string where,
  const std::string what ) const
{
  // Vector of elements in the dictionary that are not accessed
  std::vector< dictionary::value_type > not_accessed_kv_pairs;

  const auto& access_set = access_flags_.at( &dict );
  const auto comparator = [&access_set](
                            dictionary::value_type kv ) { return access_set.find( kv.first ) == access_set.end(); };

  std::copy_if( dict.begin(), dict.end(), std::back_inserter( not_accessed_kv_pairs ), comparator );

  if ( not_accessed_kv_pairs.size() > 0 )
  {
    const auto missed = std::accumulate( not_accessed_kv_pairs.begin(),
      not_accessed_kv_pairs.end(),
      key_type_(),
      []( const key_type_& a, const dictionary::value_type& b ) { return a + " " + b.first; } );

    // TODO-PYNEST-NG: special case for blank <what> ("unaccessed elements in function <where>")?

    throw nest::UnaccessedDictionaryEntry( what, where, missed );
  }
  // TODO-PYNEST-NG: clear access_flags_[ &dict ] to reclaim memory?
}

bool
DictionaryAccessFlagManager::accessed( const dictionary& dict, const key_type_& key )
{
  return access_flags_.at( &dict ).count( key ) > 0;
}
