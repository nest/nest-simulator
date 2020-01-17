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

#ifndef MUSIC_CONT_OUT_PROXY_H
#define MUSIC_CONT_OUT_PROXY_H

// Generated includes:
#include "config.h"


#ifdef HAVE_MUSIC

// C includes:
#include <mpi.h>

// C++ includes:
#include <vector>

// External includes:
#include <music.hh>

// Includes from nestkernel:
#include "device_node.h"
#include "node_collection.h"
#include "nest_types.h"
#include "device_node.h"

// Includes from sli:
#include "arraydatum.h"

namespace nest
{

/** @BeginDocumentation
@ingroup Devices
@ingroup music

Name: music_cont_out_proxy - A device which sends continuous data from NEST to
MUSIC.

Description:

A music_cont_out_proxy can be used to send continuous data from
neurons over MUSIC to remote applications. It works in a similar fashion like
the multimeter model. The user has to specify the recordable values to observe
(e.g. ["V_m"]) via the record_from parameter. The target neurons are specified
by a list of global neuron ids which must be passed via the "targets"
parameter. The music_cont_out_proxy will be connected automatically to the
specified target neurons. It is not possible to apply further changes to the
list of target neurons or observed quantities once the simulation has been
started for the first time.

In case of multiple recordables the data can be read out (PyNEST only) of the
receiving buffer via the following access pattern:

    buffer[ target_node_id_index ][ recordable_index] = buffer[ target_node_id_index *
    record_from.size() + recordable_index ]

    For example:
    target_node_ids = [ 2, 5, 4 ], record_from = ["V_m"] and

    we want to get "V_m" for neuron with node ID 5: buffer[ 1*1 + 0 ]

Parameters:

The following properties are available in the status dictionary:

\verbatim embed:rst
============ ========  ========================================================
 interval    ms        Recording interval
 targets     array     Global id list of neurons to be observed
 port_name   string    The name of the MUSIC input port to listen to (default:
                       cont_in)
 port_width  integer   The width of the MUSIC input port
 published   boolean   A bool indicating if the port has been already published
                       with MUSIC
 record_from array     Array containing the names of variables to record
                       from, obtained from the /recordables entry of the
                       model from which one wants to record
============ ========  ========================================================
\endverbatim

Author: Martin Asghar Schulze, Forschungszentrum fur Informatik Karlsruhe (FZI)

FirstVersion: March 2016

Availability: Only when compiled with MPI and MUSIC

SeeAlso: music_cont_in_proxy, music_event_out_proxy, music_event_in_proxy,
music_message_in_proxy
*/
class music_cont_out_proxy : public DeviceNode
{

public:
  music_cont_out_proxy();
  music_cont_out_proxy( const music_cont_out_proxy& );

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
  void update( Time const&, const long, const long );

private:
  struct State_; //!< Forward declarations

  struct Buffers_;

  struct Parameters_
  {
    Parameters_();                     //!< Sets default parameter values
    Parameters_( const Parameters_& ); //!< Copy constructor for parameter values

    Time interval_;                   //!< sampling interval, in ms
    std::string port_name_;           //!< the name of MUSIC port to connect to
    std::vector< Name > record_from_; //!< recordables to record from
    NodeCollectionPTR targets_;       //!< nodes to be observed

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
    void set( const DictionaryDatum&, const Node&, const State_&, const Buffers_& ); //!< Set values from dictionary
  };

  // ------------------------------------------------------------

  struct State_
  {
    State_();                           //!< Sets default state value
    State_( const State_& );            //!< Copy constructor for state values
    bool published_;                    //!< indicates whether this node has been published
                                        //!< already with MUSIC
    size_t port_width_;                 //!< the width of the MUSIC port
    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
  };

  // ------------------------------------------------------------

  struct Buffers_
  {
    Buffers_();                  //!< Initializes default buffer
    Buffers_( const Buffers_& ); //!< Copy constructor for the data buffer
    bool has_targets_;           //!< Indicates whether the proxy is recording from any
                                 //!neurons or not
    std::vector< double > data_; //!< Recorded data
  };

  // ------------------------------------------------------------

  Parameters_ P_;
  State_ S_;
  Buffers_ B_;
};

inline SignalType
nest::music_cont_out_proxy::sends_signal() const
{
  return ALL;
}

} // namespace

#endif /* #ifndef HAVE_MUSIC */
#endif /* #ifndef MUSIC_CONT_OUT_PROXY_H */
