/*
 *  fdstreamtest.cc
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

/*
  Test whether fdstream work:

  open and close file
  write to file
  read back same file

  Note: not currently built by default, do it by hand.

  Hans Ekkehard Plesser, 2003-03-21
*/

// C++ includes:
#include <iostream>
#include <string>

// Includes from sli:
#include "fdstream.h"

int
main()
{
  const char TESTFILE[] = "__sli__fdtest.dat";

  // check that we can open and close a file
  fdstream t1( TESTFILE, std::ios_base::out );

  assert( t1.is_open() );
  assert( t1.good() );
  assert( !t1.fail() );

  t1.close();

  assert( !t1.fail() );
  assert( !t1.is_open() );


  // check that we can write to a file
  fdstream o( TESTFILE, std::ios_base::out );

  if ( o.good() )
    std::cerr << "Write/open: Good." << std::endl;
  else
  {
    perror( NULL );
    std::cerr << "Write/open: Error!" << std::endl;
  }

  o << "Line 1" << std::endl;
  o << "Line 2" << std::endl << "Line 3" << std::endl;
  o.close();

  if ( o.good() )
    std::cerr << "Write/close: Good." << std::endl;
  else
  {
    perror( NULL );
    std::cerr << "Write/close: Error!" << std::endl;
    return -1;
  }

  // check if we can read
  ifdstream i( TESTFILE, std::ios_base::in );
  // ifstream i(TESTFILE, std::ios_base::in);

  if ( i.good() )
    std::cerr << "Read/open: Good." << std::endl;
  else if ( i.eof() )
  {
    std::cerr << "Read/open: at EOF." << std::endl;
    return -2;
  }
  else
  {
    perror( NULL );
    std::cerr << "Read/open: Error!" << std::endl;
    return -1;
  }

  std::string s;
  while ( !i.eof() )
  {
    s.clear();
    i >> s;

    std::cerr << "Read: " << s.size() << " chars." << std::endl;

    if ( i.eof() )
      std::cout << "End of file reached." << std::endl;
    else if ( !i.fail() )
      std::cout << "Read:|" << s << "|" << std::endl;
    else
    {
      perror( NULL );
      std::cerr << "Read/reading: Error!" << std::endl;
      return -1;
    }
  }

  i.clear();
  i.close();

  if ( i.good() )
    std::cerr << "Read/close: Good." << std::endl;
  else
  {
    perror( NULL );
    std::cerr << "Read/close: Error!" << std::endl;
    return -1;
  }
}
