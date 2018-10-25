/*
 *  correlation_detector.h
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

#ifndef CORRELATION_DETECTOR_H
#define CORRELATION_DETECTOR_H


// C++ includes:
#include <deque>
#include <vector>

// Includes from nestkernel:
#include "event.h"
#include "nest_types.h"
#include "node.h"
#include "pseudo_recording_device.h"


namespace nest
{

/** @BeginDocumentation
Name: correlation_detector - Device for evaluating cross correlation between
                             two spike sources

Description:

The correlation_detector device is a recording device. It is used to record
spikes from two pools of spike inputs and calculates the count_histogram of
inter-spike intervals (raw cross correlation) binned to bins of duration
delta_tau. The result can be obtained via GetStatus under the key
/count_histogram.
In parallel it records a weighted histogram, where the connection weights
are used to weight every count. In order to minimize numerical errors the
Kahan summation algorithm is used when calculating the weighted histogram.
(http://en.wikipedia.org/wiki/Kahan_summation_algorithm)
Both are arrays of 2*tau_max/delta_tau+1 values containing the histogram
counts in the following way:

Let t_{1,i} be the spike times of source 1,
t_{2,j} the spike times of source 2.
histogram[n] then contains the sum of products of the weight w_{1,i}*w_{2,j},
count_histogram[n] contains 1 summed over all events with t_{2,j}-t_{1,i} in

[ n*delta_tau - tau_max - delta_tau/2 , n*delta_tau - tau_max + delta_tau/2 )

The bins are centered around the time difference they represent, but are
left-closed and right-open. This means that events with time difference
-tau_max-delta_tau/2 are counted in the leftmost bin, but event with
difference tau_max+delta_tau/2 are not counted at all.

The correlation detector has two inputs, which are selected via the
receptor_port of the incoming connection: All incoming connections with
receptor_port = 0 will be pooled as the spike source 1, the ones with
receptor_port = 1 will be used as spike source 2.

Parameters:

Tstart     double    - Time when to start counting events. This time should
                       be set to at least start + tau_max in order to avoid
                       edge effects of the correlation counts.
Tstop      double    - Time when to stop counting events. This time should be
                       set to at most Tsim - tau_max, where Tsim is the
                       duration of simulation, in order to avoid edge effects
                       of the correlation counts.
delta_tau  double    - bin width in ms
tau_max    double    - one-sided histogram width in ms. Events with
                       differences in
                       [-tau_max-delta_tau/2, tau_max+delta_tau/2)
                       are counted.

histogram            double vector, read-only  - raw, weighted cross
                                                 correlation counts
histogram_correction double_vector, read-only  - correction factors for kahan
                                                 summation algorithm
count_histogram      long vector, read-only    - raw, cross correlation
                                                 counts
n_events             integer vector            - number of events from source
                                                 0 and 1. By setting n_events
                                                 to [0 0], the histogram is
                                                 cleared.

Remarks:

This recorder does not record to file, screen or memory in the usual
sense.

Correlation detectors IGNORE any connection delays.

Correlation detector breaks with the persistence scheme as
follows: the internal buffers for storing spikes are part
of State_, but are initialized by init_buffers_().

@todo The correlation detector could be made more efficient as follows
(HEP 2008-07-01):
- incoming_ is vector of two deques
- let handle() push_back() entries in incoming_ and do nothing else
- keep index to last "old spike" in each incoming_; cannot
  be iterator since that may change
- update() deletes all entries before now-tau_max, sorts the new
  entries, then registers new entries in histogram

Example:

/s1 /spike_generator Create def
/s2 /spike_generator Create def
s1 << /spike_times [ 1.0 1.5 2.7 4.0 5.1 ] >> SetStatus
s2 << /spike_times [ 0.9 1.8 2.1 2.3 3.5 3.8 4.9 ] >> SetStatus
/cd /correlation_detector Create def
cd << /delta_tau 0.5 /tau_max 2.5 >> SetStatus
s1 cd << /receptor_type 0 >> Connect
s2 cd << /receptor_type 1 >> Connect
10 Simulate
cd [/n_events] get ==   --> [# 5 7 #]
cd [/histogram] get ==  --> [. 0 3 3 1 4 3 2 6 1 2 2 .]
cd << /reset true >> SetStatus
cd [/histogram] get ==  --> [. 0 0 0 0 0 0 0 0 0 0 0 .]

Receives: SpikeEvent

Author: Moritz Helias
        Jakob Jordan (implemented Kahan summation algorithm) 2013/02/18

FirstVersion: 2007/5/21

SeeAlso: spike_detector, Device, PseudoRecordingDevice

Availability: NEST
*/
class correlation_detector : public Node
{

public:
  correlation_detector();
  correlation_detector( const correlation_detector& );

