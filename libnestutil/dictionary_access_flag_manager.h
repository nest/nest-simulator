/*
 *  dictionary_access_flag_manager.h
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

#ifndef DICTIONARY_ACCESS_FLAG_MANAGER_H
#define DICTIONARY_ACCESS_FLAG_MANAGER_H

#include <map>
#include <string>
#include <unordered_set>

#include "dictionary.h"

// TODO: PYNEST-NG: Here's an idea to improve the performance and
// address the possible bottlenecks mentioned below: Instead of
// performing access checks on node instances, they could be performed
// on the model prototypes (or clones thereof) just once prior to
// actually calling instance.set_status() or
// kernel_manager::set_status(). This would, however, require that all
// inter-dependent properties in such a call would be forced to be set
// together. In other words all calls to set_status() must be
// independent of the current state of the node instance.

/**
 * @brief Access flag manager for the dictionary class
 *
 * Manages access flags for dictionary keys and can check if all keys
 * in a dictionary has been accessed. Key access is not integrated into
 * the dictionary class to be able to keep the dictionary const.
 *
 * @note The access flag manager depends on the **address** of the dictionary to
 * keep track of each single dictionary. It is therefore essential that a dictionary is
 * **never copied**, since the new dict will have its accesses register separately.
 * There is also a **memory bloat** here because entries are not removed when
 * a dictionary is deleted.
 */
class DictionaryAccessFlagManager
{
private:
  using key_type_ = dictionary::key_type;
  std::map< const dictionary*, std::unordered_set< key_type_ > > access_flags_;

public:
  DictionaryAccessFlagManager() = default;
  ~DictionaryAccessFlagManager() = default;

  void init_access_flags( const dictionary& );
  void register_access( const dictionary&, const key_type_& );

  /**
   * @brief Check that all elements in a dictionary have been accessed.
   *
   * @param dict Dictionary to check
   * @param where Which function the error occurs in
   * @param what Which parameter triggers the error
   *
   */
  void all_accessed( const dictionary& dict, const std::string where, const std::string what ) const;

  /**
   * @brief Return whether the specified key has been accessed by the dictionary.
   *
   * @param dict Dictionary to check
   * @param key Key to check
   * @return true if key has been accessed
   * @return false if key has not been accessed
   */
  bool accessed( const dictionary& dict, const key_type_& key ) const;
};

inline void
DictionaryAccessFlagManager::init_access_flags( const dictionary& dict )
{
  // TODO PYNEST-NG: Performance bottleneck
#pragma omp critical( init_access_flags )
  {
    access_flags_[ &dict ] = {};
  }
}

inline void
DictionaryAccessFlagManager::register_access( const dictionary& dict, const key_type_& key )
{
  // TODO PYNEST-NG: Performance bottleneck
#pragma omp critical( register_access )
  {
    access_flags_[ &dict ].insert( key );
  }
}

#endif // DICTIONARY_ACCESS_FLAG_MANAGER_H
