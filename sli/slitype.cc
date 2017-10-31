/*
 *  slitype.cc
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
    class implementing SLI types.
*/

#include "slitype.h"

// C++ includes:
#include <cstdlib>

void
SLIType::settypename( const std::string& s )
{
  if ( count == 0 )
  {
    assert( Name::lookup( s ) == false );
    name = new Name( s );
  }
  else
  {
    assert( Name( s ) == *name );
  }
  ++count;
}

void
SLIType::deletetypename( void )
{
  assert( count > 0 );
  if ( count == 1 )
  {
    delete name;
  }
  --count;
}


void
SLIType::setdefaultaction( SLIFunction& c )
{
  if ( defaultaction == NULL )
  {
    defaultaction = &c;
  }
  else
  {
    assert( &c == defaultaction );
  }
}
