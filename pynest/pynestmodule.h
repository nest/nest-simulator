/*
 *  pynestmodule.h
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

#ifndef PYNESTMODULE_H
#define PYNESTMODULE_H
/* 
    This file is part of NEST

    pynestmodule.h -- Header to the pynestmodule
    (see cpp file for details)

    Author: Marc-Oliver Gewaltig (marc-oliver.gewaltig@honda-ri.de)
            Hans Ekkehard Plesser (hans.ekkehard.plesser@umb.no)

    $Date: 2008-04-10 04:47:03 +0200 (Thu, 10 Apr 2008) $
    Last change: $Author: plesser $
    $Revision: 7066 $
*/

#include "slimodule.h"
#include "slitype.h"
#include "network.h"

namespace nest
{
  class Network;
  
  /**
   * Module supplying neuron models only available in PyNEST. 
   * @note Only neuron models that depend on the presence of Python
   *       for technical reasons should be placed in this module.
   */
  class PynestModule: public SLIModule
   {
    public:

     PynestModule(Network&);
     ~PynestModule();

     /**
      * Initialize module by registering models with the network.
      * @param SLIInterpreter* SLI interpreterm, must know modeldict
      * @param nest::Network&  Network with which to register models
      */
     void init(SLIInterpreter*);

     const std::string name(void) const;
     const std::string commandstring(void) const;

     static SLIType Pyobjecttype;
     
   private:
     
     //! network where models are to be registered
     Network& net_;
   };

} // namespace

#endif
