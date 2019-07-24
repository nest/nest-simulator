/*
 *  filesystem.cc
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

#include "filesystem.h"

// C includes:
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// C++ includes:
#include <cassert>
#include <ctime>
#include <fstream>
#include <mutex>
// Includes from libnestutil:
#include "compose.hpp"

// Includes from sli:
#include "arraydatum.h"
#include "stringdatum.h"


void
FilesystemModule::init( SLIInterpreter* i )
{
  i->createcommand( "FileNames_", &filenamesfunction );
  i->createcommand( "SetDirectory_", &setdirectoryfunction );
  i->createcommand( "Directory", &directoryfunction );
  i->createcommand( "MoveFile_", &movefilefunction );
  i->createcommand( "CopyFile_", &copyfilefunction );
  i->createcommand( "DeleteFile_", &deletefilefunction );
  i->createcommand( "MakeDirectory_", &makedirectoryfunction );
  i->createcommand( "RemoveDirectory_", &removedirectoryfunction );
  i->createcommand( "tmpnam", &tmpnamfunction );
  i->createcommand( "CompareFiles_s_s", &comparefilesfunction );
}


const std::string
FilesystemModule::name( void ) const
{
  return std::string( "Filesystem access" );
}

const std::string
FilesystemModule::commandstring( void ) const
{
  return std::string( "(filesystem.sli) run" );
}


void
FilesystemModule::FileNamesFunction::execute( SLIInterpreter* i ) const
{
  StringDatum* sd = dynamic_cast< StringDatum* >( i->OStack.top().datum() );
  assert( sd != NULL );

  DIR* TheDirectory = opendir( sd->c_str() );
  if ( TheDirectory != NULL )
  {
    ArrayDatum* a = new ArrayDatum();
    i->EStack.pop();
    i->OStack.pop();
    dirent* TheEntry;
    while ( ( TheEntry = readdir( TheDirectory ) ) != NULL )
    {
      Token string_token( new StringDatum( TheEntry->d_name ) );
      a->push_back_move( string_token );
    }
    Token array_token( a );
    i->OStack.push_move( array_token );
  }
  else
  {
    i->raiseerror( i->BadIOError );
  }
}

void
FilesystemModule::SetDirectoryFunction::execute( SLIInterpreter* i ) const
// string -> boolean
{
  StringDatum* sd = dynamic_cast< StringDatum* >( i->OStack.top().datum() );
  assert( sd != NULL );
  int s = chdir( sd->c_str() );
  i->OStack.pop();
  if ( not s )
  {
    i->OStack.push( i->baselookup( i->true_name ) );
  }
  else
  {
    i->OStack.push( i->baselookup( i->false_name ) );
  };
  i->EStack.pop();
}

/** @BeginDocumentation
 Name: Directory - Return current working directory
 Synopsis: Directory -> string
 Description: Returns name of current working directory. This is where all ls,
 filestream etc. operations are done per default.
 Parameters: string : Name of current working directory
 Examples: Directory = -> /home/MyAccount/SNiFF/synod2
 Bugs: -
 Author: Hehl
 FirstVersion: Oct 12th 1999
 Remarks:
 SeeAlso: FileNames, SetDirectory, MakeDirectory, RemoveDirectory, cd, ls
*/

void
FilesystemModule::DirectoryFunction::execute( SLIInterpreter* i ) const
{
  const int SIZE = 256; // incremental buffer size, somewhat arbitrary!

  int size = SIZE;
  char* path_buffer = new char[ size ];
  while ( getcwd( path_buffer, size - 1 ) == NULL )
  { // try again with a bigger buffer!
    if ( errno != ERANGE )
    {
      i->raiseerror( i->BadIOError ); // size wasn't reason
    }
    delete[] path_buffer;
    size += SIZE;
    path_buffer = new char[ size ];
    assert( path_buffer != NULL );
  }
  Token sd( new StringDatum( path_buffer ) );
  delete[]( path_buffer );
  i->OStack.push_move( sd );
  i->EStack.pop();
}

void
FilesystemModule::MoveFileFunction::execute( SLIInterpreter* i ) const
// string string -> boolean
{
  StringDatum* src = dynamic_cast< StringDatum* >( i->OStack.pick( 1 ).datum() );
  StringDatum* dst = dynamic_cast< StringDatum* >( i->OStack.pick( 0 ).datum() );
  assert( src != NULL );
  assert( dst != NULL );
  int s = link( src->c_str(), dst->c_str() );
  if ( not s )
  {
    s = unlink( src->c_str() );
    if ( s ) // failed to remove old link: undo everything
    {
      int t = unlink( dst->c_str() );
      assert( t == 0 ); // link was just created after all!
    };
  };
  i->OStack.pop( 2 );
  if ( not s )
  {
    i->OStack.push( i->baselookup( i->true_name ) );
  }
  else
  {
    i->OStack.push( i->baselookup( i->false_name ) );
  };
  i->EStack.pop();
}

