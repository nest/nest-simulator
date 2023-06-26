.. _code_style_cpp:

NEST coding style guidelines for C++
====================================

In the code review process we want to enforce a consistent coding style to
improve readability and maintainability. The article on `why code readability
matters <http://blog.ashodnakashian.com/2011/03/code-readability/>`_ describes
the benefits of readable code. To simplify the process we use
different tools that check compliance with our coding style and developers can
reduce the workload of the review process by checking compliance of their code
on their own.


.. seealso::

    Before you make a pull request :ref:`see how to check your code <check_code>`  to ensure its compliant with our guidelines.

C++ language features
---------------------

1. Use only ISO C++ language features.
2. Prefer ISO C++ library functions over their ISO C library equivalents.
3. Prefer ISO C++ library containers (STL).
4. Prefer C++ headers over their C equivalents.
5. Don't use ``printf`` and related functions.
6. Use C++ style cast notation (see :ref:`books`).
7. Use the ``const`` qualifier where appropriate. Use it consistently (see :ref:`books`)!
8. Use namespaces and exceptions.
9. Try to avoid static class members which need a constructor (non POD).

Language of comments and identifiers
------------------------------------

1. All comments should be written in English.
2. All identifiers, class and function names should be in English.

Debugging and quality control
-----------------------------

Use the ``assert`` macro intensively to check program invariants.
Create expressive unit-tests using one of the supplied SLI and Python unit-testing
infrastructure or the C++ testing framework based on Boost.

Compiler
--------

NEST compiles with any recent version of the `GNU C/C++
Compiler <https://gcc.gnu.org/>`_ ``gcc``. Support for and limitations of other
compilers is described in the :ref:`Installation Instructions <install_nest>`

Resources
---------

Online reference documents
~~~~~~~~~~~~~~~~~~~~~~~~~~

1. `C++ Reference <http://www.cplusplus.com/reference/>`_
2. `C++ Wikibooks <https://en.wikibooks.org/wiki/C%2B%2B_Programming>`_

.. _books:

Books
~~~~~

We have found the following books to be useful.

1. Stroustrup B (1997) The C++ Programming Language, 3rd Edition, Addison-Wesley
2. Meyers S (1997) Effective C++, 2nd Edition, Addison Wesley
3. Meyers S (1996) More Effective C++, Addison Wesley
4. Coplien J O (1992) Advanced C++ programming styles and idioms, Addison-Wesley
5. Eckle B (1995) Thinking in C++, Prentice Hall
6. Plauger P J, Stepanov A, Lee M, and Musser D R (1998) The Standard Template Library,
   Comming June 1998, 1. Prentice Hall
7. Plauger P J (1995) The (draft) Standard C++ Library, Prentice Hall
8. Musser D R and Saini A (1996) STL Tutorial and Reference Guide, Addison-Wesley
9. Kernighan B and Ritchie D (1988) The C Programming Language, 2nd Edition, Prentice Hall

----

Coding style
------------

In the following the coding style guidelines are explained by example and some
parts are adopted from `Google C++ Style
Guide <https://google-styleguide.googlecode.com/svn/trunk/cppguide.html>`_.

The #define guard
~~~~~~~~~~~~~~~~~

All header files should have ``#define`` guards to prevent multiple inclusions.
The format of the symbol name should be ``<FILE>_H``. The file ``iaf_cond_alpha.h``
should have the following guard:

.. code::

   #ifndef IAF_COND_ALPHA_H
   #define IAF_COND_ALPHA_H
   ...
   #endif  // IAF_COND_ALPHA_H

Order of includes
~~~~~~~~~~~~~~~~~

Use standard order for readability and to avoid hidden dependencies: Related
header, C library, C++ library, other libraries' .h, your project's .h.

NEST's Makefiles add all project specific include paths to the compile
commands, thus the file ``iaf_cond_alpha.h`` should be included as:
``#include "iaf_cond_alpha.h"``

In ``iaf_cond_alpha.cpp``, whose main purpose is to implement ``iaf_cond_alpha.h``,
order your includes as follows:

