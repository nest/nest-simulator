/*
 *  slistartup.h
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

#ifndef SLISTARTUP_H
#define SLISTARTUP_H

#include <string>
#include "sliconfig.h"
#include "slimodule.h" 
#include "token.h"
#include "slifunction.h"
#include "name.h"
#include "compose.hpp"

#include "dirent.h"
#include "errno.h"

class SLIStartup: public SLIModule
{
  const std::string startupfilename;
  const std::string slilibpath;
  std::string slihomepath;
  std::string slidocdir;

  std::string locateSLIInstallationPath(void);
  bool checkpath(std::string const &, std::string &) const;
  std::string getenv(const std::string &) const;
  std::string checkenvpath(std::string const &, SLIInterpreter *, std::string) const;

  Token targs;
  int verbosity_;
  bool debug_;
  public:

  Name argv_name;
  Name prgname_name;
  Name exitcode_name;
  Name prgmajor_name;
  Name prgminor_name;
  Name prgpatch_name;
  Name prgbuilt_name;
  Name prefix_name;
  Name prgsourcedir_name;
  Name prgbuilddir_name;
  Name prgdatadir_name;
  Name prgdocdir_name;

  Name host_name;
  Name hostos_name;
  Name hostvendor_name;
  Name hostcpu_name;
  
  Name getenv_name;
  Name statusdict_name;
  Name start_name;

  Name intsize_name;
  Name longsize_name;
  Name havelonglong_name;
  Name longlongsize_name;
  Name doublesize_name;
  Name pointersize_name;
  Name architecturedict_name;

  Name platform_name;
  Name threading_name;

  Name have_mpi_name;
  Name ismpi_name;
  Name have_gsl_name;
  Name have_pthreads_name;
  Name havemusic_name;
  Name ndebug_name;

  Name exitcodes_name;
  Name exitcode_success_name;
  Name exitcode_scripterror_name;
  Name exitcode_abort_name;
  Name exitcode_userabort_name;
  Name exitcode_segfault_name;
  Name exitcode_exception_name;
  Name exitcode_fatal_name;
  Name exitcode_unknownerror_name;

  Name environment_name;
  
  class GetenvFunction: public SLIFunction
  {
    public:
    void execute(SLIInterpreter *) const;
  };

  GetenvFunction getenvfunction;

  SLIStartup(int, char**);
  ~SLIStartup(){}

  void init(SLIInterpreter *);

  const std::string name(void) const
    {
      return "SLIStartup";
    }
};

#endif
