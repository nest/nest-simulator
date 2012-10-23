/*
 *  matlabeng.cc
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


#include "matlabeng.h"
#include <cassert>


MatlabEngine::MatlabEngine(string const & s, int n)
  : buffer(NULL)
{
 ep = engOpen(s.c_str());
 alive = (ep!=NULL);
 
 if (alive)
  setoutputbuffer(n);
}

MatlabEngine::~MatlabEngine()
{
 if (alive)
  engClose(ep);
 delete [] buffer;
}

bool MatlabEngine::good(void) const
{
 return alive;
}

MatlabEngine::operator bool() const
{
 return good();
}

bool MatlabEngine::evalstring(string const & s)
{
 assert(alive);

 alive = (engEvalString(ep,s.c_str())==0);
 return good();
}

bool MatlabEngine::setoutputbuffer(int n)
{
 assert(alive);

 delete [] buffer;
 buffer = new char[n];
 alive = (buffer!=NULL);
 if (alive)
  engOutputBuffer(ep,buffer,n);

 return alive;
}

string MatlabEngine::getoutputbuffer(void) const
{
 assert(alive);

 return string(buffer);
}