1. ``iaf_cond_alpha.h``.
2. C system files.
3. C++ system files.
4. Other libraries' .h files.
5. Your project's .h files.

With the preferred ordering, if ``iaf_cond_alpha.h`` omits any necessary
includes, the build of ``iaf_cond_alpha.cpp`` will break. Thus, this rule ensures
that build breaks show up first for the people working on these files, not for
innocent people in other packages.

Within each section the includes should be ordered alphabetically.

You should include all the headers that define the symbols you rely upon
(except in cases of forward declaration). If you rely on symbols from ``bar.h``,
don't count on the fact that you included ``foo.h`` which (currently) includes
``bar.h``: include ``bar.h`` yourself, unless ``foo.h`` explicitly demonstrates its
intent to provide you the symbols of ``bar.h``. However, any includes present in
the related header do not need to be included again in the related cpp (i.e.,
``foo.cpp`` can rely on ``foo.h``'s includes).

For example, the includes in ``<nestdir>/models/iaf_cond_alpha.cpp`` might look
like this:

.. code::

   #include "iaf_cond_alpha.h"

   #include <sys/types.h>
   #include <unistd.h>
   #include <hash_map>
   #include <vector>

   #include "config.h"
   #include "foo.h"
   #include "node.h"

Exception
*********

Sometimes, system-specific code needs conditional includes. Such code can put
conditional includes after other includes. Of course, keep your system-specific
code small and localized. Example:

.. code::

   #include "iaf_cond_alpha.h"

   #include "port.h"  // For LANG_CXX11.

   #ifdef LANG_CXX11
   #include <initializer_list>
   #endif  // LANG_CXX11

Indentation, formatting and naming
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Files
*****

Files are named in ``lower_case_under_lined`` notation. C/C++ header files have
the extension ``.h``. C implementation files have the extension ``.c``. C++
implementation files have the extension ``.cpp``. The use of ``.cc`` is deprecated
and is only left for compatibility.

All files in NEST start with a preamble, which contains the filename and the
NEST copyright text (see example below).

Lines should not exceed 120 characters (clang-format). Files should not be too
long (max. 2000 lines) (vera++:L006). No trailing whitespace (clang-format).

Folders
*******

Use ``lower_case_under_lined`` notation for folder names.

Variables and class members
***************************

In general, use meaningful, non-abbreviated names or follow naming conventions
from the neuroscience field, e.g. the membrane potential is :hxt_ref:`V_m`. Use the
``lower_case_under_lined`` notation. Private member variables should end with an
underscore (``name_``). If applicable, the general rule is use is to use the
same notation for biophysical quantities as is used in `Dayan&Abbot, 2001
<https://www.gatsby.ucl.ac.uk/~lmate/biblio/dayanabbott.pdf>`_.

Constants should be defined with ``enums`` and not with ``#define``, and use the
``UPPER_CASE_UNDER_LINED`` notation:

.. code::

   enum StateVecElems
   {
     V_M = 0,
     DG_EXC,
     G_EXC,
     DG_INH,
     G_INH,
     STATE_VEC_SIZE
   };

Built-in types
**************

All code for the NEST kernel should use the type aliases, defined in ``nest.h``.
Thus, use ``nest::float_t`` instead of ``float``.

Functions and class methods
***************************

In general, use meaningful, non-abbreviated names or follow naming conventions
from the neuroscience field, e.g. the membrane potential is :hxt_ref:`V_m`. Use the
``lower_case_under_lined`` notation.

There should be a line-break after the method's return type (implementation
only) (clang-format). Parameters of methods should either fit into one line or
each parameter is on a separate line (clang-format).

.. code::

   inline void
   nest::Stopwatch::print( const char* msg,
                           timeunit_t timeunit,
                           std::ostream& os ) const
   {
     // code
   }

Namespaces
**********

