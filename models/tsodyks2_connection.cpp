/*
 *  tsodyks2_connection.cpp
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

#include "tsodyks2_connection.h"
#include "network.h"
#include "connector_model.h"
#include "nest_names.h"

namespace nest
{

  Tsodyks2Connection::Tsodyks2Connection() :
    ConnectionHetWD(),
    U_(0.5),
    u_(U_),
    x_(U_),
    tau_rec_(800.0),
    tau_fac_(0.0)
  {
  }

  void Tsodyks2Connection::get_status(DictionaryDatum & d) const
  {
    ConnectionHetWD::get_status(d);

    def<double_t>(d, names::dU, U_);
    def<double_t>(d, names::u, u_);
    def<double_t>(d, names::tau_rec, tau_rec_);
    def<double_t>(d, names::tau_fac, tau_fac_);
    def<double_t>(d, names::x, x_);
    
  }
  
  void Tsodyks2Connection::set_status(const DictionaryDatum & d, ConnectorModel &cm)
  {
    ConnectionHetWD::set_status(d, cm);
    
    updateValue<double_t>(d, names::dU, U_);
    updateValue<double_t>(d, names::u, u_);
    updateValue<double_t>(d, names::tau_rec, tau_rec_);
    updateValue<double_t>(d, names::tau_fac, tau_fac_);
    updateValue<double_t>(d, names::x, x_);
  }

  /**
   * Set properties of this connection from position p in the properties
   * array given in dictionary.
   */  
  void Tsodyks2Connection::set_status(const DictionaryDatum & d, index p, ConnectorModel &cm)
  {
    ConnectionHetWD::set_status(d, p, cm);

    set_property<double_t>(d, names::dUs, p, U_);
    set_property<double_t>(d, names::us, p, u_);
    set_property<double_t>(d, names::xs, p, x_);
    set_property<double_t>(d, names::tau_recs, p, tau_rec_);
    set_property<double_t>(d, names::tau_facs, p, tau_fac_);
  }

  void Tsodyks2Connection::initialize_property_arrays(DictionaryDatum & d) const
  {
    ConnectionHetWD::initialize_property_arrays(d);

    initialize_property_array(d, names::dUs); 
    initialize_property_array(d, names::us); 
    initialize_property_array(d, names::tau_recs);  
    initialize_property_array(d, names::tau_facs);  
    initialize_property_array(d, names::xs); 
  }

  /**
   * Append properties of this connection to the given dictionary. If the
   * dictionary is empty, new arrays are created first.
   */
  void Tsodyks2Connection::append_properties(DictionaryDatum & d) const
  {
    ConnectionHetWD::append_properties(d);

    append_property<double_t>(d, names::dUs, U_); 
    append_property<double_t>(d, names::us, u_); 
    append_property<double_t>(d, names::tau_recs, tau_rec_);  
    append_property<double_t>(d, names::tau_facs, tau_fac_);  
    append_property<double_t>(d, names::xs, x_); 
  }

} // of namespace nest
