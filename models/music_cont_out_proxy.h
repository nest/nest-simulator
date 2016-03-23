/*
 *  music_cont_out_proxy.h
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

#ifndef music_cont_out_PROXY_H
#define music_cont_out_PROXY_H

#include "config.h"
#ifdef HAVE_MUSIC

#include <vector>
#include "nest.h"
#include "event.h"
#include "node.h"
#include "exceptions.h"
#include "mpi.h"
#include "music.hh"
#include "name.h"
#include "nest_names.h"
#include "connection.h"
#include "dictutils.h"
#include "sibling_container.h"

/* BeginDocumentation

Name: music_cont_out_proxy - A device which sends continuous data from NEST to
MUSIC.

Description:
A music_cont_out_proxy can be used to send continuous data from neurons over
MUSIC to
remote applications. It works in a similar like a multimeter model. The user
has to specify the recordable values to observe (e.g. ["V_m"]) via the
record_from parameter.
The target neurons are specified by a list of global neuron ids which must be
passed via
the "index_map" parameter. The music_cont_out_proxy will be connected
automatically to the
specified target neurons. It is not possible to change the list of target
neurons or observed
quantities once they have been set or the simulation has been started for the
first time.

Note: If only a single continuous value is observed, then the receiver must
provide a buffer
of MPI::DOUBLE values. Otherwise a custom MPI datatype, consisting of multiple
doubles,
must be created on receiver side as follows:

--- Example ---
//#include <mpi.h>
//int observed_values= 3;
//MPI_Datatype n_double_tuple;
//MPI_Type_contiguous( observed_values, MPI::DOUBLE, &n_double_tuple );
---------------

Parameters:
The following properties are available in the status dictionary:

interval     double   - Recording interval in milliseconds
index_map    array    - Global id list of neurons to be observed
port_name    string   - The name of the MUSIC output port to send to (default:
                        cont_out)
port_width   integer  - The width of the MUSIC input port
published    bool     - A bool indicating if the port has been already published
                        with MUSIC. Read only property.
record_from  array    - Array containing the names of variables to record
                        from, obtained from the /recordables entry of the
                        model from which one wants to record

Author: Martin Asghar Schulze, Forschungszentrum fur Informatik Karlsruhe (FZI)
FirstVersion: March 2016
Availability: Only when compiled with MUSIC

SeeAlso: music_cont_in_proxy, music_event_out_proxy, music_event_in_proxy,
music_message_in_proxy
*/
namespace nest
{
class Network;

class music_cont_out_proxy : public Node
{

public:
  music_cont_out_proxy();
  music_cont_out_proxy( const music_cont_out_proxy& );
  ~music_cont_out_proxy();

  bool
  has_proxies() const
  {
    return false;
  }
  bool
  local_receiver() const
  {
    return true;
  }
  bool
  one_node_per_process() const
  {
    return false;
  }

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;
  using Node::sends_signal;
  port send_test_event( Node&, rport, synindex, bool );

  void handle( DataLoggingReply& );

  SignalType sends_signal() const;

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

protected:
  void init_state_( Node const& );
  void init_buffers_();
  void calibrate();
  void finalize();

  /**
   * Collect and output membrane potential information.
   * This function pages all its targets at all pertinent sample
   * points for membrane potential information and then outputs
   * that information. The sampled nodes must provide data from
   * the previous time slice.
   */
  void update( Time const&, const long_t, const long_t );

private:
  struct State_;

  struct Buffers_;

  struct Variables_;

  struct Parameters_
  {
    std::string port_name_;           //!< the name of MUSIC port to connect to
    Time interval_;                   //!< recording interval, in ms
    std::vector< Name > record_from_; //!< which data to record

    Parameters_();                     //!< Sets default parameter values
    Parameters_( const Parameters_& ); //!< Recalibrate all times

    void get( DictionaryDatum&,
      const Variables_& ) const; //!< Store current values in dictionary
    void set( const DictionaryDatum&,
      const State_&,
      const Buffers_& ); //!< Set values from dicitonary
  };

  // ------------------------------------------------------------

  struct State_
  {
    bool published_; //!< indicates whether this node has been published already
    // with MUSIC
    int port_width_; //!< the width of the MUSIC port
    // int max_buffered_; //!< maximum delay (measured in multiples of music
    // ticks) of publishing new data


    State_(); //!< Sets default state value

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
  };

  // ------------------------------------------------------------

  struct Buffers_
  {
    /** Does this multimeter have targets?
     * Placed here since it is implementation detail.
     * @todo Ideally, one should be able to ask ConnectionManager.
     */
    Buffers_();

    bool has_targets_;
    std::vector< double > data_; //!< Recorded data
  };

  // ------------------------------------------------------------

  struct Variables_
  {
    MUSIC::ContOutputPort* MP_; //!< The MUSIC event port for output of spikes

    std::vector< MUSIC::GlobalIndex > index_map_;
    MUSIC::PermutationIndex* music_perm_ind_; //!< The permutation index needed
    // to map the ports of MUSIC.
    MUSIC::ArrayData* dmap_;
    Variables_();
  };

  // ------------------------------------------------------------

  // RecordingDevice device_;

  // ------------------------------------------------------------

  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;
};

inline SignalType
nest::music_cont_out_proxy::sends_signal() const
{
  return ALL;
}

} // namespace

#endif /* #ifndef music_cont_out_PROXY_H */

#endif
