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

#include "nest_names.h"
#include "node.h"
#include "node_manager.h"

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
  TargetIdentifierPtrRport()
    : target_( nullptr )
    , rport_( 0 )
  {
  }

  TargetIdentifierPtrRport( const TargetIdentifierPtrRport& t ) = default;
  TargetIdentifierPtrRport& operator=( const TargetIdentifierPtrRport& t ) = default;

  void get_status( Dictionary& d ) const;
  Node* get_target_ptr( const size_t ) const;

  size_t get_rport() const;

  void set_target( Node* target );

  void set_rport( size_t rprt );

private:
  Node* target_;  //!< Target node
  size_t rport_;  //!< Receiver port at the target node
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
  TargetIdentifierIndex()
    : target_( invalid_targetindex )
  {
  }

  TargetIdentifierIndex( const TargetIdentifierIndex& t ) = default;
  TargetIdentifierIndex& operator=( const TargetIdentifierIndex& t ) = default;

  void get_status( Dictionary& d ) const;

  Node* get_target_ptr( const size_t tid ) const;
  size_t get_rport() const;

  void set_target( Node* target );

  void set_rport( size_t rprt );

private:
  targetindex target_;  //!< Target node
};

// --------------------------
// TargetIdentifierPtrRport
// --------------------------

inline void
TargetIdentifierPtrRport::get_status( Dictionary& d ) const
{
  // Do nothing if called on synapse prototype
  if ( target_ )
  {
    d[ names::rport ] = static_cast< long >( rport_ );
    d[ names::target ] = static_cast< long >( target_->get_node_id() );
  }
}

inline Node*
TargetIdentifierPtrRport::get_target_ptr( const size_t ) const
{
  return target_;
}

inline size_t
TargetIdentifierPtrRport::get_rport() const
{
  return rport_;
}

inline void
TargetIdentifierPtrRport::set_target( Node* target )
{
  target_ = target;
}

inline void
TargetIdentifierPtrRport::set_rport( size_t rprt )
{
  rport_ = rprt;
}


// -----------------------
// TargetIdentifierIndex
// -----------------------

inline void
TargetIdentifierIndex::get_status( Dictionary& d ) const
{
  // Do nothing if called on synapse prototype
  if ( target_ != invalid_targetindex )
  {
    d[ names::rport ] = 0;
    d[ names::target ] = target_;
  }
}

inline Node*
TargetIdentifierIndex::get_target_ptr( const size_t tid ) const
{
  assert( target_ != invalid_targetindex );
  return kernel::manager< NodeManager >.thread_lid_to_node( tid, target_ );
}

inline size_t
TargetIdentifierIndex::get_rport() const
{
  return 0;
}

inline void
TargetIdentifierIndex::set_target( Node* target )
{
  assert( kernel::manager< NodeManager >.thread_local_data_is_up_to_date() );

  const size_t target_lid = target->get_thread_lid();
  if ( target_lid > max_targetindex )
  {
    throw IllegalConnection(
      String::compose( "HPC synapses support at most %1 nodes per thread. "
                       "See Kunkel et al, Front Neuroinform 8:78 (2014), Sec 3.3.2.",
        max_targetindex ) );
  }
  target_ = target_lid;
}

inline void
TargetIdentifierIndex::set_rport( size_t rprt )
{
  if ( rprt != 0 )
  {
    throw IllegalConnection(
      "Only rport==0 allowed for HPC synapses. Use normal synapse models "
      "instead. See Kunkel et al, Front Neuroinform 8:78 (2014), Sec 3.3.2." );
  }
}

}  // namespace nest


#endif
