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

    Before you make a pull request :ref:`see which developer tools are required <required_dev_tools>`  to ensure its compliant with our guidelines.

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
Guide <https://google.github.io/styleguide/cppguide.html>`_.

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

* Lines should not exceed 120 character
* Files should not be too long (max. 2000 lines)
* No trailing whitespace

Folders
*******

Use ``lower_case_under_lined`` notation for folder names.

Variables and class members
***************************

In general, use meaningful, non-abbreviated names or follow naming conventions
from the neuroscience field, for example, the membrane potential is :hxt_ref:`V_m`. Use the
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
only). Parameters of methods should either fit into one line or
each parameter is on a separate line.

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
statements in header files. The closing brace of a namespace should be
followed by a comment containing the namespace statement.
Do not indent the body of namespaces.

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
not indented.

Do not implement methods inside the class definition, but implement small
``inline`` methods after the class definition and other methods in the
corresponding implementation file.

Template class declarations follow the same style as normal class declarations.
This applies in particular to inline declarations. The keyword template
followed by the list of template parameters appear on a separate line. The <
and > in template expressions have one space after and before the sign,
respectively, e.g., ``std::vector< int >``.

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

As a general rule of thumb, always indent with two spaces. Do
not use TAB character in any source file. Always use braces
around blocks of code. The braces of code blocks have their own
line.

Control structures (``if``, ``while``, ``for``, ...) have a single space after the
keyword. The parenthesis around the tests
have a space after the opening and before the closing parenthesis.
The case labels in ``switch`` statements are not indented.

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
``a + b``.

Unary operators have no space between operator and operand, e.g. ``-a``.
Do not use the negation operator `!` since it can easily be
overseen. Instead use ``not``, e.g. ``not vec.empty()``.

There is no space between a statement and its corresponding semicolon:

