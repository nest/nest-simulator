Rules for C++  code comments
============================

* C++ comments single line (max 2 lines if sentence is long): //
* C++ comments multile line: ``/* * * * */``

* avoid doxygen comments in .cpp files, move to header file
* avoid duplicating code in comments; only include code(parameters, functions etc) that also have additional context, remove redundancies
* include the variable name in functions in header file to match cpp file.


Functions and classes
---------------------

doxygen style: ``/** * */``


Variables
---------


doxygen comments single line (max 2 lines if sentence is long): //!
multiline doxygen style: `/** * */`
doxygen comments side: //!<

Use //! for long one-liners above variables and //!< for short ones behind them

Examples::

 /**
  * Set the minimal and maximal delay and override automatically determined values.
  */
 void set_delay_extrema(long min_delay, long max_delay);

 //! The variable min_delay holds the value for the smallest transmission delay in the network in steps
 long min_delay_;

 long max_delay_; //!< The largest transmission delay in the network (steps)