Use ``lower_case_under_lined`` notation for namespaces. Do not use ``using namespace``
statements in header files (vera++:T018). The closing brace of a namespace should be
followed by a comment containing the namespace statement.
Do not indent the body of namespaces (clang-format).

.. code::

   namespace example
   {
   // code
   } // namespace example

All symbols for the NEST kernel are declared in the namespace ``nest``.

Structs and classes
*******************

Use a ``struct`` only for passive objects that carry data; everything else is a
``class``. Use ``CamelCase`` notation for naming classes, structs and enums, e.g.
``GenericConnBuilderFactory``. Private, nested classes and structs end with an
underscore (``State_``).

The access modifier (``public``, ``protected``, ``private``) in class definitions are
not indented (clang-format).

Do not implement methods inside the class definition, but implement small
``inline`` methods after the class definition and other methods in the
corresponding implementation file.

Template class declarations follow the same style as normal class declarations.
This applies in particular to inline declarations. The keyword template
followed by the list of template parameters appear on a separate line. The <
and > in template expressions have one space after and before the sign,
respectively, e.g. ``std::vector< int >`` (clang-format).

.. code::

   template< typename T >
   class MyClass: public T
   {
   public:
     // code
   private:
     // more code
   };

Further indentation and formatting
**********************************

Avoid committing indentation and formatting changes together with changes in
logic. Always commit these changes separately._

As a general rule of thumb, always indent with two spaces (clang-format). Do
not use TAB character in any source file (vera++:L002). Always use braces
around blocks of code (vera++:T019). The braces of code blocks have their own
line (clang-format).

Control structures (``if``, ``while``, ``for``, ...) have a single space after the
keyword (clang-format / vera++:T003, T008). The parenthesis around the tests
have a space after the opening and before the closing parenthesis
(clang-format). The case labels in ``switch`` statements are not indented
(clang-format).

.. code::

   if ( x > 0 )
   {
     // code
   }
   else
   {
     // code
   }

   switch ( i )
   {
   case 0:
     // code
   default:
     // code
   }

Binary operators (`+`, `-`, `*`, `||`, `&`, ...) are surrounded by one space, e.g.
``a + b`` (clang-format).

Unary operators have no space between operator and operand, e.g. ``-a``
(clang-format). Do not use the negation operator `!` since it can easily be
overseen. Instead use ``not``, e.g. ``not vec.empty()`` (vera++:T012).

There is no space between a statement and its corresponding semicolon
(clang-format):

.. code::

   return a + 3 ; // bad
   return a + 3;  // good

Further checks performed by vera++
**********************************