  /**
   * This device has proxies, so that it will receive
   * spikes also from sources which live on other threads.
   */
  bool
  has_proxies() const
  {
    return true;
  }

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;

  void handle( SpikeEvent& ); //!< @todo implement if-else in term of function

  port handles_test_event( SpikeEvent&, rport );

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  void init_state_( Node const& );
  void init_buffers_();
  void calibrate();

  void update( Time const&, const long, const long );

  // ------------------------------------------------------------

  /**
   * Spike structure to store in the deque of recently
   * received events
   */
  struct Spike_
  {
    long timestep_;
    double weight_;

    Spike_( long timestep, double weight )
      : timestep_( timestep )
      , weight_( weight )
    {
    }

    /**
     * Greater operator needed for insertion sort.
     */
    inline bool operator>( const Spike_& second ) const
    {
      return timestep_ > second.timestep_;
    }
  };

  typedef std::deque< Spike_ > SpikelistType;

  // ------------------------------------------------------------

  struct State_;

  struct Parameters_
  {

    Time delta_tau_; //!< width of correlation histogram bins
    Time tau_max_;   //!< maximum time difference of events to detect
    Time Tstart_;    //!< start of recording
    Time Tstop_;     //!< end of recording

    Parameters_();                     //!< Sets default parameter values
    Parameters_( const Parameters_& ); //!< Recalibrate all times

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary

    /**
     * Set values from dicitonary.
     * @returns true if the state needs to be reset after a change of
     *          binwidth or tau_max.
     */
    bool set( const DictionaryDatum&, const correlation_detector& );
  };

  // ------------------------------------------------------------

  /**
   * @todo Is there a replacement for std::list that allows fast
   *       insertion inside, fast deletion at the beginning, and
   *       maintains sorting?
   * @note Constructed with empty structures, which are set to
   *       proper sizes by init_buffers_().
   * @note State_ only contains read-out values, so we copy-construct
   *       using the default c'tor.
   */
  struct State_
  {
    std::vector< long > n_events_;          //!< spike counters
    std::vector< SpikelistType > incoming_; //!< incoming spikes, sorted

    /** Weighted histogram.
     * @note Data type is double to accommodate weights.
     */
    std::vector< double > histogram_;

    //! used for Kahan summation algorithm
    std::vector< double > histogram_correction_;

    //! Unweighted histogram.
    std::vector< long > count_histogram_;

    State_(); //!< initialize default state

    void get( DictionaryDatum& ) const;

    /**
     * @param bool if true, force state reset
     */
    void set( const DictionaryDatum&, const Parameters_&, bool );

    void reset( const Parameters_& );
  };

  // ------------------------------------------------------------

  PseudoRecordingDevice device_;
  Parameters_ P_;
  State_ S_;
};

inline port
correlation_detector::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type < 0 || receptor_type > 1 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

  return receptor_type;
}

inline void
nest::correlation_detector::get_status( DictionaryDatum& d ) const
{
  device_.get_status( d );
  P_.get( d );
  S_.get( d );

  ( *d )[ names::element_type ] = LiteralDatum( names::recorder );
}

inline void
nest::correlation_detector::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;
  const bool reset_required = ptmp.set( d, *this );
  State_ stmp = S_;
  stmp.set( d, P_, reset_required );

  device_.set_status( d );
  P_ = ptmp;
  S_ = stmp;
}


} // namespace

#endif /* #ifndef CORRELATION_DETECTOR_H */
