/*
 *  correlospinmatrix_detector.h
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

#ifndef CORRELOSPINMATRIX_DETECTOR_H
#define CORRELOSPINMATRIX_DETECTOR_H


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

The correlospinmatrix_detector is a recording device. It is used
to record correlations from binary neurons from several binary sources and
calculates the raw auto and cross correlation binned to bins of duration
delta_tau. The result can be obtained via GetStatus under the key
/count_covariance. The result is a tensor of rank 3 of size
N_channels x N_channels, with each entry :math:`C_{ij}` being a vector of size
:math:`2*\tau_{max}/\delta_{\tau} + 1` containing the histogram for the
different
time lags.

The bins are centered around the time difference they represent, and are
left-closed and right-open in the lower triangular part of the matrix. On the
diagonal and in the upper triangular part the intervals are left-open and
right-closed. This ensures proper counting of events at the border of bins.

The correlospinmatrix_detector has a variable number of inputs which can be
set via SetStatus under the key N_channels. All incoming connections to a
specified receptor will be pooled.

Remarks:

This recorder does not record to file, screen or memory in the usual
sense. The result must be obtained by a call to GetStatus. Setting either
N_channels, Tstart, Tstop, tau_max or delta_tau clears count_covariance.

Correlospinmatrix detectors IGNORE any connection delays.

Correlospinmatrix detector breaks with the persistence scheme as
follows: the internal buffers for storing spikes are part
of State_, but are initialized by init_buffers_().

See pynest/examples/correlospinmatrix_detector_two_neuron.py
for a script reproducing a setting studied in Fig 1 of Grinzburg &
Sompolinsky (1994) PRE 50(4) p. 3171.

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
================ ========= ====================================================

Receives
++++++++

SpikeEvent

See also
++++++++

correlation_detector, correlomatrix_detector, spike_recorder

EndUserDocs */

class correlospinmatrix_detector : public Node
{

public:
  correlospinmatrix_detector();
  correlospinmatrix_detector( const correlospinmatrix_detector& );

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
  using Node::receives_signal;

  void handle( SpikeEvent& );

  port handles_test_event( SpikeEvent&, rport );

  SignalType receives_signal() const;

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

  void calibrate_time( const TimeConverter& tc );

private:
  void init_state_();
  void init_buffers_();
  void calibrate();

  void update( Time const&, const long, const long );

  // ------------------------------------------------------------

  /**
   * Structure to store in the deque of recently
   * received events marked by beginning and end of the binary on pulse
   */
  struct BinaryPulse_
  {
    long t_on_;
    long t_off_;
    long receptor_channel_;

    BinaryPulse_( long timeon, long timeoff, long receptorchannel )
      : t_on_( timeon )
      , t_off_( timeoff )
      , receptor_channel_( receptorchannel )
    {
    }

    /**
     * Greater operator needed for insertion sort.
     */
    inline bool operator>( const BinaryPulse_& second ) const
    {
      return t_off_ > second.t_off_;
    }
  };

  typedef std::deque< BinaryPulse_ > BinaryPulselistType;

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
    bool set( const DictionaryDatum&, const correlospinmatrix_detector&, Node* );
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
    BinaryPulselistType incoming_; //!< incoming binary pulses, sorted
                                   /**
                                    * rport of last event coming in
                                    * (needed for decoding logic of binary events)
                                    */
    rport last_i_;
    /**
     * time of last event coming in (needed for decoding logic of binary events)
     */
    Time t_last_in_spike_;

    //! potentially a down transition (single spike received)
    bool tentative_down_;

    std::vector< bool > curr_state_; //!< current state of neuron i

    //! last time pointof change of neuron i
    std::vector< long > last_change_;

    /** Unweighted covariance matrix.
     */
    std::vector< std::vector< std::vector< long > > > count_covariance_;

    State_(); //!< initialize default state

    void get( DictionaryDatum& ) const;

    /**
     * @param bool if true, force state reset
     */
    void set( const DictionaryDatum&, const Parameters_&, bool, Node* );

    void reset( const Parameters_& );
  };

  // ------------------------------------------------------------

  PseudoRecordingDevice device_;
  Parameters_ P_;
  State_ S_;
};

inline port
correlospinmatrix_detector::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type < 0 || receptor_type > P_.N_channels_ - 1 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return receptor_type;
}

inline void
nest::correlospinmatrix_detector::get_status( DictionaryDatum& d ) const
{
  device_.get_status( d );
  P_.get( d );
  S_.get( d );
}

inline void
nest::correlospinmatrix_detector::set_status( const DictionaryDatum& d )
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


inline SignalType
nest::correlospinmatrix_detector::receives_signal() const
{
  return BINARY;
}

inline void
nest::correlospinmatrix_detector::calibrate_time( const TimeConverter& tc )
{
  P_.delta_tau_ = tc.from_old_tics( P_.delta_tau_.get_tics() );
  P_.tau_max_ = tc.from_old_tics( P_.tau_max_.get_tics() );
  P_.Tstart_ = tc.from_old_tics( P_.Tstart_.get_tics() );
  P_.Tstop_ = tc.from_old_tics( P_.Tstop_.get_tics() );

  S_.t_last_in_spike_ = tc.from_old_tics( S_.t_last_in_spike_.get_tics() );
}

} // namespace

#endif /* #ifndef CORRELOSPINMATRIX_DETECTOR_H */