.. code::

   return a + 3 ; // bad
   return a + 3;  // good

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

    // C++ includes:
    #include <algorithm>
    #include <iostream>
    #include <vector>

    // Includes from nestkernel:
    #include "arraydatum.h"
    #include "dictdatum.h"
    #include "dictutils.h"
    #include "exceptions.h"
    #include "kernel_manager.h"
    #include "vp_manager.h"

    namespace nest
    {

    #ifdef TIMER_DETAILED
    constexpr bool use_detailed_timers = true;
    #else
    constexpr bool use_detailed_timers = false;
    #endif
    #ifdef THREADED_TIMERS
    constexpr bool use_threaded_timers = true;
    #else
    constexpr bool use_threaded_timers = false;
    #endif

    enum class StopwatchGranularity
    {
      Normal,  //<! Always measure stopwatch
      Detailed //<! Only measure if detailed stopwatches are activated
    };

    enum class StopwatchParallelism
    {
      MasterOnly, //<! Only the master thread owns a stopwatch
      Threaded    //<! Every thread measures an individual stopwatch
    };

    // Forward class declaration required here because friend declaration
    // in timers::StopwatchTimer must refer to nest::Stopwatch to be correct,
    // and that requires the name to be known from before. See
    // https://stackoverflow.com/questions/30418270/clang-bug-namespaced-template-class-friend
    // for details.
    template < StopwatchGranularity, StopwatchParallelism, typename >
    class Stopwatch;

    /********************************************************************************
     * Stopwatch                                                                    *
     *   Accumulates time between start and stop, and provides the elapsed time     *
     *   with different time units. Either runs multi-threaded or only on master.   *
     *                                                                              *
     *   Usage example:                                                             *
     *     Stopwatch< StopwatchGranularity::Normal, StopwatchParallelism::MasterOnly > x;    *
     *     x.start();                                                               *
     *     // ... do computations for 15.34 sec                                     *
     *     x.stop(); // only pauses stopwatch                                       *
     *     x.print("Time needed "); // > Time needed 15.34 sec.                     *
     *     x.start(); // resumes stopwatch                                          *
     *     // ... next computations for 11.22 sec                                   *
     *     x.stop();                                                                *
     *     x.print("Time needed "); // > Time needed 26,56 sec.                     *
     *     x.reset(); // reset to default values                                    *
     *     x.start(); // starts the stopwatch from 0                                *
     *     // ... computation 5.7 sec                                               *
     *     x.print("Time "); // > Time 5.7 sec.                                     *
     *     // ^ intermediate timing without stopping the stopwatch                  *
     *     // ... more computations 1.7643 min                                      *
     *     x.stop();                                                                *
     *     x.print("Time needed ", timeunit_t::MINUTES, std::cerr);              *
     *     // > Time needed 1,8593 min. (on cerr)                                   *
     *     // other units and output streams possible                               *
     ********************************************************************************/
    namespace timers
    {
    enum class timeunit_t : size_t
    {
      NANOSEC = 1,
      MICROSEC = NANOSEC * 1000,
      MILLISEC = MICROSEC * 1000,
      SECONDS = MILLISEC * 1000,
      MINUTES = SECONDS * 60,
      HOURS = MINUTES * 60,
      DAYS = HOURS * 24
    };

    /** This class represents a single timer which measures the execution time of a single thread for a given clock type.
     * Typical clocks are monotonic wall-time clocks or clocks just measuring cpu time.
     */
    template < clockid_t clock_type >
    class StopwatchTimer
    {
      template < StopwatchGranularity, StopwatchParallelism, typename >
      friend class nest::Stopwatch;

    public:
      typedef size_t timestamp_t;

      //! Creates a stopwatch that is not running.
      StopwatchTimer()
      {
        reset();
      }

      //! Starts or resumes the stopwatch, if it is not running already.
      void start();

      //! Stops the stopwatch, if it is not stopped already.
      void stop();

      /**
       * Returns the time elapsed between the start and stop of the stopwatch in the given unit. If it is running, it
       * returns the time from start until now. If the stopwatch is run previously, the previous runtime is added. If you
       * want only the last measurement, you have to reset the timer, before stating the measurement.
       * Does not change the running state.
       */
      double elapsed( timeunit_t timeunit = timeunit_t::SECONDS ) const;

      //! Resets the stopwatch.
      void reset();

      //! This method prints out the currently elapsed time.
      void
      print( const std::string& msg = "", timeunit_t timeunit = timeunit_t::SECONDS, std::ostream& os = std::cout ) const;

    private:
      //! Returns, whether the stopwatch is running.
      bool is_running_() const;

    #ifndef DISABLE_TIMING
      timestamp_t _beg, _end;
      size_t _prev_elapsed;
      bool _running;
    #endif

      //! Returns current time in microseconds since EPOCH.
      static size_t get_current_time();
    };

    template < clockid_t clock_type >
    inline void
    StopwatchTimer< clock_type >::start()
    {
    #ifndef DISABLE_TIMING
      if ( not is_running_() )
      {
        _prev_elapsed += _end - _beg;     // store prev. time, if we resume
        _end = _beg = get_current_time(); // invariant: _end >= _beg
        _running = true;                  // we start running
      }
    #endif
    }

    template < clockid_t clock_type >
    inline void
    StopwatchTimer< clock_type >::stop()
    {
    #ifndef DISABLE_TIMING
      if ( is_running_() )
      {
        _end = get_current_time(); // invariant: _end >= _beg
        _running = false;          // we stopped running
      }
    #endif
    }

    template < clockid_t clock_type >
    inline bool
    StopwatchTimer< clock_type >::is_running_() const
    {
    #ifndef DISABLE_TIMING
      return _running;
    #else
      return false;
    #endif
    }

    template < clockid_t clock_type >
    inline double
    StopwatchTimer< clock_type >::elapsed( timeunit_t timeunit ) const
    {
    #ifndef DISABLE_TIMING
      size_t time_elapsed;
      if ( is_running_() )
      {
        // get intermediate elapsed time; do not change _end, to be const
        time_elapsed = get_current_time() - _beg + _prev_elapsed;
      }
      else
      {
        // stopped before, get time of current measurement + last measurements
        time_elapsed = _end - _beg + _prev_elapsed;
      }
      return static_cast< double >( time_elapsed ) / static_cast< double >( timeunit );
    #else
      return 0.;
    #endif
    }

    template < clockid_t clock_type >
    inline void
    StopwatchTimer< clock_type >::reset()
    {
    #ifndef DISABLE_TIMING
      _beg = 0; // invariant: _end >= _beg
      _end = 0;
      _prev_elapsed = 0; // erase all prev. measurements
      _running = false;  // of course not running.
    #endif
    }

    template < clockid_t clock_type >
    inline void
    StopwatchTimer< clock_type >::print( const std::string& msg, timeunit_t timeunit, std::ostream& os ) const
    {
    #ifndef DISABLE_TIMING
      double e = elapsed( timeunit );
      os << msg << e;
      switch ( timeunit )
      {
      case timeunit_t::NANOSEC:
        os << " nanosec.";
      case timeunit_t::MICROSEC:
        os << " microsec.";
        break;
      case timeunit_t::MILLISEC:
        os << " millisec.";
        break;
      case timeunit_t::SECONDS:
        os << " sec.";
        break;
      case timeunit_t::MINUTES:
        os << " min.";
        break;
      case timeunit_t::HOURS:
        os << " h.";
        break;
      case timeunit_t::DAYS:
        os << " days.";
        break;
      default:
        throw BadParameter( "Invalid timeunit provided to stopwatch." );
      }
    #ifdef DEBUG
      os << " (running: " << ( _running ? "true" : "false" ) << ", begin: " << _beg << ", end: " << _end
         << ", diff: " << ( _end - _beg ) << ", prev: " << _prev_elapsed << ")";
    #endif
      os << std::endl;
    #endif
    }

    template < clockid_t clock_type >
    inline size_t
    StopwatchTimer< clock_type >::get_current_time()
    {
      timespec now;
      clock_gettime( clock_type, &now );
      return now.tv_nsec + now.tv_sec * static_cast< long >( timeunit_t::SECONDS );
    }

    template < clockid_t clock_type >
    inline std::ostream&
    operator<<( std::ostream& os, const StopwatchTimer< clock_type >& stopwatch )
    {
      stopwatch.print( "", timeunit_t::SECONDS, os );
      return os;
    }

    } // namespace timers


    /** This is the base template for all Stopwatch specializations.
     *
     * This template will be specialized in case detailed timers are deactivated or the timer is supposed to be run by
     * multiple threads. If the timer should be deactivated, because detailed timers are disabled, the template
     * specialization will be empty and optimized away by the compiler.
     * This base template only measures a single timer, owned by the master thread, which applies for both detailed and
     * regular timers. Detailed, master-only timers that are deactivated when detailed timers are turned off are handled
     * by one of the template specializations below.
     *
     * The template has three template arguments of which two act as actual parameters that need to be specified for each
     * stopwatch instance. The first one "detailed_timer" controls the granularity of the stopwatch, i.e., if the timer is
     * considered a normal or detailed timer. The second one "threaded_timer" defines if the timer is supposed to be
     * measured by each thread individually. In case a timer is specified as threaded, but threaded timers are turned off
     * globally, the stopwatch will run in master-only mode instead. The third template argument is used to enable or
     * disable certain template specializations based on compiler flags (i.e., detailed timers and threaded timers).
     *
     * In all cases, both the (monotonic) wall-time and cpu time are measured.
     */
    template < StopwatchGranularity detailed_timer, StopwatchParallelism threaded_timer, typename = void >
    class Stopwatch
    {
    public:
      void
      start()
      {
    #pragma omp master
        {
          walltime_timer_.start();
          cputime_timer_.start();
        }
      }

      void
      stop()
      {
    #pragma omp master
        {
          walltime_timer_.stop();
          cputime_timer_.stop();
        }
      }

      double
      elapsed( timers::timeunit_t timeunit = timers::timeunit_t::SECONDS ) const
      {
        double elapsed = 0.;
    #pragma omp master
        {
          elapsed = walltime_timer_.elapsed( timeunit );
        };
        return elapsed;
      }

      void
      reset()
      {
    #pragma omp master
        {
          walltime_timer_.reset();
          cputime_timer_.reset();
        }
      }

      void
      print( const std::string& msg = "",
        timers::timeunit_t timeunit = timers::timeunit_t::SECONDS,
        std::ostream& os = std::cout ) const
      {
    #pragma omp master
        walltime_timer_.print( msg, timeunit, os );
      }

      void
      get_status( DictionaryDatum& d, const Name& walltime_name, const Name& cputime_name ) const
      {
        def< double >( d, walltime_name, walltime_timer_.elapsed() );
        def< double >( d, cputime_name, cputime_timer_.elapsed() );
      }

    private:
      bool
      is_running_() const
      {
        bool is_running_ = false;
    #pragma omp master
        {
          is_running_ = walltime_timer_.is_running_();
        };
        return is_running_;
      }

      // We use a monotonic timer to make sure the stopwatch is not influenced by time jumps (e.g. summer/winter time).
      timers::StopwatchTimer< CLOCK_MONOTONIC > walltime_timer_;
      timers::StopwatchTimer< CLOCK_THREAD_CPUTIME_ID > cputime_timer_;
    };

    //! Stopwatch template specialization for detailed, master-only timer instances if detailed timers are deactivated.
    template <>
    class Stopwatch< StopwatchGranularity::Detailed,
      StopwatchParallelism::MasterOnly,
      std::enable_if< not use_detailed_timers > >
    {
    public:
      void
      start()
      {
      }
      void
      stop()
      {
      }
      double
      elapsed( timers::timeunit_t = timers::timeunit_t::SECONDS ) const
      {
        return 0;
      }
      void
      reset()
      {
      }
      void
      print( const std::string& = "", timers::timeunit_t = timers::timeunit_t::SECONDS, std::ostream& = std::cout ) const
      {
      }
      void
      get_status( DictionaryDatum&, const Name&, const Name& ) const
      {
      }

    private:
      bool
      is_running_() const
      {
        return false;
      }
    };

    //! Stopwatch template specialization for detailed, threaded timer instances if detailed timers are deactivated.
    template < StopwatchGranularity detailed_timer >
    class Stopwatch< detailed_timer,
      StopwatchParallelism::Threaded,
      std::enable_if_t< use_threaded_timers
        and ( detailed_timer == StopwatchGranularity::Detailed and not use_detailed_timers ) > >
    {
    public:
      void
      start()
      {
      }
      void
      stop()
      {
      }
      double
      elapsed( timers::timeunit_t = timers::timeunit_t::SECONDS ) const
      {
        return 0;
      }
      void
      reset()
      {
      }
      void
      print( const std::string& = "", timers::timeunit_t = timers::timeunit_t::SECONDS, std::ostream& = std::cout ) const
      {
      }
      void
      get_status( DictionaryDatum&, const Name&, const Name& ) const
      {
      }

    private:
      bool
      is_running_() const
      {
        return false;
      }
    };

    /** Stopwatch template specialization for threaded timer instances if the timer is a detailed one and detailed timers
     * are activated or the timer is not a detailed one in the first place.
     */
    template < StopwatchGranularity detailed_timer >
    class Stopwatch< detailed_timer,
      StopwatchParallelism::Threaded,
      std::enable_if_t< use_threaded_timers
        and ( detailed_timer == StopwatchGranularity::Normal or use_detailed_timers ) > >
    {
    public:
      void start();

      void stop();

      double elapsed( timers::timeunit_t timeunit = timers::timeunit_t::SECONDS ) const;

      void reset();

      void print( const std::string& msg = "",
        timers::timeunit_t timeunit = timers::timeunit_t::SECONDS,
        std::ostream& os = std::cout ) const;

      void
      get_status( DictionaryDatum& d, const Name& walltime_name, const Name& cputime_name ) const
      {
        std::vector< double > wall_times( walltime_timers_.size() );
        std::transform( walltime_timers_.begin(),
          walltime_timers_.end(),
          wall_times.begin(),
          []( const timers::StopwatchTimer< CLOCK_MONOTONIC >& timer ) { return timer.elapsed(); } );
        def< ArrayDatum >( d, walltime_name, ArrayDatum( wall_times ) );

        std::vector< double > cpu_times( cputime_timers_.size() );
        std::transform( cputime_timers_.begin(),
          cputime_timers_.end(),
          cpu_times.begin(),
          []( const timers::StopwatchTimer< CLOCK_THREAD_CPUTIME_ID >& timer ) { return timer.elapsed(); } );
        def< ArrayDatum >( d, cputime_name, ArrayDatum( cpu_times ) );
      }

    private:
      bool is_running_() const;

      // We use a monotonic timer to make sure the stopwatch is not influenced by time jumps (e.g. summer/winter time).
      std::vector< timers::StopwatchTimer< CLOCK_MONOTONIC > > walltime_timers_;
      std::vector< timers::StopwatchTimer< CLOCK_THREAD_CPUTIME_ID > > cputime_timers_;
    };

    template < StopwatchGranularity detailed_timer >
    void
    Stopwatch< detailed_timer,
      StopwatchParallelism::Threaded,
      std::enable_if_t< use_threaded_timers
        and ( detailed_timer == StopwatchGranularity::Normal or use_detailed_timers ) > >::start()
    {
      kernel::manager<VPManager>().assert_thread_parallel();

      walltime_timers_[ kernel::manager<VPManager>().get_thread_id() ].start();
      cputime_timers_[ kernel::manager<VPManager>().get_thread_id() ].start();
    }

    template < StopwatchGranularity detailed_timer >
    void
    Stopwatch< detailed_timer,
      StopwatchParallelism::Threaded,
      std::enable_if_t< use_threaded_timers
        and ( detailed_timer == StopwatchGranularity::Normal or use_detailed_timers ) > >::stop()
    {
      kernel::manager<VPManager>().assert_thread_parallel();

      walltime_timers_[ kernel::manager<VPManager>().get_thread_id() ].stop();
      cputime_timers_[ kernel::manager<VPManager>().get_thread_id() ].stop();
    }

    template < StopwatchGranularity detailed_timer >
    bool
    Stopwatch< detailed_timer,
      StopwatchParallelism::Threaded,
      std::enable_if_t< use_threaded_timers
        and ( detailed_timer == StopwatchGranularity::Normal or use_detailed_timers ) > >::is_running_() const
    {
      kernel::manager<VPManager>().assert_thread_parallel();

      return walltime_timers_[ kernel::manager<VPManager>().get_thread_id() ].is_running_();
    }

    template < StopwatchGranularity detailed_timer >
    double
    Stopwatch< detailed_timer,
      StopwatchParallelism::Threaded,
      std::enable_if_t< use_threaded_timers
        and ( detailed_timer == StopwatchGranularity::Normal or use_detailed_timers ) > >::elapsed( timers::timeunit_t
        timeunit ) const
    {
      kernel::manager<VPManager>().assert_thread_parallel();

      return walltime_timers_[ kernel::manager<VPManager>().get_thread_id() ].elapsed( timeunit );
    }

    template < StopwatchGranularity detailed_timer >
    void
    Stopwatch< detailed_timer,
      StopwatchParallelism::Threaded,
      std::enable_if_t< use_threaded_timers
        and ( detailed_timer == StopwatchGranularity::Normal or use_detailed_timers ) > >::print( const std::string& msg,
      timers::timeunit_t timeunit,
      std::ostream& os ) const
    {
      kernel::manager<VPManager>().assert_thread_parallel();

      walltime_timers_[ kernel::manager<VPManager>().get_thread_id() ].print( msg, timeunit, os );
    }

    template < StopwatchGranularity detailed_timer >
    void
    Stopwatch< detailed_timer,
      StopwatchParallelism::Threaded,
      std::enable_if_t< use_threaded_timers
        and ( detailed_timer == StopwatchGranularity::Normal or use_detailed_timers ) > >::reset()
    {
      kernel::manager<VPManager>().assert_single_threaded();

      const size_t num_threads = kernel::manager<VPManager>().get_num_threads();
      walltime_timers_.resize( num_threads );
      cputime_timers_.resize( num_threads );
      for ( size_t i = 0; i < num_threads; ++i )
      {
        walltime_timers_[ i ].reset();
        cputime_timers_[ i ].reset();
      }
    }

    } /* namespace nest */
    #endif /* STOPWATCH_H */
