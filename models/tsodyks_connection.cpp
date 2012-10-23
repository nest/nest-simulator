/*
 *  tsodyks_connection.cpp
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

#include "tsodyks_connection.h"
#include "network.h"
#include "connector_model.h"

namespace nest
{

  TsodyksConnection::TsodyksConnection() :
    ConnectionHetWD(),
    tau_psc_(3.0),
    tau_fac_(0.0),
    tau_rec_(800.0),
    U_(0.5),
    x_(1.0),
    y_(0.0),
    u_(0.0)
  { }

  void TsodyksConnection::get_status(DictionaryDatum & d) const
  {
    ConnectionHetWD::get_status(d);

    def<double_t>(d, "U", U_);
    def<double_t>(d, "tau_psc", tau_psc_);
    def<double_t>(d, "tau_rec", tau_rec_);
    def<double_t>(d, "tau_fac", tau_fac_);
    def<double_t>(d, "x", x_);
    def<double_t>(d, "y", y_);
    def<double_t>(d, "u", u_);
  }
  
  void TsodyksConnection::set_status(const DictionaryDatum & d, ConnectorModel &cm)
  {
    ConnectionHetWD::set_status(d, cm);

    updateValue<double_t>(d, "U", U_);
    updateValue<double_t>(d, "tau_psc", tau_psc_);
    updateValue<double_t>(d, "tau_rec", tau_rec_);
    updateValue<double_t>(d, "tau_fac", tau_fac_);

    double_t x = x_;
    double_t y = y_;
    updateValue<double_t>(d, "x", x);
    updateValue<double_t>(d, "y", y);

    if (x + y > 1.0)
    {
      cm.network().message(SLIInterpreter::M_ERROR,
			   "TsodyksConnection::set_status()", "x + y must be <= 1.0.");
      throw BadProperty();
    }
    else
    {
      x_ = x;
      y_ = y;
    }

    updateValue<double_t>(d, "u", u_);
  }

  /**
   * Set properties of this connection from position p in the properties
   * array given in dictionary.
   */  
  void TsodyksConnection::set_status(const DictionaryDatum & d, index p, ConnectorModel &cm)
  {
    ConnectionHetWD::set_status(d, p, cm);

    set_property<double_t>(d, "Us", p, U_);
    set_property<double_t>(d, "tau_pscs", p, tau_psc_);
    set_property<double_t>(d, "tau_facs", p, tau_fac_);
    set_property<double_t>(d, "tau_recs", p, tau_rec_);


    double_t x = x_;
    double_t y = y_;
    set_property<double_t>(d, "xs", p, x);
    set_property<double_t>(d, "ys", p, y);

    if (x + y > 1.0)
    {
      cm.network().message(SLIInterpreter::M_ERROR, 
			   "TsodyksConnection::set_status()", 
			   "x + y must be <= 1.0.");
      throw BadProperty();
    }
    else
    {
      x_ = x;
      y_ = y;
    }

    set_property<double_t>(d, "us", p, u_);
  }

  void TsodyksConnection::initialize_property_arrays(DictionaryDatum & d) const
  {
    ConnectionHetWD::initialize_property_arrays(d);

    initialize_property_array(d, "Us"); 
    initialize_property_array(d, "tau_pscs");
    initialize_property_array(d, "tau_facs");
    initialize_property_array(d, "tau_recs");  
    initialize_property_array(d, "xs"); 
    initialize_property_array(d, "ys");
    initialize_property_array(d, "us");
  }

  /**
   * Append properties of this connection to the given dictionary. If the
   * dictionary is empty, new arrays are created first.
   */
  void TsodyksConnection::append_properties(DictionaryDatum & d) const
  {
    ConnectionHetWD::append_properties(d);

    append_property<double_t>(d, "Us", U_); 
    append_property<double_t>(d, "tau_pscs", tau_psc_);
    append_property<double_t>(d, "tau_facs", tau_fac_);
    append_property<double_t>(d, "tau_recs", tau_rec_);  
    append_property<double_t>(d, "xs", x_); 
    append_property<double_t>(d, "ys", y_);
    append_property<double_t>(d, "us", u_);
  }

} // of namespace nest
