/*
 *  fdstream.cc
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

#include "fdstream.h"

// Generated includes:
#include "config.h"

#ifdef HAVE_ISTREAM

std::streamsize const fdbuf::s_bufsiz;

fdbuf*
fdbuf::open( const char* s, std::ios_base::openmode mode )
{
  if ( is_open() )
  {
    //      std::cerr<<"Is already open!"<<std::endl;
    return NULL;
  }
  //  bool success=true;

  int oflag;
  std::ios_base::openmode open_mode = ( mode & ~std::ios_base::ate & ~std::ios_base::binary );

  // the corresponding O_*-flags are as described in Josuttis Chap. 13 (p.632)
  if ( open_mode == std::ios_base::out ) // corresponds to "w"
  {
    oflag = ( O_WRONLY | O_TRUNC | O_CREAT );
  }
  else if ( open_mode == ( std::ios_base::out | std::ios_base::app ) ) // corresponds to "a"
  {
    oflag = ( O_WRONLY | O_APPEND | O_CREAT );
  }
  else if ( open_mode == ( std::ios_base::out | std::ios_base::trunc ) ) // corresponds to "w"
  {
    oflag = ( O_WRONLY | O_TRUNC | O_CREAT );
  }
  else if ( open_mode == std::ios_base::in ) // corresponds to "r"
  {
    oflag = O_RDONLY;
  }
  else if ( open_mode == ( std::ios_base::in | std::ios_base::out ) ) // corresponds to "r+"
  {
    oflag = O_RDWR;
  }
  else if ( open_mode == ( std::ios_base::in | std::ios_base::out | std::ios_base::trunc ) ) // corresponds to "w+"
  {
    oflag = ( O_RDWR | O_TRUNC | O_CREAT );
  }
  else
  {
    // std::cerr<<"bad flags!"<<std::endl;
    return NULL;
  }

  // these file permissions are required by POSIX.1 (see Stevens 5.5)
  m_fd = ::open( s, oflag, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH );

  if ( m_fd == -1 )
  {
    // std::cerr<<"::open failed!"<<std::endl;
    // perror(NULL);
    return NULL;
  }

  // beware of operator precedence --- HEP
  if ( ( mode & std::ios_base::ate ) != 0 )
  {
    if ( lseek( m_fd, 0, SEEK_END ) == -1 )
    {
      close();
      // std::cerr<<"seek failed!"<<std::endl;
      // perror(NULL);
      return NULL;
    }
  };

  m_isopen = true;
  return this;
}

fdbuf*
fdbuf::close()
{
  if ( not is_open() )
  {
    // std::cerr<<"File was not open."<<std::endl;
    return NULL;
  }

  bool success = true;

  if ( overflow( traits_type::eof() ) == traits_type::eof() )
  {
    // std::cerr<<"overflow failed!"<<std::endl;
    success = false;
  }
  if (::close( m_fd ) == -1 )
  {
    // std::cerr<<"::close failed: "<<std::endl;perror(NULL);
    success = false;
  }

  m_isopen = false;

  return ( success ? this : NULL );
}

void
ofdstream::close()
{
  if ( rdbuf()->close() == NULL )
  {
    setstate( failbit );
  }
}


void
ifdstream::close()
{
  if ( rdbuf()->close() == NULL )
  {
    setstate( failbit );
  }
}

void
fdstream::close()
{
  if ( rdbuf()->close() == NULL )
  {
    setstate( failbit );
  }
}


#endif
