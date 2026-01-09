/*
 *  target_identifier.cpp
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

#include "target_identifier.h"

#include "compose.hpp"
#include "kernel_manager.h"
#include "node.h"
#include "node_manager.h"

#include <cassert>

namespace nest
{

// --------------------------
// TargetIdentifierPtrRport
// --------------------------

TargetIdentifierPtrRport::TargetIdentifierPtrRport()
  : target_( nullptr )
  , rport_( 0 )
{
}

void
TargetIdentifierPtrRport::get_status( DictionaryDatum& d ) const
{
  // Do nothing if called on synapse prototype
  if ( target_ )
  {
    def< long >( d, names::rport, rport_ );
    def< long >( d, names::target, target_->get_node_id() );
  }
}

Node*
TargetIdentifierPtrRport::get_target_ptr( const size_t ) const
{
  return target_;
}

size_t
TargetIdentifierPtrRport::get_rport() const
{
  return rport_;
}

void
TargetIdentifierPtrRport::set_target( Node* target )
{
  target_ = target;
}

void
TargetIdentifierPtrRport::set_rport( size_t rprt )
{
  rport_ = rprt;
}


// -----------------------
// TargetIdentifierIndex
// -----------------------

TargetIdentifierIndex::TargetIdentifierIndex()
  : target_( invalid_targetindex )
{
}

void
TargetIdentifierIndex::get_status( DictionaryDatum& d ) const
{
  // Do nothing if called on synapse prototype
  if ( target_ != invalid_targetindex )
  {
    def< long >( d, names::rport, 0 );
    def< long >( d, names::target, target_ );
  }
}

Node*
TargetIdentifierIndex::get_target_ptr( const size_t tid ) const
{
  assert( target_ != invalid_targetindex );
  return kernel::manager< NodeManager >.thread_lid_to_node( tid, target_ );
}

size_t
TargetIdentifierIndex::get_rport() const
{
  return 0;
}

void
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

void
TargetIdentifierIndex::set_rport( size_t rprt )
{
  if ( rprt != 0 )
  {
    throw IllegalConnection(
      "Only rport==0 allowed for HPC synapses. Use normal synapse models "
      "instead. See Kunkel et al, Front Neuroinform 8:78 (2014), Sec 3.3.2." );
  }
}

} // namespace nest
