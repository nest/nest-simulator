/*
 *  clopath_archiving_node.h
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

#ifndef CLOPATH_ARCHIVING_NODE_H
#define CLOPATH_ARCHIVING_NODE_H

// C++ includes:
#include <deque>

// Includes from nestkernel:
#include "histentry.h"
#include "nest_time.h"
#include "nest_types.h"
#include "archiving_node.h"
#include "synaptic_element.h"

// Includes from sli:
#include "dictdatum.h"

namespace nest
{

/**
 * \class Clopath_Archiving_Node
 * a archiving node which additionally archives parameters
 * needed for the Clopath plasticity rule
 */
class Clopath_Archiving_Node : public Archiving_Node
{

public:
  /**
   * \fn Clopath_Archiving_Node()
   * Constructor.
   */
  Clopath_Archiving_Node();

  /**
   * \fn Clopath_Archiving_Node()
   * Copy Constructor.
   */
  Clopath_Archiving_Node( const Clopath_Archiving_Node& );

  /**
   * \fn double get_LTD_value(long t)
   * Returns value in LTD history at time t
   */
  double get_LTD_value( double t );

  /**
   * \fn void get_LTP_history(long t1, long t2,
   * std::deque<Archiver::histentry>::iterator* start,
   * std::deque<Archiver::histentry>::iterator* finish)
   * Sets pointer start (finish) to the first (last) entry in LTP_history
   * whose time argument is between t1 and t2
   */
  void get_LTP_history( double t1,
    double t2,
    std::deque< histentry_cl >::iterator* start,
    std::deque< histentry_cl >::iterator* finish );

  /**
   * \fn double get_theta_plus()
   * Returns threshold theta_plus_
   */
  double get_theta_plus() const;

  /**
   * \fn double get_theta_minus()
   * Returns threshold theta_minus_
   */
  double get_theta_minus() const;

protected:
  /**
   * \fn void write_LTD_history( Time const& t_sp,
   * double u_bar_minus, double u_bar_bar )
   * Creates a new entry in the LTD history and deletes old entries that
   * are not needed any more.
   */
  void write_LTD_history( const double t_ltd_ms, double u_bar_minus, double u_bar_bar );

  /**
   * \fn void write_LTP_history( const double t_ltp_ms, double u,
   * double u_bar_plus )
   * Creates a new entry in the LTP history and delets old entries that
   * are not needed any more.
   */
  void write_LTP_history( const double t_ltp_ms, double u, double u_bar_plus );

  /**
   * \fn void write_clopath_history( Time const& t_sp,
   * double u, double u_bar_plus, double u_bar_minus, double u_bar_bar )
   * Writes and reads the delayed_u_bar_[plus/minus] buffers and
   * calls write_LTD_history and write_LTP_history if
   * the corresponding Heaviside functions yield 1.
   */
  void write_clopath_history( Time const& t_sp, double u, double u_bar_plus, double u_bar_minus, double u_bar_bar );

  void init_clopath_buffers();
  void get_status( DictionaryDatum& d ) const;
  void set_status( const DictionaryDatum& d );

private:
  std::vector< histentry_cl > ltd_history_;
  std::deque< histentry_cl > ltp_history_;

  double A_LTD_;

  double A_LTP_;

  double u_ref_squared_;

  double theta_plus_;

  double theta_minus_;

  bool A_LTD_const_;

  double delay_u_bars_;
  size_t delay_u_bars_steps_;
  std::vector< double > delayed_u_bar_plus_;
  std::vector< double > delayed_u_bar_minus_;
  size_t delayed_u_bars_idx_;

  size_t ltd_hist_len_;

  size_t ltd_hist_current_;
};

inline double
Clopath_Archiving_Node::get_theta_plus() const
{
  return theta_plus_;
}

inline double
Clopath_Archiving_Node::get_theta_minus() const
{
  return theta_minus_;
}

} // of namespace
#endif
