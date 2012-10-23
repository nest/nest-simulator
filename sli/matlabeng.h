/*
 *  matlabeng.h
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


// ********************************************************************
// * Interface to matlab Engine                             
// * --------------------------
// *
// *  compiles with:
// * 
// *  setenv LD_LIBRARY_PATH /opt/matlab5/extern/lib/lnx86/
// *  g++ -Wall -ansi -I /opt/matlab5/extern/include -o engdemo engdemo.cc  
// *            -L /opt/matlab5/extern/lib/lnx86 -leng -lmx
// *
// * based on:  engdemo.c (Mathworks)
// *
// * History:
// *         (0) first version
// *            3.4.1998, Diesmann, Freiburg
// *
// ********************************************************************


#include <string>
#ifdef HAVE_CONFIG_H
#include "Config.h"
#endif
#ifdef HAVE_MATLAB
#include "engine.h"
#endif

class MatlabEngine
{
 Engine *ep;
 bool alive;

 char *buffer;

 public:

  MatlabEngine(string const & =string(), int =256);
  ~MatlabEngine();
  bool good(void) const;
  operator bool() const;
  bool evalstring(string const &);
  bool setoutputbuffer(int);
  string getoutputbuffer(void) const;
};
