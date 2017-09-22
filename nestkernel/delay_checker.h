/*
 *  delay_checker.h
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

#ifndef DELAY_CHECKER_H
#define DELAY_CHECKER_H

// Includes from nestkernel:
#include "nest_time.h"

// Includes from sli:
#include "dictdatum.h"

namespace nest
{
class TimeConverter;

class DelayChecker
{
public:
  DelayChecker();
  DelayChecker( const DelayChecker& );

  const Time& get_min_delay() const;

  const Time& get_max_delay() const;

  /**
   * This method freezes the min/ max delay update in SetDefaults of connections
   * method. This is used, when the delay of default connections in the
   * ConnectorModel is set: we do not know, whether new connections with this
   * delay will ever be created.
   */
  void freeze_delay_update();

  /**
   * This method enables the min/ max delay update in SetDefaults of connections
   * method. This is used, when the delay of default connections in the
   * ConnectorModel is set: we do not know, whether new connections with this
   * delay will ever be created.
   */
  void enable_delay_update();

  /**
   * Raise exception if delay value in milliseconds is invalid.
   *
   * @note Not const, since it may update delay extrema as a side-effect.
   */
  void assert_valid_delay_ms( double );

  /**
   * Raise exception if either of the two delays in steps is invalid.
   *
   * @note Setting continuous delays requires testing d and d+1. This function
   *       implements this more efficiently than two calls to
   *       assert_valid_delay().
   * @note This test accepts the delays in steps, as this makes more sense when
   *       working with continuous delays.
   * @note Not const, since it may update delay extrema as a side-effect.
   */
  void assert_two_valid_delays_steps( delay, delay );

  bool get_user_set_delay_extrema() const;

  void calibrate( const TimeConverter& );

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  Time min_delay_;              //!< Minimal delay of all created synapses.
  Time max_delay_;              //!< Maximal delay of all created synapses.
  bool user_set_delay_extrema_; //!< Flag indicating if the user set the delay
                                //!< extrema.
  bool freeze_delay_update_;
};

inline const Time&
DelayChecker::get_min_delay() const
{
  return min_delay_;
}

inline const Time&
DelayChecker::get_max_delay() const
{
  return max_delay_;
}

inline bool
DelayChecker::get_user_set_delay_extrema() const
{
  return user_set_delay_extrema_;
}

inline void
DelayChecker::freeze_delay_update()
{
  freeze_delay_update_ = true;
}

inline void
DelayChecker::enable_delay_update()
{
  freeze_delay_update_ = false;
}
}


#endif /* DELAY_CHECKER_H */
