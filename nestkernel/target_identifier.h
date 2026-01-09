/*
 *  target_identifier.h
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


#ifndef TARGET_IDENTIFIER_H
#define TARGET_IDENTIFIER_H

/**
 * @file Provide classes to be used as template arguments to Connection<T>.
 */

#include "node.h"

namespace nest
{

/**
 * Class providing classic target identified information with target pointer and
 * rport.
 *
 * This class represents a connection target using a pointer to the target
 * neuron and the rport. Connection classes with this class as template argument
 * provide "full" synapses.
 *
 * See Kunkel et al, Front Neuroinform 8:78 (2014), Sec 3.3.
 */
class TargetIdentifierPtrRport
{

public:
  TargetIdentifierPtrRport();

  TargetIdentifierPtrRport( const TargetIdentifierPtrRport& t ) = default;
  TargetIdentifierPtrRport& operator=( const TargetIdentifierPtrRport& t ) = default;

  void get_status( DictionaryDatum& d ) const;
  Node* get_target_ptr( const size_t ) const;

  size_t get_rport() const;

  void set_target( Node* target );

  void set_rport( size_t rprt );

private:
  Node* target_; //!< Target node
  size_t rport_; //!< Receiver port at the target node
};


/**
 * Class providing compact (hpc) target identified by index.
 *
 * This class represents a connection target using a thread-local index, while
 * fixing the rport to 0. Connection classes with this class as template
 * argument provide "hpc" synapses with minimal memory requirement.
 *
 * See Kunkel et al, Front Neuroinform 8:78 (2014), Sec 3.3.
 */
class TargetIdentifierIndex
{

public:
  TargetIdentifierIndex();

  TargetIdentifierIndex( const TargetIdentifierIndex& t ) = default;
  TargetIdentifierIndex& operator=( const TargetIdentifierIndex& t ) = default;

  void get_status( DictionaryDatum& d ) const;

  Node* get_target_ptr( const size_t tid ) const;
  size_t get_rport() const;

  void set_target( Node* target );

  void set_rport( size_t rprt );

private:
  targetindex target_; //!< Target node
};


} // namespace nest


#endif
