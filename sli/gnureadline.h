/*
 *  gnureadline.h
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

#ifndef GNUREADLINE_H
#define GNUREADLINE_H

// Generated includes:
#include "config.h"

#ifdef HAVE_READLINE

// Includes from sli:
#include "slimodule.h"
#include "slitype.h"

class GNUReadline : public SLIModule
{
public:
  GNUReadline()
  {
  }
  ~GNUReadline();

  const std::string
  name( void ) const
  {
    return "GNUReadline";
  }

  void init( SLIInterpreter* );

  class GNUReadlineFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class GNUAddhistoryFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  GNUReadlineFunction gnureadlinefunction;
  GNUAddhistoryFunction gnuaddhistoryfunction;
};

#endif // HAVE_READLINE

#endif