void
FilesystemModule::CopyFileFunction::execute( SLIInterpreter* i ) const
// string string -> -
{
  StringDatum* src = dynamic_cast< StringDatum* >( i->OStack.pick( 1 ).datum() );
  StringDatum* dst = dynamic_cast< StringDatum* >( i->OStack.pick( 0 ).datum() );
  assert( src != NULL );
  assert( dst != NULL );

  std::ofstream deststream( dst->c_str() );
  if ( not deststream )
  {
    i->message( SLIInterpreter::M_ERROR, "CopyFile", "Could not create destination file." );
    i->raiseerror( i->BadIOError );
    return;
  }

  std::ifstream sourcestream( src->c_str() );
  if ( not sourcestream )
  {
    i->message( SLIInterpreter::M_ERROR, "CopyFile", "Could not open source file." );
    i->raiseerror( i->BadIOError );
    return;
  }

  // copy while file in one call (see Josuttis chap 13.9 (File Access), p. 631)
  deststream << sourcestream.rdbuf();

  if ( not deststream )
  {
    i->message( SLIInterpreter::M_ERROR, "CopyFile", "Error copying file." );
    i->raiseerror( i->BadIOError );
    return;
  }

  i->OStack.pop( 2 );
  i->EStack.pop();

} // closes files automatically


void
FilesystemModule::DeleteFileFunction::execute( SLIInterpreter* i ) const
// string -> boolean
{
  StringDatum* sd = dynamic_cast< StringDatum* >( i->OStack.top().datum() );
  assert( sd != NULL );
  int s = unlink( sd->c_str() );
  i->OStack.pop();
  if ( not s )
  {
    i->OStack.push( i->baselookup( i->true_name ) );
  }
  else
  {
    i->OStack.push( i->baselookup( i->false_name ) );
  };
  i->EStack.pop();
}

void
FilesystemModule::MakeDirectoryFunction::execute( SLIInterpreter* i ) const
// string -> Boolean
{
  StringDatum* sd = dynamic_cast< StringDatum* >( i->OStack.top().datum() );
  assert( sd != NULL );
  int s = mkdir( sd->c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP );
  i->OStack.pop();
  if ( not s )
  {
    i->OStack.push( i->baselookup( i->true_name ) );
  }
  else
  {
    i->OStack.push( i->baselookup( i->false_name ) );
  };
  i->EStack.pop();
}

void
FilesystemModule::RemoveDirectoryFunction::execute( SLIInterpreter* i ) const
// string -> Boolean
{
  StringDatum* sd = dynamic_cast< StringDatum* >( i->OStack.top().datum() );
  assert( sd != NULL );
  int s = rmdir( sd->c_str() );
  i->OStack.pop();
  if ( not s )
  {
    i->OStack.push( i->baselookup( i->true_name ) );
  }
  else
  {
    i->OStack.push( i->baselookup( i->false_name ) );
  };
  i->EStack.pop();
}

/** @BeginDocumentation
   Name: tmpnam - Generate a string that is a valid non-existing file-name.

   Synopsis: tpmnam -> filename

   Description:
   This command is a thin wrapper around the POSIX.1 tmpnam() kernel function.
   The tmpnam function generates a string that is a valid filename and
   that is not the name of an existing file. The tmpnam function
   generates a different name each time it is called, (up to a number of
   times that is defined by the compiler macro TMP_MAX).

   Author: R. Kupper

   References: Donald Lewin, "The POSIX Programmer's Guide"

*/
std::mutex mtx;
void
FilesystemModule::TmpNamFunction::execute( SLIInterpreter* i ) const
{
  std::lock_guard< std::mutex > lock( mtx );
  static unsigned int seed = std::time( 0 );
  char* env = getenv( "TMPDIR" );
  std::string tmpdir( "/tmp" );
  if ( env )
  {
    tmpdir = std::string( env );
  }

  std::string tempfile;
  do
  {
    int rng = rand_r( &seed );
    tempfile = tmpdir + String::compose( "/nest-tmp-%1", rng );
  } while ( std::ifstream( tempfile.c_str() ) );

  Token filename_t( new StringDatum( tempfile ) );

  i->OStack.push_move( filename_t );
  i->EStack.pop();
}

/** @BeginDocumentation
   Name: CompareFiles - Compare two files for equality.

   Synopsis: filenameA filenameB CompareFiles -> bool

   Description:
   This command compares the two files named and returns true if they
   have identical content. Files are read in binary mode. FileOpenError
   is raised if one of the files cannot be opened.

   Author: Hans E Plesser
*/
void
FilesystemModule::CompareFilesFunction::execute( SLIInterpreter* i ) const
{
  i->assert_stack_load( 2 );

  StringDatum const* const flA = dynamic_cast< StringDatum* >( i->OStack.pick( 1 ).datum() );
  StringDatum const* const flB = dynamic_cast< StringDatum* >( i->OStack.pick( 0 ).datum() );
  assert( flA );
  assert( flB );

  std::ifstream as( flA->c_str(), std::ifstream::in | std::ifstream::binary );
  std::ifstream bs( flB->c_str(), std::ifstream::in | std::ifstream::binary );

  if ( not( as.good() && bs.good() ) )
  {
    as.close();
    bs.close();
    throw IOError();
  }

  bool equal = true;
  while ( equal && as.good() && bs.good() )
  {
    const int ac = as.get();
    const int bc = bs.get();

    if ( not( as.fail() || bs.fail() ) )
    {
      equal = ac == bc;
    }
  }

  if ( as.fail() != bs.fail() )
  {
    equal = false; // different lengths
  }

  as.close();
  bs.close();

  i->OStack.pop( 2 );
  if ( equal )
  {
    i->OStack.push( i->baselookup( i->true_name ) );
  }
  else
  {
    i->OStack.push( i->baselookup( i->false_name ) );
  }

  i->EStack.pop();
}
