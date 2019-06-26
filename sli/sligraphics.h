/*
 *  sligraphics.h
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

#ifndef SLIPGM_H
#define SLIPGM_H

// C++ includes:
#include <iostream>
#include <string>
#include <vector>

// Includes from sli:
#include "fdstream.h"
#include "interpret.h"
#include "slifunction.h"
#include "slimodule.h"
#include "stringdatum.h"

class SLIgraphics : public SLIModule
{

  class ReadPGMFunction : public SLIFunction
  {
  private:
    std::istream* openPGMFile( StringDatum* ) const;        //!< opens the file
    void readMagicNumber( std::istream*, char[ 2 ] ) const; //!< reads the magic number into string magic
    void initRead( std::istream*, int&, int&, int& ) const; //!< reads width, height, maxval
    void readImage( std::istream*, char[ 2 ], std::vector< long >&, int, int, int ) const; //!< reads the image

  public:
    virtual void execute( SLIInterpreter* ) const;
  };

  class WritePGMFunction : public SLIFunction
  {
  public:
    virtual void execute( SLIInterpreter* ) const;
  };

  ReadPGMFunction readpgmfunction;
  WritePGMFunction writepgmfunction;

public:
  SLIgraphics()
  {
  }

  void init( SLIInterpreter* );
  const std::string name( void ) const;
  const std::string commandstring( void ) const;
};


#endif
