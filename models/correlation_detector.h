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
#include "nest_timeconverter.h"
#include "nest_types.h"
#include "node.h"
#include "pseudo_recording_device.h"


namespace nest
{

/* BeginUserDocs: device, detector

Short description
+++++++++++++++++

Device for evaluating cross correlation between two spike sources

Description
+++++++++++

The ``correlation_detector`` is a device that receives spikes from two pools of
spike inputs and calculates the ``count_histogram`` of inter-spike intervals
(raw cross correlation) binned to bins of duration :math:`\delta_\tau`.
The corresponding parameter ``delta_tau`` defaults to 5 times the simulation
resolution.

The result can be obtained from the node's status dictionary under the key
``count_histogram``.

In parallel it records a weighted histogram, where the connection weights
are used to weight every count. In order to minimize numerical errors, the
`Kahan summation algorithm <http://en.wikipedia.org/wiki/Kahan_summation_algorithm>`_
is used when calculating the weighted histogram.
Both ``histogram`` and ``count_histogram`` are arrays of
:math:`2\cdot\tau_{max}/\delta_{\tau}+1` values, indexed by the bin number
:math:`n`, and are filled in the following way:

Let :math:`t_{1,i}` be the spike times of source 1 and
:math:`t_{2,j}` the spike times of source 2.
``histogram[n]`` then contains the sum of the weight products
:math:`w_{1,i}\cdot w_{2,j}`, and ``count_histogram[n]`` contains 1 summed over
all event pairs whose time difference :math:`t_{2,j}-t_{1,i}` falls in the
half-open interval

.. math::

    \left[ n\cdot\delta_\tau - \tau_{max} - \delta_\tau/2,\;
           n\cdot\delta_\tau - \tau_{max} + \delta_\tau/2 \right)

The bins are centered around the time difference they represent and are
left-closed and right-open. This means that events with time difference
:math:`-\tau_{max}-\delta_\tau/2` are counted in the leftmost bin, but events
with difference :math:`\tau_{max}+\delta_\tau/2` are not counted at all.

The bin centers run from :math:`-\tau_{max}` to :math:`+\tau_{max}` in steps of
:math:`\delta_\tau`. The corresponding array of time lags for the histogram bins
can therefore be constructed in PyNEST as

.. code-block:: python

    import numpy as np

    n_bins = int(2 * tau_max / delta_tau) + 1
    times = np.linspace(-tau_max, tau_max, n_bins)

The correlation detector has exactly two inputs, which are selected via the
``receptor_type`` of the incoming connection: all incoming connections with
``receptor_type = 0`` are pooled as spike source 1, the ones with
``receptor_type = 1`` as spike source 2.

Correlation detectors ignore any connection delays.

This recorder does not record to file, screen, or memory in the usual sense.
The recorded data is only available from the status dictionary.


Parameters
++++++++++

The following parameters can be set in the status dictionary.

============= ==== ==================================================================
Parameter     Unit Description
============= ==== ==================================================================
``Tstart``    ms   Time at which to start counting events. Set this to at least
                   ``tau_max`` in order to avoid edge effects of the correlation
                   counts.
``Tstop``     ms   Time at which to stop counting events. Set this to at most
                   ``Tsim - tau_max``, where ``Tsim`` is the duration of the
                   simulation, in order to avoid edge effects of the correlation
                   counts.
``delta_tau`` ms   Bin width. This has to be an odd multiple of the simulation
                   resolution, to allow the symmetry between positive and negative
                   time lags. Defaults to 5 times the simulation resolution.
``tau_max``   ms   One-sided maximum absolute time lag. Time differences in the
                   range ``[-tau_max - delta_tau/2, tau_max + delta_tau/2)`` are
                   binned. Must be a multiple of ``delta_tau``. Defaults to 10 times
                   the value of ``delta_tau``.
============= ==== ==================================================================

The following read-only quantities are available in the status dictionary.

======================== ===================================================================
Recordable               Description
======================== ===================================================================
``count_histogram``      Raw, unweighted cross-correlation counts (array of integers).
``histogram``            Weighted cross-correlation counts, where each count is weighted by
                         the product of the connection weights. The unit is squared synaptic
                         weights and depends on the model (array of doubles).
``histogram_correction`` Correction factors used internally for the Kahan summation
                         algorithm (array of doubles).
``n_events``             Number of events from source 0 and source 1 (list of two integers).
                         Setting ``n_events`` to ``[0, 0]`` clears the histograms.
======================== ===================================================================

Receives
++++++++

SpikeEvent

See also
++++++++

spike_recorder

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: correlation_detector

EndUserDocs */

/**
 * Correlation detector breaks with the persistence scheme as follows:
 * the internal buffers for storing spikes are part of State_, but are
 * initialized by init_buffers_().
 *
 * @todo The correlation detector could be made more efficient as follows
 * (HEP 2008-07-01):
 * - incoming_ is vector of two deques
 * - let handle() push_back() entries in incoming_ and do nothing else
 * - keep index to last "old spike" in each incoming_; cannot
 *   be iterator since that may change
 * - update() deletes all entries before now-tau_max, sorts the new
 *   entries, then registers new entries in histogram
 */

void register_correlation_detector( const std::string& name );

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
  has_proxies() const override
  {
    return true;
  }

