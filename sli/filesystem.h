/*
 *  filesystem.h
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

#ifndef FILESYSTEM_H
#define FILESYSTEM_H
/**************************************
  Functions related to the filesystem.
  SLI's stream I/O functions are located in
  sli_io.h.
  *************************************/

// Includes from sli:
#include "slifunction.h"
#include "slimodule.h"


class FilesystemModule : public SLIModule
{
  class FileNamesFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class SetDirectoryFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class DirectoryFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class MoveFileFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class CopyFileFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class DeleteFileFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class MakeDirectoryFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class RemoveDirectoryFunction : public SLIFunction
  {
  public:
    void execute( SLIInterpreter* ) const;
  };

  class TmpNamFunction : public SLIFunction
  {
  public:
    TmpNamFunction()
    {
    }
    void execute( SLIInterpreter* ) const;
  };

  class CompareFilesFunction : public SLIFunction
  {
  public:
    CompareFilesFunction()
    {
    }
    void execute( SLIInterpreter* ) const;
  };

public:
  FileNamesFunction filenamesfunction;
  SetDirectoryFunction setdirectoryfunction;
  DirectoryFunction directoryfunction;
  MoveFileFunction movefilefunction;
  CopyFileFunction copyfilefunction;
  DeleteFileFunction deletefilefunction;
  MakeDirectoryFunction makedirectoryfunction;
  RemoveDirectoryFunction removedirectoryfunction;
  TmpNamFunction tmpnamfunction;
  CompareFilesFunction comparefilesfunction;

  FilesystemModule( void ){};
  ~FilesystemModule(){};

  void init( SLIInterpreter* );
  const std::string name( void ) const;
  const std::string commandstring( void ) const;
};

#endif