* **F001** Source files should not use the '\r' (CR) character
* **F002** File names should be well-formed
* **L001** No trailing whitespace (clang-format)
* **L003** no leading / ending empty lines
* **L005** not to many (> 2) consecutive empty lines
* **T001** One-line comments should not have forced continuation ( ``// ... \``)
* **T002** Reserved names should not be used for preprocessor macros
* **T004** Some keywords should be immediately followed by a colon (clang-format)
* **T005** Keywords break and continue should be immediately followed by a semicolon (clang-format)
* **T006** Keywords return and throw should be immediately followed by a semicolon or a single space (clang-format)
* **T007** Semicolons should not be isolated by spaces or comments from the rest of the code (~ clang-format)
* **T010** Identifiers should not be composed of 'l' and 'O' characters only
* **T017** Unnamed namespaces are not allowed in header files

Further transformations performed by clang-format
*************************************************

* Align trailing comments
* Always break before multi-line strings
* Always break template declarations
* Break constructor initializers before comma
* Pointer alignment: Left
* Space before assignment operators
* Spaces before trailing comments: 1
* Spaces in parentheses
* Spaces in square brackets

Stopwatch example
~~~~~~~~~~~~~~~~~

For example, the ``stopwatch.h`` file could look like:

.. code:: cpp

    /*
     *  stopwatch.h
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

    #ifndef STOPWATCH_H
    #define STOPWATCH_H

    // C includes:
    #include <sys/time.h>

    // C++ includes:
    #include <cassert>
    #include <iostream>

    namespace nest
    {

    /***********************************************************************
     * Stopwatch                                                           *
     *   Accumulates time between start and stop, and provides             *
     *   the elapsed time with different time units.                       *
     *                                                                     *
     *   Partly inspired by com.google.common.base.Stopwatch.java          *
     *   Not thread-safe: - Do not share stopwatches among threads.        *
     *                    - Let each thread have its own stopwatch.        *
     *                                                                     *
     *   Usage example:                                                    *
     *     Stopwatch x;                                                    *
     *     x.start();                                                      *
     *     // ... do computations for 15.34 sec                            *
     *     x.stop(); // only pauses stopwatch                              *
     *     x.print("Time needed "); // > Time needed 15.34 sec.            *
     *     x.start(); // resumes stopwatch                                 *
     *     // ... next computations for 11.22 sec                          *
     *     x.stop();                                                       *
     *     x.print("Time needed "); // > Time needed 26,56 sec.            *
     *     x.reset(); // reset to default values                           *
     *     x.start(); // starts the stopwatch from 0                       *
     *     // ... computation 5.7 sec                                      *
     *     x.print("Time "); // > Time 5.7 sec.                            *
     *     // ^ intermediate timing without stopping the stopwatch         *
     *     // ... more computations 1.7643 min                             *
     *     x.stop();                                                       *
     *     x.print("Time needed ", Stopwatch::MINUTES, std::cerr);         *
     *     // > Time needed 1,8593 min. (on cerr)                          *
     *     // other units and output streams possible                      *
     ***********************************************************************/
    class Stopwatch
    {
    public:
      typedef size_t timestamp_t;
      typedef size_t timeunit_t;

      enum
      {
        MICROSEC = ( timeunit_t ) 1,
        MILLISEC = MICROSEC * 1000,
        SECONDS = MILLISEC * 1000,
        MINUTES = SECONDS * 60,
        HOURS = MINUTES * 60,
        DAYS = HOURS * 24
      };

      static bool correct_timeunit( timeunit_t t );

      /**
       * Creates a stopwatch that is not running.
       */
      Stopwatch()
      {
        reset();
      }

      /**
       * Starts or resumes the stopwatch, if it is not running already.
       */
      void start();

      /**
       * Stops the stopwatch, if it is not stopped already.
       */
      void stop();

      /**
       * Returns, whether the stopwatch is running.
       */
      bool isRunning() const;

      /**
       * Returns the time elapsed between the start and stop of the
       * stopwatch. If it is running, it returns the time from start
       * until now. If the stopwatch is run previously, the previous
       * runtime is added. If you want only the last measurment, you
       * have to reset the timer, before stating the measurment.
       * Does not change the running state.
       */
      double elapsed( timeunit_t timeunit = SECONDS ) const;

      /**
       * Returns the time elapsed between the start and stop of the
       * stopwatch. If it is running, it returns the time from start
       * until now. If the stopwatch is run previously, the previous
       * runtime is added. If you want only the last measurment, you
       * have to reset the timer, before stating the measurment.
       * Does not change the running state.
       * In contrast to Stopwatch::elapsed(), only the timestamp is returned,
       * that is the number if microseconds as an integer.
       */
      timestamp_t elapsed_timestamp() const;

      /**
       * Resets the stopwatch.
       */
      void reset();

      /**
       * This method prints out the currently elapsed time.
       */
      void print( const char* msg = "", timeunit_t timeunit = SECONDS, std::ostream& os = std::cout ) const;

      /**
       * Convenient method for writing time in seconds
       * to some ostream.
       */
      friend std::ostream& operator<<( std::ostream& os, const Stopwatch& stopwatch );

    private:
    #ifndef DISABLE_TIMING
      timestamp_t _beg, _end;
      size_t _prev_elapsed;
      bool _running;
    #endif

      /**
       * Returns current time in microseconds since EPOCH.
       */
      static timestamp_t get_timestamp();
    };

    inline bool
    Stopwatch::correct_timeunit( timeunit_t t )
    {
      return t == MICROSEC || t == MILLISEC || t == SECONDS || t == MINUTES || t == HOURS || t == DAYS;
    }

    inline void
    nest::Stopwatch::start()
    {
    #ifndef DISABLE_TIMING
      if ( not isRunning() )
      {
        _prev_elapsed += _end - _beg;  // store prev. time, if we resume
        _end = _beg = get_timestamp(); // invariant: _end >= _beg
        _running = true;               // we start running
      }
    #endif
    }

    inline void
    nest::Stopwatch::stop()
    {
    #ifndef DISABLE_TIMING
      if ( isRunning() )
      {
        _end = get_timestamp(); // invariant: _end >= _beg
        _running = false;       // we stopped running
      }
    #endif
    }

    inline bool
    nest::Stopwatch::isRunning() const
    {
    #ifndef DISABLE_TIMING
      return _running;
    #else
      return false;
    #endif
    }

    inline double
    nest::Stopwatch::elapsed( timeunit_t timeunit ) const
    {
    #ifndef DISABLE_TIMING
      assert( correct_timeunit( timeunit ) );
      return 1.0 * elapsed_timestamp() / timeunit;
    #else
      return 0.0;
    #endif
    }

    inline nest::Stopwatch::timestamp_t
    nest::Stopwatch::elapsed_timestamp() const
    {
    #ifndef DISABLE_TIMING
      if ( isRunning() )
      {
        // get intermediate elapsed time; do not change _end, to be const
        return get_timestamp() - _beg + _prev_elapsed;
      }
      else
      {
        // stopped before, get time of current measurement + last measurements
        return _end - _beg + _prev_elapsed;
      }
    #else
      return ( timestamp_t ) 0;
    #endif
    }

    inline void
    nest::Stopwatch::reset()
    {
    #ifndef DISABLE_TIMING
      _beg = 0; // invariant: _end >= _beg
      _end = 0;
      _prev_elapsed = 0; // erase all prev. measurements
      _running = false;  // of course not running.
    #endif
    }

    inline void
    nest::Stopwatch::print( const char* msg, timeunit_t timeunit, std::ostream& os ) const
    {
    #ifndef DISABLE_TIMING
      assert( correct_timeunit( timeunit ) );
      double e = elapsed( timeunit );
      os << msg << e;
      switch ( timeunit )
      {
      case MICROSEC:
        os << " microsec.";
        break;
      case MILLISEC:
        os << " millisec.";
        break;
      case SECONDS:
        os << " sec.";
        break;
      case MINUTES:
        os << " min.";
        break;
      case HOURS:
        os << " h.";
        break;
      case DAYS:
        os << " days.";
        break;
      }
    #ifdef DEBUG
      os << " (running: " << ( _running ? "true" : "false" ) << ", begin: " << _beg << ", end: " << _end
         << ", diff: " << ( _end - _beg ) << ", prev: " << _prev_elapsed << ")";
    #endif
      os << std::endl;
    #endif
    }

    inline nest::Stopwatch::timestamp_t
    nest::Stopwatch::get_timestamp()
    {
      // works with:
      // * hambach (Linux 2.6.32 x86_64)
      // * JuQueen (BG/Q)
      // * MacOS 10.9
      struct timeval now;
      gettimeofday( &now, ( struct timezone* ) 0 );
      return ( nest::Stopwatch::timestamp_t ) now.tv_usec
        + ( nest::Stopwatch::timestamp_t ) now.tv_sec * nest::Stopwatch::SECONDS;
    }

    } /* namespace timer */
    #endif /* STOPWATCH_H */

And the corresponding ``stopwatch.cpp``:

.. code:: cpp

    /*
     *  stopwatch.cpp
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

    #include "stopwatch.h"

    namespace nest
    {
    std::ostream& operator<<( std::ostream& os, const Stopwatch& stopwatch )
    {
      stopwatch.print( "", Stopwatch::SECONDS, os );
      return os;
    }
    }


