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
  return ( he.t_ ) < t;
}

class HistEntryEprop
{
public:
  HistEntryEprop( double t );

  double t_;
  virtual ~HistEntryEprop()
  {
  }

  friend bool operator<( const HistEntryEprop& he, double t );
};

inline bool
operator<( const HistEntryEprop& he, double t )
{
  return ( he.t_ ) < t;
}

class HistEntryEpropArchive : public HistEntryEprop
{
public:
  HistEntryEpropArchive( double t, double V_m_pseudo_deriv, double learning_signal );

  double V_m_pseudo_deriv_;
  double learning_signal_;
};

class HistEntryEpropUpdate : public HistEntryEprop
{
public:
  HistEntryEpropUpdate( double t, size_t access_counter );

  size_t access_counter_;
};

class HistEntryEpropFiringRateReg : public HistEntryEprop
{
public:
  HistEntryEpropFiringRateReg( double t, double firing_rate_reg );

  double firing_rate_reg_;
};
}

#endif
