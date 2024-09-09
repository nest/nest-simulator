/*
 *  histentry.h
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

#ifndef HISTENTRY_H
#define HISTENTRY_H

// Includes from nestkernel:
#include "nest_types.h"

namespace nest
{

/**
 * Class to represent a single entry in the spiking history of the ArchivingNode.
 */
class histentry
{
public:
  histentry( double t, double Kminus, double Kminus_triplet, size_t access_counter );

  double t_;              //!< point in time when spike occurred (in ms)
  double Kminus_;         //!< value of Kminus at that time
  double Kminus_triplet_; //!< value of triplet STDP Kminus at that time
  size_t access_counter_; //!< access counter to enable removal of the entry, once all neurons read it
};

// entry in the history of plasticity rules which consider additional factors
class histentry_extended
{
public:
  histentry_extended( double t, double dw, size_t access_counter );

  double t_; //!< point in time when spike occurred (in ms)
  double dw_;
  //! how often this entry was accessed (to enable removal, once read by all
  //! neurons which need it)
  size_t access_counter_;

  friend bool operator<( const histentry_extended he, double t );
};

inline bool
operator<( const histentry_extended he, double t )
{
  return he.t_ < t;
}

/**
 * Base class implementing history entries for e-prop plasticity.
 */
class HistEntryEprop
{
public:
  HistEntryEprop( long t );

  long t_;
  virtual ~HistEntryEprop()
  {
  }

  friend bool operator<( const HistEntryEprop& he, long t );
};

inline bool
operator<( const HistEntryEprop& he, long t )
{
  return he.t_ < t;
}

/**
 * Class implementing entries of the recurrent node model's history of e-prop dynamic variables.
 */
class HistEntryEpropRecurrent : public HistEntryEprop
{
public:
  HistEntryEpropRecurrent( long t, double surrogate_gradient, double learning_signal, double firing_rate_reg );

  double surrogate_gradient_;
  double learning_signal_;
  double firing_rate_reg_;
};

/**
 * Class implementing entries of the readout node model's history of e-prop dynamic variables.
 */
class HistEntryEpropReadout : public HistEntryEprop
{
public:
  HistEntryEpropReadout( long t, double error_signal );

  double error_signal_;
};

/**
 * Class implementing entries of the update history for e-prop plasticity.
 */
class HistEntryEpropUpdate : public HistEntryEprop
{
public:
  HistEntryEpropUpdate( long t, size_t access_counter );

  size_t access_counter_;
};

/**
 * Class implementing entries of the firing rate regularization history for e-prop plasticity.
 */
class HistEntryEpropFiringRateReg : public HistEntryEprop
{
public:
  HistEntryEpropFiringRateReg( long t, double firing_rate_reg );

  double firing_rate_reg_;
};
}

#endif
