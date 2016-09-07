/*
 *  sliregexp.h
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

#ifndef SLIREGEXP_H
#define SLIREGEXP_H
/*
    SLI's array access functions
*/

// C includes:
#include <regex.h>
#include <sys/types.h>

// Includes from sli:
#include "lockptrdatum.h"
#include "slifunction.h"
#include "slimodule.h"

class Regex
{
  regex_t r;

public:
  Regex();
  ~Regex();
  regex_t* get( void );
};

class RegexpModule : public SLIModule
{
  class RegcompFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };
  class RegexecFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };
  class RegerrorFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

public:
  RegcompFunction regcompfunction;
  RegexecFunction regexecfunction;
  RegerrorFunction regerrorfunction;

  const Name regexdict_name;

  const Name REG_NOTBOL_name;
  const Name REG_NOTEOL_name;
  const Name REG_ESPACE_name;
  const Name REG_BADPAT_name;
  const Name REG_EXTENDED_name;
  const Name REG_ICASE_name;
  const Name REG_NOSUB_name;
  const Name REG_NEWLINE_name;
  const Name REG_ECOLLATE_name;
  const Name REG_ECTYPE_name;
  const Name REG_EESCAPE_name;
  const Name REG_ESUBREG_name;
  const Name REG_EBRACK_name;
  const Name REG_EPAREN_name;
  const Name REG_EBRACE_name;
  const Name REG_BADBR_name;
  const Name REG_ERANGE_name;
  const Name REG_BADRPT_name;

  static SLIType RegexType;

  RegexpModule( void )
    : regexdict_name( "regexdict" )
    , REG_NOTBOL_name( "REG_NOTBOL" )
    , REG_NOTEOL_name( "REG_NOTEOL" )
    , REG_ESPACE_name( "REG_ESPACE" )
    , REG_BADPAT_name( "REG_BADPAT" )
    , REG_EXTENDED_name( "REG_EXTENDED" )
    , REG_ICASE_name( "REG_ICASE" )
    , REG_NOSUB_name( "REG_NOSUB" )
    , REG_NEWLINE_name( "REG_NEWLINE" )
    , REG_ECOLLATE_name( "REG_ECOLLATE" )
    , REG_ECTYPE_name( "REG_ECTYPE" )
    , REG_EESCAPE_name( "REG_EESCAPE" )
    , REG_ESUBREG_name( "REG_ESUBREG" )
    , REG_EBRACK_name( "REG_EBRACK" )
    , REG_EPAREN_name( "REG_EPAREN" )
    , REG_EBRACE_name( "REG_EBRACE" )
    , REG_BADBR_name( "REG_BADBR" )
    , REG_ERANGE_name( "REG_ERANGE" )
    , REG_BADRPT_name( "REG_BADRPT" )
  {
  }

  ~RegexpModule();

  void init( SLIInterpreter* );
  const std::string name( void ) const;
  const std::string commandstring( void ) const;
};

#endif
