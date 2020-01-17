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

#include "kernel_manager.h"
#include "compose.hpp"

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
    : target_( 0 )
    , rport_( 0 )
  {
  }


  TargetIdentifierPtrRport( const TargetIdentifierPtrRport& t )
    : target_( t.target_ )
    , rport_( t.rport_ )
  {
  }


  void
  get_status( DictionaryDatum& d ) const
  {
    // Do nothing if called on synapse prototype
    if ( target_ != 0 )
    {
      def< long >( d, names::rport, rport_ );
      def< long >( d, names::target, target_->get_node_id() );
    }
  }

  Node*
  get_target_ptr( const thread ) const
  {
    return target_;
  }

  rport
  get_rport() const
  {
    return rport_;
  }

  void
  set_target( Node* target )
  {
    target_ = target;
  }

  void
  set_rport( rport rprt )
  {
    rport_ = rprt;
  }

private:
  Node* target_; //!< Target node
  rport rport_;  //!< Receiver port at the target node
};


/**
 * Class providing compact (hpc) target identified by index.
 *
 * This class represents a connection target using a thread-local index, while
 * fixing the rport to 0. Connection classes with this class as template
 * argument provide "hpc" synapses with minimal memory requirement..
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


  TargetIdentifierIndex( const TargetIdentifierIndex& t )
    : target_( t.target_ )
  {
  }


  void
  get_status( DictionaryDatum& d ) const
  {
    // Do nothing if called on synapse prototype
    if ( target_ != invalid_targetindex )
    {
      def< long >( d, names::rport, 0 );
      def< long >( d, names::target, target_ );
    }
  }

  Node*
  get_target_ptr( const thread tid ) const
  {
    assert( target_ != invalid_targetindex );
    return kernel().node_manager.thread_lid_to_node( tid, target_ );
  }

  rport
  get_rport() const
  {
    return 0;
  }

  void set_target( Node* target );

  void
  set_rport( rport rprt )
  {
    if ( rprt != 0 )
    {
      throw IllegalConnection(
        "Only rport==0 allowed for HPC synpases. Use normal synapse models "
        "instead. See Kunkel et al, Front Neuroinform 8:78 (2014), Sec "
        "3.3.2." );
    }
  }

private:
  targetindex target_; //!< Target node
};

inline void
TargetIdentifierIndex::set_target( Node* target )
{
  kernel().node_manager.ensure_valid_thread_local_ids();

  index target_lid = target->get_thread_lid();
  if ( target_lid > max_targetindex )
  {
    throw IllegalConnection( String::compose(
      "HPC synapses support at most %1 nodes per thread. "
      "See Kunkel et al, Front Neuroinform 8:78 (2014), Sec 3.3.2.",
      max_targetindex ) );
  }
  target_ = target_lid;
}


} // namespace nest


#endif
