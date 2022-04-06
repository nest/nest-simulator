//
// Created by ayssar on 21.03.22.
//

#ifndef ARCHIVING_VECTOR_H
#define ARCHIVING_VECTOR_H

// C++ includes:
#include <algorithm>
#include <deque>

// Includes from nestkernel:
#include "histentry.h"
#include "nest_time.h"
#include "nest_types.h"
#include "node.h"
#include "structural_plasticity_vector.h"

// Includes from sli:
#include "dictdatum.h"

#define DEBUG_ARCHIVER 1

namespace nest
{
class ArchivingVector : public StructuralPlasticityVector
{
public:
  /**
   * \fn ArchivingVector()
   * Constructor.
   */
  ArchivingVector();

  /**
   * \fn ArchivingNode()
   * Copy Constructor.
   */
  ArchivingVector( const ArchivingVector& );

  /**
   * \fn double get_K_value(long t)
   * return the Kminus (synaptic trace) value at t (in ms). When the trace is
   * requested at the exact same time that the neuron emits a spike, the trace
   * value as it was just before the spike is returned.
   */
  double get_K_value( double t, index local_id );

  /**
   * \fn void get_K_values( double t,
   *   double& Kminus,
   *   double& nearest_neighbor_Kminus,
   *   double& Kminus_triplet )
   * write the Kminus (eligibility trace for STDP),
   * nearest_neighbour_Kminus (eligibility trace for nearest-neighbour STDP:
   *   like Kminus, but increased to 1, rather than by 1, on a spike
   *   occurrence),
   * and Kminus_triplet
   * values at t (in ms) to the provided locations.
   * @throws UnexpectedEvent
   */
  void
  get_K_values( double t, double& Kminus, double& nearest_neighbor_Kminus, double& Kminus_triplet, index local_id );

  /**
   * \fn void get_K_values( double t,
   *   double& Kminus,
   *   double& Kminus_triplet )
   * The legacy version of the function, kept for compatibility
   * after changing the function signature in PR #865.
   * @throws UnexpectedEvent
   */
  void
  get_K_values( double t, double& Kminus, double& Kminus_triplet, index local_id )
  {
    double nearest_neighbor_Kminus_to_discard;
    get_K_values( t, Kminus, nearest_neighbor_Kminus_to_discard, Kminus_triplet, local_id );
  }

  /**
   * \fn double get_K_triplet_value(std::deque<histentry>::iterator &iter)
   * return the triplet Kminus value for the associated iterator.
   */
  double get_K_triplet_value( const std::deque< histentry >::iterator& iter, index local_id );

  /**
   * \fn void get_history(long t1, long t2,
   * std::deque<Archiver::histentry>::iterator* start,
   * std::deque<Archiver::histentry>::iterator* finish)
   * return the spike times (in steps) of spikes which occurred in the range
   * (t1,t2].
   */
  void get_history( double t1,
    double t2,
    std::deque< histentry >::iterator* start,
    std::deque< histentry >::iterator* finish,
    index local_id );

  /**
   * Register a new incoming STDP connection.
   *
   * t_first_read: The newly registered synapse will read the history entries
   * with t > t_first_read.
   */
  void register_stdp_connection( double t_first_read, double delay, index local_id );

  void get_status( DictionaryDatum& d, index local_id ) const;
  void set_status( const DictionaryDatum& d, index local_id );

  void resize(index extended_space);

protected:
  /**
   * \fn void set_spiketime(Time const & t_sp, double offset)
   * record spike history
   */
  void set_spiketime( Time const& t_sp, index local_id, double offset = 0.0 );

  /**
   * \fn double get_spiketime()
   * return most recent spike time in ms
   */
  inline double get_spiketime_ms( index local_id ) const;

  /**
   * \fn void clear_history()
   * clear spike history
   */
  void clear_history( index local_id );

  // number of incoming connections from stdp connectors.
  // needed to determine, if every incoming connection has
  // read the spikehistory for a given point in time
 std::vector< size_t> n_incoming_;

private:
  // sum exp(-(t-ti)/tau_minus)
  std::vector< double > Kminus_;

  // sum exp(-(t-ti)/tau_minus_triplet)
  std::vector< double > Kminus_triplet_;

  std::vector< double > tau_minus_;
  std::vector< double > tau_minus_inv_;

  // time constant for triplet low pass filtering of "post" spike train
  std::vector< double > tau_minus_triplet_;
  std::vector< double > tau_minus_triplet_inv_;

  std::vector< double > max_delay_;
  std::vector< double > trace_;

  std::vector< double > last_spike_;

  // spiking history needed by stdp synapses
  std::vector< std::deque< histentry > > history_;
};
inline double
ArchivingVector::get_spiketime_ms( index local_id ) const
{
  return last_spike_.at( local_id );
}

}

#endif // ARCHIVING_VECTOR_H
