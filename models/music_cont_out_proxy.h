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
//#include "recording_device.h"



/* BeginDocumentation

Name: music_cont_out_proxy - Device to forward contiguous values to remote applications using MUSIC.

Description:
A music_cont_out_proxy is used to send contiguous values to a remote application that
also uses MUSIC.

The music_cont_out_proxy represents a complete MUSIC contiguous output
port. The channel on the port to which a source node forwards its
events is determined during connection setup by using the parameter
music_channel of the connection. The name of the port is set via
SetStatus (see Parameters section below).

Parameters:
The following properties are available in the status dictionary:

port_name      - The name of the MUSIC output_port to forward events to
                 (default: event_out)
port_width     - The width of the MUSIC input port
published      - A bool indicating if the port has been already published
                 with MUSIC

The parameter port_name can be set using SetStatus.

Examples:
/iaf_neuron Create /n Set
/music_cont_out_proxy Create /meop Set
n meop << /music_channel 2 >> Connect

Author: Moritz Helias, Jochen Martin Eppler
FirstVersion: March 2009
Availability: Only when compiled with MUSIC

SeeAlso: music_event_in_proxy, music_cont_in_proxy, music_message_in_proxy
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
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and Hiding
   */
  using Node::handle;
  using Node::handles_test_event;
  using Node::sends_signal;
  port send_test_event( Node&, rport , synindex, bool );

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
    std::string port_name_; //!< the name of MUSIC port to connect to
    Time interval_;                   //!< recording interval, in ms
    std::vector< Name > record_from_; //!< which data to record

    Parameters_();                     //!< Sets default parameter values
    Parameters_( const Parameters_& ); //!< Recalibrate all times

    void get( DictionaryDatum&, const Variables_& ) const;          //!< Store current values in dictionary
    void set( const DictionaryDatum&, const State_&, const Buffers_&, Variables_& ); //!< Set values from dicitonary
  };

  // ------------------------------------------------------------

  struct State_
  {
    bool published_; //!< indicates whether this node has been published already with MUSIC
    int port_width_; //!< the width of the MUSIC port
    //int max_buffered_; //!< maximum delay (measured in multiples of music ticks) of publishing new data


    State_(); //!< Sets default state value

    void get( DictionaryDatum& ) const;                     //!< Store current values in dictionary
    void set( const DictionaryDatum&, const Parameters_& ); //!< Set values from dicitonary
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
    MUSIC::ContOutputPort * MP_; //!< The MUSIC event port for output of spikes

    std::vector< MUSIC::GlobalIndex > index_map_;
    MUSIC::PermutationIndex * music_perm_ind_; //!< The permutation index needed to map the ports of MUSIC.
    MUSIC::ArrayData * dmap_;
    //MPI_Datatype * multi_double_type;

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
