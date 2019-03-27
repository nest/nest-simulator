/*
 *  multimeter.cpp
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

#include "voltmeter.h"

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"

namespace nest
{
Voltmeter::Voltmeter()
  : Multimeter()
{
    //record_from_.push_back( names::V_m );
  DictionaryDatum vmdict = DictionaryDatum( new Dictionary );
  ArrayDatum ad;
  ad.push_back( LiteralDatum( names::V_m.toString() ) );
  ( *vmdict )[ names::record_from ] = ad;
  set_status( vmdict );
}

Voltmeter::Voltmeter( const Voltmeter& n )
  : Multimeter( n )
{
  /*DictionaryDatum vmdict;
  n.get_status(vmdict);
  assert(vmdict[names::record_from] == names::V_m.toString());*/
}

/*nest::Voltmeter::Parameters_::Parameters_( const Parameters_& p )
  : interval_( p.interval_ )
  , offset_( p.offset_ )
  , record_from_( p.record_from_ )
{
  assert (p.record_from_.size() == 1 
          && p.record_from_[0] == names::V_m );
  interval_.calibrate();
}*/

}

