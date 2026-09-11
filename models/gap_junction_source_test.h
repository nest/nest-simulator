/*
 *  gap_junction_source_test.h
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

#ifndef GAP_JUNCTION_SOURCE_TEST_H
#define GAP_JUNCTION_SOURCE_TEST_H

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "secondary_event.h"

namespace nest
{

/* BeginUserDocs: neuron, gap junction, testing

Short description
+++++++++++++++++

Test node exposing several gap-junction source and target ports

Description
+++++++++++

``gap_junction_source_test`` is an infrastructure test node for the generic
``source_port`` secondary-event routing. It exposes as many gap-junction source
and target ports as entries in ``source_values``. During each update step it
emits, on source port ``p``, a constant ``GapJunctionEvent`` whose value is
``source_values[p]``, so different ports carry deliberately different waveforms.
As a target it records, per receptor, the most recently delivered
``weight * value`` in ``received``.

The node is deliberately trivial (no dynamics); it exists so kernel tests can
verify that each target receives the waveform selected by its connection's
``source_port``.

Parameters
++++++++++

=============== ============= =================================================
 source_values  list of real  Constant value emitted on each source port; its
                              length sets the number of source and target ports
 received       list of real  (read-only) last ``weight * value`` per receptor
=============== ============= =================================================

Sends
+++++

GapJunctionEvent

Receives
++++++++

GapJunctionEvent

EndUserDocs */

void register_gap_junction_source_test( const std::string& name );

class gap_junction_source_test : public ArchivingNode
{

public:
  typedef Node base;

  gap_junction_source_test();
  gap_junction_source_test( const gap_junction_source_test& );

  using Node::handle;
  using Node::handles_test_event;
  using Node::sends_secondary_event;

  size_t send_test_event( Node& target, size_t receptor_type, synindex, bool ) override;

  void handle( GapJunctionEvent& ) override;
  size_t handles_test_event( GapJunctionEvent&, size_t ) override;

  void
  sends_secondary_event( GapJunctionEvent& ) override
  {
  }

  void sends_secondary_event( GapJunctionEvent&, const size_t source_port ) override;

  void get_status( Dictionary& ) const override;
  void set_status( const Dictionary& ) override;

private:
  void init_buffers_() override;
  void pre_run_hook() override;

  bool update_( Time const&, const long, const long, const bool called_from_wfr_update );
  void update( Time const&, const long, const long ) override;
  bool wfr_update( Time const&, const long, const long ) override;

  //! Number of source/target ports this node exposes.
  size_t n_ports_() const;

  //! Value emitted on each source port; length defines the number of ports.
  std::vector< double > source_values_;

  //! Last delivered weight * value per target receptor.
  std::vector< double > received_;
};

inline size_t
gap_junction_source_test::n_ports_() const
{
  return source_values_.size();
}

inline void
gap_junction_source_test::update( Time const& origin, const long from, const long to )
{
  update_( origin, from, to, false );
}

inline bool
gap_junction_source_test::wfr_update( Time const& origin, const long from, const long to )
{
  // The emitted waveform is constant, so a waveform-relaxation iteration always
  // reproduces the committed value and is trivially converged.
  update_( origin, from, to, true );
  return true;
}

inline size_t
gap_junction_source_test::send_test_event( Node& target, size_t receptor_type, synindex, bool )
{
  SpikeEvent se;
  se.set_sender( *this );
  return target.handles_test_event( se, receptor_type );
}

inline size_t
gap_junction_source_test::handles_test_event( GapJunctionEvent&, size_t receptor_type )
{
  if ( receptor_type >= n_ports_() )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return receptor_type;
}

}  // namespace nest

#endif /* #ifndef GAP_JUNCTION_SOURCE_TEST_H */
