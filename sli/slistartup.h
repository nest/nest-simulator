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

// C++ includes:
#include <string>

// Generated includes:
#include "dirent.h"
#include "errno.h"
#include "config.h"

// Includes from libnestutil:
#include "compose.hpp"

// Includes from sli:
#include "name.h"
#include "slifunction.h"
#include "slimodule.h"
#include "token.h"

// Exit codes
#define EXITCODE_UNKNOWN_ERROR 10
#define EXITCODE_USERABORT 15
#define EXITCODE_EXCEPTION 125
#define EXITCODE_SCRIPTERROR 126
#define EXITCODE_FATAL 127

// The range 200-215 is reserved for test skipping exitcodes. Any new codes must
// also be added to testsuite/do_tests_sh.in.
#define EXITCODE_SKIPPED 200
#define EXITCODE_SKIPPED_NO_MPI 201
#define EXITCODE_SKIPPED_HAVE_MPI 202
#define EXITCODE_SKIPPED_NO_THREADING 203
#define EXITCODE_SKIPPED_NO_GSL 204
#define EXITCODE_SKIPPED_NO_MUSIC 205
#define EXITCODE_SKIPPED_NO_RECORDINGBACKEND_ARBOR 206


class SLIStartup : public SLIModule
{
  const std::string startupfilename;
  const std::string slilibpath;
  std::string slilibdir;
  std::string slidocdir;
  std::string sliprefix;

  std::string locateSLIInstallationPath( void );
  bool checkpath( std::string const&, std::string& ) const;
  std::string getenv( const std::string& ) const;
  std::string checkenvpath( std::string const&, SLIInterpreter*, std::string ) const;

  Token targs;
  int verbosity_;
  bool debug_;

public:
  Name argv_name;
  Name version_name;
  Name exitcode_name;
  Name prgbuilt_name;
  Name prefix_name;
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
  Name have_music_name;
  Name have_recordingbackend_arbor_name;
  Name have_libneurosim_name;
  Name have_sionlib_name;
  Name ndebug_name;

  Name exitcodes_name;
  Name exitcode_success_name;
  Name exitcode_skipped_name;
  Name exitcode_skipped_no_mpi_name;
  Name exitcode_skipped_have_mpi_name;
  Name exitcode_skipped_no_threading_name;
  Name exitcode_skipped_no_gsl_name;
  Name exitcode_skipped_no_music_name;
  Name exitcode_skipped_no_recordingbackend_arbor_name;
  Name exitcode_scripterror_name;
  Name exitcode_abort_name;
  Name exitcode_userabort_name;
  Name exitcode_segfault_name;
  Name exitcode_exception_name;
  Name exitcode_fatal_name;
  Name exitcode_unknownerror_name;

  Name environment_name;

  class GetenvFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  GetenvFunction getenvfunction;

  SLIStartup( int, char** );
  ~SLIStartup()
  {
  }

  void init( SLIInterpreter* );

  const std::string
  name( void ) const
  {
    return "SLIStartup";
  }
};

#endif
