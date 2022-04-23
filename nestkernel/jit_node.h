/*
 *  jit_node.h
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

#ifndef JITNODE_H
#define JITNODE_H

#include "node.h"
#include "vectorized_node.h"


namespace nest
{

class JitNode : public Node
{
private:
  index local_id_;


public:
  std::shared_ptr< VectorizedNode > container_;

  JitNode();
  ~JitNode();

  JitNode( JitNode const& );

  void reset_node();

  void resize( index extended_space );

  std::shared_ptr< VectorizedNode >
  get_container()
  {
    return container_;
  }

  void clone_container( std::shared_ptr< VectorizedNode > container );

  std::map< std::string, const std::vector< double >& > get_recordables() const;


  index get_pos_in_thread() const;

  void set_pos_in_thread( index pos );

  bool supports_urbanczik_archiving() const;

  bool local_receiver() const;

  bool one_node_per_process() const;

  bool is_off_grid() const;

  bool is_proxy() const;

  index get_node_id() const;

  index
  get_node_local_id() const
  {
    return this->local_id_;
  }

  bool is_frozen() const;

  bool node_uses_wfr() const;

  void set_node_uses_wfr( const bool );

  void init();

  void calibrate();

  void calibrate_time( const TimeConverter& );

  void post_run_cleanup();

  void finalize();

  void update( Time const&, const long, const long );

  bool wfr_update( Time const&, const long, const long );

  void set_status( const DictionaryDatum& );

  void get_status( DictionaryDatum& ) const;

  void set_container( std::shared_ptr< VectorizedNode > container );

  port send_test_event( Node& receiving_node, rport receptor_type, synindex syn_id, bool dummy_target );

  port handles_test_event( SpikeEvent&, rport receptor_type );

  port handles_test_event( WeightRecorderEvent&, rport receptor_type );

  port handles_test_event( RateEvent&, rport receptor_type );

  port handles_test_event( DataLoggingRequest&, rport receptor_type );

  port handles_test_event( CurrentEvent&, rport receptor_type );

  port handles_test_event( ConductanceEvent&, rport receptor_type );

  port handles_test_event( DoubleDataEvent&, rport receptor_type );

  port handles_test_event( DSSpikeEvent&, rport receptor_type );

  port handles_test_event( DSCurrentEvent&, rport receptor_type );

  port handles_test_event( GapJunctionEvent&, rport receptor_type );

  port handles_test_event( InstantaneousRateConnectionEvent&, rport receptor_type );

  port handles_test_event( DiffusionConnectionEvent&, rport receptor_type );

  port handles_test_event( DelayedRateConnectionEvent&, rport receptor_type );

  void sends_secondary_event( GapJunctionEvent& ge );

  void sends_secondary_event( InstantaneousRateConnectionEvent& re );

  void sends_secondary_event( DelayedRateConnectionEvent& re );

  void sends_secondary_event( DiffusionConnectionEvent& de );

  void register_stdp_connection( double, double );

  void handle( SpikeEvent& e );

  void handle( WeightRecorderEvent& e );

  void handle( RateEvent& e );

  void handle( DataLoggingRequest& e );

  void handle( DataLoggingReply& e );

  void handle( CurrentEvent& e );

  void handle( ConductanceEvent& e );

  void handle( DoubleDataEvent& e );

  void handle( GapJunctionEvent& e );

  void handle( InstantaneousRateConnectionEvent& e );

  void handle( DiffusionConnectionEvent& e );

  void handle( DelayedRateConnectionEvent& e );

  double get_Ca_minus() const;

  double get_synaptic_elements( Name ) const;

  int get_synaptic_elements_vacant( Name ) const;

  int get_synaptic_elements_connected( Name ) const;

  std::map< Name, double > get_synaptic_elements() const;

  void update_synaptic_elements( double );

  void decay_synaptic_elements_vacant();

  void connect_synaptic_element( Name, int );

  double get_K_value( double t );

  double get_LTD_value( double t );

  void get_K_values( double t, double& Kminus, double& nearest_neighbor_Kminus, double& Kminus_triplet );

  void get_history( double t1,
    double t2,
    std::deque< histentry >::iterator* start,
    std::deque< histentry >::iterator* finish );


  void get_LTP_history( double t1,
    double t2,
    std::deque< histentry_extended >::iterator* start,
    std::deque< histentry_extended >::iterator* finish );

  void get_urbanczik_history( double t1,
    double t2,
    std::deque< histentry_extended >::iterator* start,
    std::deque< histentry_extended >::iterator* finish,
    int value );

  double get_C_m( int comp );

  double get_g_L( int comp );

  double get_tau_L( int comp );

  double get_tau_s( int comp );

  double get_tau_syn_ex( int comp );

  double get_tau_syn_in( int comp );

  void event_hook( DSSpikeEvent& );

  void event_hook( DSCurrentEvent& );


  SignalType sends_signal() const;

  SignalType receives_signal() const;

  void set_status_base( const DictionaryDatum& );

private:
  void set_node_id_( index );

protected:
  void init_state_();

  void init_buffers_();

  void set_initialized_();

  void set_frozen_( bool frozen );
};
}
#endif // JITNODE_H
