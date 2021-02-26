/*
 *  correlomatrix_detector.h
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

#ifndef CORRELOMATRIX_DETECTOR_H
#define CORRELOMATRIX_DETECTOR_H


// C++ includes:
#include <deque>
#include <vector>

// Includes from nestkernel:
#include "event.h"
#include "nest_timeconverter.h"
#include "nest_types.h"
#include "node.h"
#include "pseudo_recording_device.h"


namespace nest
{

/* BeginUserDocs: device, detector

Short description
+++++++++++++++++

Device for measuring the covariance matrix from several inputs

Description
+++++++++++

The correlomatrix_detector is a recording device. It is used to
record spikes from several pools of spike inputs and calculates the
covariance matrix of inter-spike intervals (raw auto and cross correlation)
binned to bins of duration delta_tau. The histogram is only recorded for
non-negative time lags. The negative part can be obtained by the symmetry of
the covariance matrix
 :math:` C(t) = C^T(-t)`.
The result can be obtained via GetStatus under the key /count_covariance.
In parallel it records a weighted histogram, where the connection weight are
used to weight every count, which is available under the key /covariance.
Both are matrices of size N_channels x N_channels, with each entry C_ij being
a vector of size tau_max/delta_tau + 1 containing the (weighted) histogram
for non-negative time lags.

The bins are centered around the time difference they represent, and are
left-closed and right-open in the lower triangular part of the matrix. On the
diagonal and in the upper triangular part the intervals are left-open and
right-closed. This ensures proper counting of events at the border of bins,
allowing consistent integration of a histogram over negative and positive
time lags by stacking two parts of the histogram

    (C(t)=[C[i][j][::-1],C[j][i][1:]]).

In this case one needs to exclude C[j][i][0] to avoid counting the zero-lag
bin twice.

The correlomatrix_detector has a variable number of inputs which can be set
via SetStatus under the key N_channels. All incoming connections to a
specified receptor will be pooled.

Remarks:

This recorder does not record to file, screen or memory in the usual
sense.

@note Correlomatrix detectors IGNORE any connection delays.

@note Correlomatrix detector breaks with the persistence scheme as
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

Parameters
++++++++++

================ ========= ====================================================
Tstart           real      Time when to start counting events. This time should
                           be set to at least start + tau_max in order to avoid
                           edge effects of the correlation counts.
Tstop            real      Time when to stop counting events. This time should
                           be set to at most Tsim - tau_max, where Tsim is the
                           duration of simulation, in order to avoid edge
                           effects of the correlation counts.
delta_tau        ms        Bin width. This has to be an odd multiple of
                           the resolution, to allow the symmetry between
                           positive and negative time-lags.
tau_max          ms        One-sided width. In the lower triagnular part
                           events with differences in [0, tau_max+delta_tau/2)
                           are counted. On the diagonal and in the upper
                           triangular part events with differences in
                           (0, tau_max+delta_tau/2].
N_channels       integer   The number of pools. This defines the range of
                           receptor_type. Default is 1.
                           Setting N_channels clears count_covariance,
                           covariance and n_events.
covariance       3D        matrix of read-only -raw, weighted, auto/cross
                 matrix of correlation
                 integers
count_covariance 3D        matrix of read-only -raw, auto/cross correlation
                 matrix of counts
                 integers
n_events         list of   number of events from all sources
                 integers
================ ========= ====================================================

Receives
++++++++

SpikeEvent

See also
++++++++

correlation_detector, spike_recorder

EndUserDocs */

class correlomatrix_detector : public Node
{

public:
  correlomatrix_detector();
  correlomatrix_detector( const correlomatrix_detector& );

  /**
   * This device has proxies, so that it will receive
   * spikes also from sources which live on other threads.
   */
  bool
  has_proxies() const
  {
    return true;
  }

  Name
  get_element_type() const
  {
    return names::recorder;
  }

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;

  void handle( SpikeEvent& );

  port handles_test_event( SpikeEvent&, rport );

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

  void calibrate_time( const TimeConverter& tc );

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
    long receptor_channel_;

    Spike_( long timestep, double weight, long receptorchannel )
      : timestep_( timestep )
      , weight_( weight )
      , receptor_channel_( receptorchannel )
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
    Time delta_tau_;  //!< width of correlation histogram bins
    Time tau_max_;    //!< maximum time difference of events to detect
    Time Tstart_;     //!< start of recording
    Time Tstop_;      //!< end of recording
    long N_channels_; //!< number of channels

    Parameters_();                     //!< Sets default parameter values
    Parameters_( const Parameters_& ); //!< Recalibrate all times

    Parameters_& operator=( const Parameters_& );

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary

    /**
     * Set values from dicitonary.
     * @returns true if the state needs to be reset after a change of
     *          binwidth or tau_max.
     */
    bool set( const DictionaryDatum&, const correlomatrix_detector&, Node* node );
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
    std::vector< long > n_events_; //!< spike counters
    SpikelistType incoming_;       //!< incoming spikes, sorted
                                   /** Weighted covariance matrix.
                                    *  @note Data type is double to accomodate weights.
                                    */
    std::vector< std::vector< std::vector< double > > > covariance_;

    /** Unweighted covariance matrix.
     */
    std::vector< std::vector< std::vector< long > > > count_covariance_;

    State_(); //!< initialize default state

    void get( DictionaryDatum& ) const;

    /**
     * @param bool if true, force state reset
     */
    void set( const DictionaryDatum&, const Parameters_&, bool, Node* node );

    void reset( const Parameters_& );
  };

  // ------------------------------------------------------------

  PseudoRecordingDevice device_;
  Parameters_ P_;
  State_ S_;
};

inline port
correlomatrix_detector::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type < 0 || receptor_type > P_.N_channels_ - 1 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return receptor_type;
}

inline void
nest::correlomatrix_detector::get_status( DictionaryDatum& d ) const
{
  device_.get_status( d );
  P_.get( d );
  S_.get( d );
}

inline void
nest::correlomatrix_detector::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;
  const bool reset_required = ptmp.set( d, *this, this );

  device_.set_status( d );
  P_ = ptmp;
  if ( reset_required == true )
  {
    S_.reset( P_ );
  }
}

inline void
nest::correlomatrix_detector::calibrate_time( const TimeConverter& tc )
{
  P_.delta_tau_ = tc.from_old_tics( P_.delta_tau_.get_tics() );
  P_.tau_max_ = tc.from_old_tics( P_.tau_max_.get_tics() );
  P_.Tstart_ = tc.from_old_tics( P_.Tstart_.get_tics() );
  P_.Tstop_ = tc.from_old_tics( P_.Tstop_.get_tics() );
}

} // namespace

#endif /* #ifndef CORRELOMATRIX_DETECTOR_H */
