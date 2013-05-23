/*
 *  stdp_dopa_connection.cpp
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

#include "network.h"
#include "dictdatum.h"
#include "connector_model.h"
#include "common_synapse_properties.h"
#include "stdp_dopa_connection.h"
#include "event.h"
#include "nestmodule.h"

namespace nest
{
  //
  // Implementation of class STDPDopaCommonProperties.
  //

  STDPDopaCommonProperties::STDPDopaCommonProperties() :
    CommonSynapseProperties(),
    vt_(0),
    A_plus_(1.0),
    A_minus_(1.5),
    tau_plus_(20.0),
    tau_c_(1000.0),
    tau_n_(200.0),
    b_(0.0),
    Wmin_(0.0),
    Wmax_(200.0)
  {}

  void STDPDopaCommonProperties::get_status(DictionaryDatum & d) const
  {
    CommonSynapseProperties::get_status(d);

    if(vt_!= 0)
      def<long_t>(d, "vt", vt_->get_gid());
    else
      def<long_t>(d, "vt", -1);

    def<double_t>(d, "A_plus", A_plus_);
    def<double_t>(d, "A_minus", A_minus_);
    def<double_t>(d, "tau_plus", tau_plus_);
    def<double_t>(d, "tau_c", tau_c_);
    def<double_t>(d, "tau_n", tau_n_);
    def<double_t>(d, "b", b_);
    def<double_t>(d, "Wmin", Wmin_);
    def<double_t>(d, "Wmax", Wmax_);
  }

  void STDPDopaCommonProperties::set_status(const DictionaryDatum & d, ConnectorModel &cm)
  {
    CommonSynapseProperties::set_status(d, cm);

    long_t vtgid;
    if ( updateValue<long_t>(d, "vt", vtgid) )
    {
      vt_ = dynamic_cast<volume_transmitter *>(NestModule::get_network().get_node(vtgid));

      if(vt_==0)
	throw BadProperty("Dopamine source must be volume transmitter");
    }

    updateValue<double_t>(d, "A_plus", A_plus_);
    updateValue<double_t>(d, "A_minus", A_minus_);
    updateValue<double_t>(d, "tau_plus", tau_plus_);
    updateValue<double_t>(d, "tau_c", tau_c_);
    updateValue<double_t>(d, "tau_n", tau_n_);
    updateValue<double_t>(d, "b", b_);
    updateValue<double_t>(d, "Wmin", Wmin_);
    updateValue<double_t>(d, "Wmax", Wmax_);
  }

  Node* STDPDopaCommonProperties::get_node()
  {
    if(vt_==0)
      throw BadProperty("No volume transmitter has been assigned to the dopamine synapse.");
    else
      return vt_;
  }


  //
  // Implementation of class STDPDopaConnection.
  //

  STDPDopaConnection::STDPDopaConnection() :
    c_(0.0),
    n_(0.0),
    dopa_spikes_idx_(0),
    t_last_update_(0.0)
  {}

  STDPDopaConnection::STDPDopaConnection(const STDPDopaConnection &rhs) :
    ConnectionHetWD(rhs)
  {
    c_ = rhs.c_;
    n_ = rhs.n_;
    dopa_spikes_idx_ = rhs.dopa_spikes_idx_;
    t_last_update_ = rhs.t_last_update_;
  }

  void STDPDopaConnection::get_status(DictionaryDatum & d) const
  {
    // base class properties, different for individual synapse
    ConnectionHetWD::get_status(d);

    // own properties, different for individual synapse
    def<double_t>(d, "c", c_);
    def<double_t>(d, "n", n_);
  }

  void STDPDopaConnection::set_status(const DictionaryDatum & d, ConnectorModel &cm)
  {
    // base class properties
    ConnectionHetWD::set_status(d, cm);

    updateValue<double_t>(d, "c", c_);
    updateValue<double_t>(d, "n", n_);
  }

  /**
   * Set properties of this connection from position p in the properties
   * array given in dictionary.
   */
  void STDPDopaConnection::set_status(const DictionaryDatum & d, index p, ConnectorModel &cm)
  {
    ConnectionHetWD::set_status(d, p, cm);

    if ( d->known("A_pluss")    ||
	 d->known("A_minuss")   ||
	 d->known("tau_pluss")  ||
	 d->known("tau_cs")     ||
	 d->known("tau_ns")     ||
	 d->known("bs")         ||
	 d->known("Wmins")      ||
	 d->known("Wmaxs") )
      {
	cm.network().message(SLIInterpreter::M_ERROR, "STDPDopaConnection::set_status()", "you are trying to set common properties via an individual synapse.");
      }

    set_property<double_t>(d, "cs", p, c_);
    set_property<double_t>(d, "ns", p, n_);
  }

  void STDPDopaConnection::initialize_property_arrays(DictionaryDatum & d) const
  {
    ConnectionHetWD::initialize_property_arrays(d);

    initialize_property_array(d, "cs");
    initialize_property_array(d, "ns");
  }

  /**
   * Append properties of this connection to the given dictionary. If the
   * dictionary is empty, new arrays are created first.
   */
  void STDPDopaConnection::append_properties(DictionaryDatum & d) const
  {
    ConnectionHetWD::append_properties(d);

    append_property<double_t>(d, "cs", c_);
    append_property<double_t>(d, "ns", n_);
  }

} // of namespace nest
