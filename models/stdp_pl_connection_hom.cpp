
/*
 *  stdp_pl_connection_hom.cpp
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
#include "stdp_pl_connection_hom.h"
#include "event.h"

namespace nest
{
  //
  // Implementation of class STDPPLHomCommonProperties.
  //

  STDPPLHomCommonProperties::STDPPLHomCommonProperties() :
    CommonSynapseProperties(),
    tau_plus_(20.0),
    lambda_(0.1),
    alpha_(1.0),
    mu_(0.4)
  { }

  void STDPPLHomCommonProperties::get_status(DictionaryDatum & d) const
  {
    CommonSynapseProperties::get_status(d);

    def<double_t>(d, "tau_plus", tau_plus_);
    def<double_t>(d, "lambda", lambda_);
    def<double_t>(d, "alpha", alpha_);
    def<double_t>(d, "mu", mu_);
  }
  
  void STDPPLHomCommonProperties::set_status(const DictionaryDatum & d, ConnectorModel &cm)
  {
    CommonSynapseProperties::set_status(d, cm);

    updateValue<double_t>(d, "tau_plus", tau_plus_);
    updateValue<double_t>(d, "lambda", lambda_);
    updateValue<double_t>(d, "alpha", alpha_);
    updateValue<double_t>(d, "mu", mu_);
  }


  //
  // Implementation of class STDPPLConnectionHom.
  //

  STDPPLConnectionHom::STDPPLConnectionHom() :
    Kplus_(0.0)
  { }

  STDPPLConnectionHom::STDPPLConnectionHom(const STDPPLConnectionHom &rhs) :
    ConnectionHetWD(rhs)
  {
    Kplus_ = rhs.Kplus_;
  }

  void STDPPLConnectionHom::get_status(DictionaryDatum & d) const
  {

    // base class properties, different for individual synapse
    ConnectionHetWD::get_status(d);

    // own properties, different for individual synapse
    def<double_t>(d, "Kplus", Kplus_);
  }
  
  void STDPPLConnectionHom::set_status(const DictionaryDatum & d, ConnectorModel &cm)
  {
    // base class properties
    ConnectionHetWD::set_status(d, cm);

//     if (d->known("tau_plus") ||
// 	d->known("lambd") ||
// 	d->known("alpha") ||
// 	d->known("mu_plus") ||
// 	d->known("mu_minus") ||
// 	d->known("Wmax") )
//       {
// 	cm.network().error("STDPPLConnectionHom::set_status()", "you are trying to set common properties via an individual synapse.");
//       }

    updateValue<double_t>(d, "Kplus", Kplus_);    
  }

   /**
   * Set properties of this connection from position p in the properties
   * array given in dictionary.
   */  
  void STDPPLConnectionHom::set_status(const DictionaryDatum & d, index p, ConnectorModel &cm)
  {
    ConnectionHetWD::set_status(d, p, cm);
  
     if (d->known("tau_pluss") ||
	d->known("lambdas") ||
	d->known("alphas") ||
	 d->known("mus")) 
      {
	cm.network().message(SLIInterpreter::M_ERROR, "STDPPLConnectionHom::set_status()", "you are trying to set common properties via an individual synapse.");
      }

    set_property<double_t>(d, "Kpluss", p, Kplus_);
  }

  void STDPPLConnectionHom::initialize_property_arrays(DictionaryDatum & d) const
  {
    ConnectionHetWD::initialize_property_arrays(d);
    initialize_property_array(d, "Kpluss");
  }
  

  /**
   * Append properties of this connection to the given dictionary. If the
   * dictionary is empty, new arrays are created first.
   */
  void STDPPLConnectionHom::append_properties(DictionaryDatum & d) const
  {
    std::cout << "ConnectionHetWD call..." << std::endl;
    ConnectionHetWD::append_properties(d);
    std::cout << "ConnectionHetWD complete" << std::endl;
    append_property<double_t>(d, "Kpluss", Kplus_);
  }

} // of namespace nest
