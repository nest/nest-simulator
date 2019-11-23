/*
 *  proxynode.h
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

#ifndef PROXYNODE_H
#define PROXYNODE_H

// Includes from nestkernel:
#include "node.h"

namespace nest
{
class SpikeEvent;
class CurrentEvent;

/** @BeginDocumentation
Name: proxynode - Proxy to provide Nodes on remote machines

Description:

Remarks:

Parameters:

References:

Author: June 2005, Jochen Martin Eppler
*/

/**
 * Proxy Node to provide Nodes, where there aren't real Nodes to be
 */
class proxynode : public Node
{

public:
  proxynode()
    : Node()
  {
    set_frozen_( true );
  }

  /**
   * Construct proxy node for internal use from
   * @param node_id of represented node
   * @param model id of represented node
   * @param vp of represented node
   */
  proxynode( index, index, index );

  /**
   * Import sets of overloaded virtual functions.
   * We need to explicitly include sets of overloaded
   * virtual functions into the current scope.
   * According to the SUN C++ FAQ, this is the correct
   * way of doing things, although all other compilers
   * happily live without.
   */
  using Node::handle;
  using Node::sends_signal;

  port send_test_event( Node&, rport, synindex, bool );

  void sends_secondary_event( GapJunctionEvent& );

  SignalType sends_signal() const;

  void sends_secondary_event( InstantaneousRateConnectionEvent& );

  void sends_secondary_event( DiffusionConnectionEvent& );

  void sends_secondary_event( DelayedRateConnectionEvent& );

  void
  handle( SpikeEvent& )
  {
  }

  void get_status( DictionaryDatum& ) const;

  /**
   * Proxy nodes have no properties.
   * If set_status() gets called for a proxy node, this is
   * and error; we must prevent this from happening, since the
   * user might otherwise thaw a proxy node. It also causes
   * problems with dictionary entry checking
   */
  void
  set_status( const DictionaryDatum& )
  {
    assert( false );
  }

  bool is_proxy() const;

  thread
  get_thread() const
  {
    assert( false );
  }

private:
  void
  init_state_( const Node& )
  {
  }
  void
  init_buffers_()
  {
  }
  void
  calibrate()
  {
  }
  void
  update( Time const&, const long, const long )
  {
  }
};

inline bool
proxynode::is_proxy() const
{
  return true;
}

} // namespace

#endif // PROXYNODE_H