  std::string
  get_element_type() const override
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

  void handle( SpikeEvent& ) override;

  size_t handles_test_event( SpikeEvent&, size_t ) override;

  void get_status( Dictionary& ) const override;
  void set_status( const Dictionary& ) override;

  void calibrate_time( const TimeConverter& tc ) override;

private:
  void init_state_() override;
  void init_buffers_() override;
  void pre_run_hook() override;

  void update( Time const&, const long, const long ) override;

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
    inline bool
    operator>( const Spike_& second ) const
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

    Parameters_();                      //!< Sets default parameter values
    Parameters_( const Parameters_& );  //!< Recalibrate all times

    Parameters_& operator=( const Parameters_& );

    void get( Dictionary& ) const;  //!< Store current values in dictionary

    /**
     * Set values from dictionary.
     * @returns true if the state needs to be reset after a change of
     *          binwidth or tau_max.
     */
    bool set( const Dictionary&, const correlation_detector&, Node* );

    Time get_default_delta_tau();
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
    std::vector< long > n_events_;           //!< spike counters
    std::vector< SpikelistType > incoming_;  //!< incoming spikes, sorted

    /** Weighted histogram.
     * @note Data type is double to accommodate weights.
     */
    std::vector< double > histogram_;

    //! used for Kahan summation algorithm
    std::vector< double > histogram_correction_;

    //! Unweighted histogram.
    std::vector< long > count_histogram_;

    State_();  //!< initialize default state

    void get( Dictionary& ) const;

    /**
     * @param bool if true, force state reset
     */
    void set( const Dictionary&, const Parameters_&, bool, Node* );

    void reset( const Parameters_& );
  };

  // ------------------------------------------------------------

  PseudoRecordingDevice device_;
  Parameters_ P_;
  State_ S_;
};

inline size_t
correlation_detector::handles_test_event( SpikeEvent&, size_t receptor_type )
{
  if ( receptor_type > 1 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }

  return receptor_type;
}

inline void
correlation_detector::get_status( Dictionary& d ) const
{
  device_.get_status( d );
  P_.get( d );
  S_.get( d );
}

inline void
correlation_detector::set_status( const Dictionary& d )
{
  Parameters_ ptmp = P_;
  const bool reset_required = ptmp.set( d, *this, this );
  State_ stmp = S_;
  stmp.set( d, P_, reset_required, this );

  device_.set_status( d );
  P_ = ptmp;
  S_ = stmp;
}

inline Time
correlation_detector::Parameters_::get_default_delta_tau()
{
  return 5 * Time::get_resolution();
}


}  // namespace

#endif /* #ifndef CORRELATION_DETECTOR_H */
