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
#include "archiving_node.h"
#include "histentry.h"
#include "nest_time.h"
#include "nest_types.h"
#include "synaptic_element.h"

// Includes from sli:
#include "dictdatum.h"

namespace nest
{

/**
 * An archiving node which additionally archives parameters
 * and buffers needed for the Clopath plasticity rule
 */
class ClopathArchivingNode : public ArchivingNode
{

public:
  ClopathArchivingNode();

  ClopathArchivingNode( const ClopathArchivingNode& );

  /**
   * Returns value in LTD history at time t
   */
  double get_LTD_value( double t ) override;

  /**
   * Sets pointer start (finish) to the first (last) entry in LTP_history
   * whose time argument is between t1 and t2
   */
  void get_LTP_history( double t1,
    double t2,
    std::deque< histentry_extended >::iterator* start,
    std::deque< histentry_extended >::iterator* finish ) override;

  /**
   * Returns threshold theta_plus_
   */
  double get_theta_plus() const;

  /**
   * Returns threshold theta_minus_
   */
  double get_theta_minus() const;

protected:
  /**
   * Creates a new entry in the LTD history and deletes old entries that
   * are not needed any more.
   */
  void write_LTD_history( const double t_ltd_ms, double u_bar_minus, double u_bar_bar );

  /**
   * Creates a new entry in the LTP history and delets old entries that
   * are not needed any more.
   */
  void write_LTP_history( const double t_ltp_ms, double u, double u_bar_plus );

  /**
   * Writes and reads the delayed_u_bar_[plus/minus] buffers and
   * calls write_LTD_history and write_LTP_history if
   * the corresponding Heaviside functions yield 1.
   */
  void write_clopath_history( Time const& t_sp, double u, double u_bar_plus, double u_bar_minus, double u_bar_bar );

  /**
   * Initialization of buffers.
   *
   * The implementation of the delay of the convolved membrane potentials as used
   * here is not described in Clopath et al. 2010, but is present in the code
   * on ModelDB (https://senselab.med.yale.edu/ModelDB/showmodel.cshtml?model=144566)
   * which was presumably used to create the figures in the paper. Since we write
   * into the buffer before we read from it, we have to add 1 to the size of the buffers.
   */
  void init_clopath_buffers();
  void get_status( DictionaryDatum& d ) const override;
  void set_status( const DictionaryDatum& d ) override;

private:
  std::vector< histentry_extended > ltd_history_;
  std::deque< histentry_extended > ltp_history_;

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


} // of namespace
#endif
