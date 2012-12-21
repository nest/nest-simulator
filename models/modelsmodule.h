/*
 *  modelsmodule.h
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

#ifndef MODELSMODULE_H
#define MODELSMODULE_H
/* 
    This file is part of NEST

    modelmodule.h -- Header to the modelmodule
    (see cpp file for details)


    Author: Marc-Oliver Gewaltig (marc-oliver.gewaltig@honda-ri.de)
            Hans Ekkehard Plesser (hans.ekkehard.plesser@umb.no)

    $Date: 2012-11-15 17:09:23 +0100 (Thu, 15 Nov 2012) $
    Last change: $Author: eppler $
    $Revision: 9952 $
*/

#include "slimodule.h"

namespace nest
{
  class Network;
  
  /**
   * Module supplying all models that are included in the NEST release. 
   * @todo Should this be a dynamic module?
   */
   class ModelsModule: public SLIModule
   {
    public:

     ModelsModule(Network&);
     ~ModelsModule();

     /**
      * Initialize module by registering models with the network.
      * @param SLIInterpreter* SLI interpreter
      * @param nest::Network&  Network with which to register models
      */
     void init(SLIInterpreter*);

     const std::string name(void) const;
     const std::string commandstring(void) const;

   private:
     
     //! network where models are to be registered
     Network& net_;
   };


} // namespace

#endif
