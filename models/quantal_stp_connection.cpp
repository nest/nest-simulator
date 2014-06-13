/*
 *  quantal_stp_connection.cpp
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

#include "quantal_stp_connection.h"
#include "network.h"
#include "connection.h"
#include "connector_model.h"
#include "nest_names.h"
#include "dictutils.h"

namespace nest
{
  

  /**
   * Polymorphic version of set_property.
   * This version is needed, because DataConnect will pass all properties as doubles. 
   * This code will take either an int or a double and convert is to an int. 
   */

  bool set_property_int(const DictionaryDatum & d, Name propname, index p, int &prop)
  {
    if (d->known(propname))
      {
	ArrayDatum* arrd = dynamic_cast<ArrayDatum*>((*d)[propname].datum());
	if (! arrd)
	  {
	    ArrayDatum const arrd;
	    throw TypeMismatch(arrd.gettypename().toString(), 
			       (*d)[propname].datum()->gettypename().toString());
	  }
	
	Datum  *dat= (*arrd)[p].datum();
	IntegerDatum *intdat= dynamic_cast<IntegerDatum *>(dat);
	if(intdat !=0)
	  {
	    prop = static_cast<int>(intdat->get());
	    return true;
	  }
	DoubleDatum *doubledat= dynamic_cast<DoubleDatum *>(dat);
	if(doubledat !=0)
	  {
	    prop = static_cast<int>(doubledat->get());
	    return true;
	  }
	else
	  throw TypeMismatch();
      }
    
    return false;
  }

  /* Polymorphic version of update_value.
   * This version is needed, because DataConnect will pass all properties as doubles. 
   * This code will take either an int or a double and convert is to an int. 
   */

  bool update_value_int(const DictionaryDatum & d, Name propname, int &prop)
  {
    if (d->known(propname))
      {
	Datum  *dat= (*d)[propname].datum();
	IntegerDatum *intdat= dynamic_cast<IntegerDatum *>(dat);
	if(intdat !=0)
	  {
	    prop = static_cast<int>(intdat->get());
	    return true;
	  }
	DoubleDatum *doubledat= dynamic_cast<DoubleDatum *>(dat);
	if(doubledat !=0)
	  {
	    prop = static_cast<int>(doubledat->get());
	    return true;
	  }
	else
	  throw TypeMismatch();
      }
    
    return false;
  }

  Quantal_StpConnection::Quantal_StpConnection() :
    ConnectionHetWD(),
    U_(0.5),
    u_(U_),
    tau_rec_(800.0),
    tau_fac_(10.0),
    n_(1),
    a_(n_)

  {
  }

  Quantal_StpConnection::Quantal_StpConnection(const Quantal_StpConnection &rhs) :
    ConnectionHetWD(rhs),
    U_(rhs.U_),
    u_(rhs.u_),
    tau_rec_(rhs.tau_rec_),
    tau_fac_(rhs.tau_fac_),
    n_(rhs.n_),
    a_(rhs.a_)
  {
  }


  void Quantal_StpConnection::get_status(DictionaryDatum & d) const
  {
    ConnectionHetWD::get_status(d);
    def<double_t>(d, names::dU, U_);
    def<double_t>(d, names::u, u_);
    def<double_t>(d, names::tau_rec, tau_rec_);
    def<double_t>(d, names::tau_fac, tau_fac_);
    def<int>(d, names::n, n_); 
    def<int>(d, names::a, a_); 
  }
 

  void Quantal_StpConnection::set_status(const DictionaryDatum & d, ConnectorModel &cm)
  {
      ConnectionHetWD::set_status(d, cm);
    
      updateValue<double_t>(d, names::dU, U_);
      updateValue<double_t>(d, names::u, u_);
      updateValue<double_t>(d, names::tau_rec, tau_rec_);
      updateValue<double_t>(d, names::tau_fac, tau_fac_);
      updateValue<double_t>(d, names::weight, weight_); 
      update_value_int(d, names::n, n_);
      update_value_int(d, names::a, a_);
  }

  /**
   * Set properties of this connection from position p in the properties
   * array given in dictionary.
   */  
  void Quantal_StpConnection::set_status(const DictionaryDatum & d, index p, ConnectorModel &cm)
  {
    ConnectionHetWD::set_status(d, p, cm);

    set_property<double_t>(d, names::dUs, p, U_);
    set_property<double_t>(d, names::us, p, u_);
    set_property<double_t>(d, names::tau_recs, p, tau_rec_);
    set_property<double_t>(d, names::tau_facs, p, tau_fac_);
    set_property_int(d, names::ns, p, n_); 
    set_property_int(d, names::as, p, a_); 
 }

  void Quantal_StpConnection::initialize_property_arrays(DictionaryDatum & d) const
  {
    ConnectionHetWD::initialize_property_arrays(d);

    initialize_property_array(d, names::dUs); 
    initialize_property_array(d, names::us); 
    initialize_property_array(d, names::tau_recs);  
    initialize_property_array(d, names::tau_facs);  
    initialize_property_array(d, names::ns); 
    initialize_property_array(d, names::as); 
  }

  /**
   * Append properties of this connection to the given dictionary. If the
   * dictionary is empty, new arrays are created first.
   */
  void Quantal_StpConnection::append_properties(DictionaryDatum & d) const
  {
    ConnectionHetWD::append_properties(d);

    append_property<double_t>(d, names::dUs, U_); 
    append_property<double_t>(d, names::us, u_); 
    append_property<double_t>(d, names::tau_recs, tau_rec_);  
    append_property<double_t>(d, names::tau_facs, tau_fac_);  
    append_property<int>(d, names::ns, n_); 
    append_property<int>(d, names::as, a_); 
  }

} // of namespace nest
