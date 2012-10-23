/*
 *  slimathlink.h
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
#ifndef MATHINTERFACE_H
#define MATHINTERFACE_H
#include "slitype.h"
#include "slimodule.h" 
#include <string>

class MLInterface: public SLIModule
{
  public:
  
  MLInterface(){}

  const std::string name(void) const
      {
	  return "MathLink";
      }
  
  void init(SLIInterpreter *);
    
  class MathLinkOpenFunction: public SLIFunction  
      {
      public:
	  MathLinkOpenFunction() {}
	  void execute(SLIInterpreter *) const;
      };
  
  class MathLinkCloseFunction: public SLIFunction
      {
      public:
	  MathLinkCloseFunction() {}
	  void execute(SLIInterpreter *) const;
      };

  class MathLinkFlushFunction: public SLIFunction
      {
      public:
	  MathLinkFlushFunction() {}
	  void execute(SLIInterpreter *) const;
      };
  
  class MathLinkGetStringFunction: public SLIFunction  
      {
      public:
	  MathLinkGetStringFunction() {}
	  void execute(SLIInterpreter *) const;
      };
  
  class MathLinkPutStringFunction: public SLIFunction
      {
      public:
	  MathLinkPutStringFunction() {}
	  void execute(SLIInterpreter *) const;
      };

  const MathLinkOpenFunction       mathlinkopenfunction;
  const MathLinkCloseFunction      mathlinkclosefunction;
  const MathLinkFlushFunction      mathlinkflushfunction;
  const MathLinkPutStringFunction  mathlinkputstringfunction;
  const MathLinkGetStringFunction  mathlinkgetstringfunction;
  
};
#endif







