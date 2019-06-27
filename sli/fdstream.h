/*
 *  fdstream.h
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

#ifndef FDSTREAM_H
#define FDSTREAM_H

/** @file fdstream.h
 *  An implementation of C++ filestreams that supports an interface to
 *  the file descriptor.
 */

// Generated includes:
#include "config.h"

#ifndef HAVE_ISTREAM
// C++ includes:
#include <iostream>
#include <fstream>

typedef std::ifstream ifdstream;
typedef std::ofstream ofdstream;
typedef std::iostream fdstream;
#else

// C includes:
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// C++ includes:
#include <cassert>
#include <cstdio> // for the parallel FILE* workaround.
#include <fstream>
#include <iostream>
#include <istream>
#include <ostream>
#include <streambuf>
#include <string>

// The Compaq C++ compiler V6.5-014 surpresses all non standard
// names if compilation is performed with -std strict_ansi.
// However, fdopen() is POSIX and guaranteed to be available in
// <stdio.h>.
// The flag __PURE_CNAME is specific for the Compaq compiler.
// We need to add a workaround flag HAVE_FDOPEN_IGNORED here.
// 24.4.03, Diesmann
//
// Compaq C++ V6.5-040 required fdopen without :: here.  In fdstream.cc,
// ::fdopen is fine then.
// 28.6.04, Plesser
#ifdef __PURE_CNAME
extern "C" std::FILE* fdopen( int, const char* );
#endif


class fdbuf : public std::streambuf
{
  static std::streamsize const s_bufsiz = 1024;

public:
  fdbuf()
    : m_fd( -1 )
    , m_isopen( false )
  {
    setp( m_outbuf, m_outbuf + s_bufsiz );
  }

  fdbuf( int fd )
    : m_fd( fd )
    , m_isopen( true )
  {
    setp( m_outbuf, m_outbuf + s_bufsiz );
  }

  ~fdbuf()
  {
    // sync();
    close();
  }


  bool
  is_open() const
  {
    return m_isopen;
  }

  fdbuf* open( const char*, std::ios_base::openmode );

  fdbuf* close();

  /** Return the underlying file descriptor.
   *  Now this is why we are doing all this!!
   */
  int
  fd()
  {
    return m_fd;
  }

protected:
  int_type
  underflow()
  {
    if ( gptr() == egptr() )
    {
      int size = ::read( m_fd, m_inbuf, s_bufsiz );
      if ( size < 1 )
      {
        return traits_type::eof();
      }
      setg( m_inbuf, m_inbuf, m_inbuf + size );
    }
    return traits_type::to_int_type( *gptr() );
  }

  int_type
  overflow( int_type c )
  {
    if ( sync() == -1 )
    {
      // std::cerr<<"sync failed!"<<std::endl;
      return traits_type::eof();
    }

    if ( not traits_type::eq_int_type( c, traits_type::eof() ) )
    {
      *pptr() = traits_type::to_char_type( c );
      pbump( 1 );
      return c;
    }
    return traits_type::not_eof( c );
  }

  int
  sync()
  {
    std::streamsize size = pptr() - pbase();
    if ( size > 0 && ::write( m_fd, m_outbuf, size ) != size )
    {
      return -1;
    }
    setp( m_outbuf, m_outbuf + s_bufsiz );
    return 0;
  }

private:
  int m_fd;
  bool m_isopen;
  char m_inbuf[ s_bufsiz ];
  char m_outbuf[ s_bufsiz ];
};


class ofdstream : public std::ostream
{
public:
  /**
   * @note In this an all other constructors, we initialize the stream with
   *       a 0-pointer to a stream buffer first and then set the pointer through
   *       a call to rdbuf() in the constructor body. This is necessary because
   *       the stream buffer sb is constructed after the stream object, which
   *       is inherited from the base class. This causes problems at least when
   *       using the PGI compiler.
   */

  ofdstream()
    : std::ostream( 0 )
    , sb()
  {
    std::ostream::rdbuf( &sb );
    init( &sb );
  }

  explicit ofdstream( const char* s, std::ios_base::openmode mode = std::ios_base::out )
    : std::ostream( 0 )
    , sb()
  {
    std::ostream::rdbuf( &sb );
    init( &sb );
    assert( good() );
    open( s, mode );
  }

  explicit ofdstream( int fd )
    : std::ostream( 0 )
    , sb( fd )
  {
    std::ostream::rdbuf( &sb );
    init( &sb );
  }

  fdbuf*
  rdbuf() const
  {
    // return type is non-const, member is const, by C++ specs!
    return const_cast< fdbuf* >( &sb );
  }

  bool
  is_open()
  {
    return rdbuf()->is_open();
  }

  void
  open( const char* s, std::ios_base::openmode mode = std::ios_base::out )
  {
    if ( rdbuf()->open( s, mode | std::ios_base::out ) == NULL )
    {
      setstate( failbit );
    }
  }

  void close();

private:
  fdbuf sb;
};

class ifdstream : public std::istream
{
public:
  ifdstream()
    : std::istream( 0 )
    , sb()
  {
    std::istream::rdbuf( &sb );
    init( &sb );
  }

  explicit ifdstream( const char* s, std::ios_base::openmode mode = std::ios_base::in )
    : std::istream( 0 )
    , sb()
  {
    std::istream::rdbuf( &sb );
    init( &sb );
    open( s, mode );
  }

  explicit ifdstream( int fd )
    : std::istream( 0 )
    , sb( fd )
  {
    std::istream::rdbuf( &sb );
    init( &sb );
  }


  fdbuf*
  rdbuf() const
  {
    // return type is non-const, member is const, by C++ specs!
    return const_cast< fdbuf* >( &sb );
  }

  bool
  is_open()
  {
    return rdbuf()->is_open();
  }

  void
  open( const char* s, std::ios_base::openmode mode = std::ios_base::in )
  {
    if ( rdbuf()->open( s, mode | std::ios_base::in ) == NULL )
    {
      setstate( failbit );
    }
  }

  void close();

private:
  fdbuf sb;
};

class fdstream : public std::iostream
{
public:
  fdstream()
    : std::iostream( 0 )
    , sb() // See Constructor comment above.
  {
    std::iostream::rdbuf( &sb );
    init( &sb ); // See Constructor comment above.
  }

  explicit fdstream( const char* s, std::ios_base::openmode mode )
    : std::iostream( 0 )
    , sb() // See Constructor comment above.
  {
    std::iostream::rdbuf( &sb );
    init( &sb ); // See Constructor comment above.
    open( s, mode );
  }

  explicit fdstream( int fd )
    : // See Constructor comment above.
    std::iostream( 0 )
    , sb( fd )
  {
    std::iostream::rdbuf( &sb );
    init( &sb ); // See Constructor comment above.
  }


  fdbuf*
  rdbuf() const
  {
    // return type is non-const, member is const, by C++ specs!
    return const_cast< fdbuf* >( &sb );
  }

  bool
  is_open()
  {
    return rdbuf()->is_open();
  }

  void
  open( const char* s, std::ios_base::openmode mode )
  {
    if ( rdbuf()->open( s, mode ) == NULL )
    {
      setstate( failbit );
    }
  }

  void close();

private:
  fdbuf sb;
};
#endif

#endif
