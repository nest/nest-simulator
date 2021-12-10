Rules for C++  code comments
============================

* C++ comments single line (max 2 lines if sentence is long): //
* C++ comments multile line: `/* * * * */`

* avoid doxygen comments in .cpp files, move to header file
* avoid duplicating code in comments; only include code(parameters, functions etc) that also have additional context, remove redundancies
* include the variable name in functions in header file to match cpp file.


Functions and classes
---------------------

doxygen style: `/** * */`


Variables
---------


doxygen comments single line (max 2 lines if sentence is long): //!
multiline doxygen style: `/** * */`
doxygen comments side: //!<

(I would use //! for long one-liners above variables and //!< for short ones behind them)


Here are some examples that hopefully clarify my style preferences mentioned above::

 /**
  * Set the minimal and maximal delay and override automatically determined values.
  */
 void set_delay_extrema(long min_delay, long max_delay);

 //! The variable min_delay holds the value for the smallest transmission delay in the network in steps
 long min_delay_;

 long max_delay_; //!< The largest transmission delay in the network (steps)

.. note::

 In real code, I would then actually prefer if similar variables were documented consistently, i.e. both min_delay_ and max_delay_ with either the long version, or with the short one, depending on the length of the comment. Above, I would likely prefer the short one, as the long description is a bit redundant and the short one would allow to skim through the code quicker (due to less optical clutter), cf.

 //! The variable min_delay holds the value for the smallest transmission delay in the network in steps
 long min_delay_;
 //! The variable max_delay holds the value for the largest transmission delay in the network in steps
 long max_delay_;

 long min_delay_;   //!< The smallest transmission delay in the network (steps)
 long max_delay_;   //!< The largest transmission delay in the network (steps)
