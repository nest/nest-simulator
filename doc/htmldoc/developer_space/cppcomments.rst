Guidelines for C++ code comments
================================

There are two types of code comments for C++ files: doxygen style and C++ style comments.

* Doxygen styled comments are used for describing things like the purpose of the function, which parameters it accepts, and what output it generates.
* Use Doxygen style comments in the header (``.h``)  files. Avoid using them in ``.cpp`` files.
* Do not duplicate code in comments.

..  Include the variable name in functions in header file to match cpp file.


Generate HTML from doxygen comments
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To generate HTML output of the doxgyen comments,

in the build directory,

run ``cmake`` with developer documentation on::

  cmake -Dwith-devdoc=ON path/to/nest-simulator.

then

::

   make docs
   xdg-open doc/doxygen/html/index.html

.. seealso::

  See `the official doxygen documentation <https://www.doxygen.nl/>`_ for details.


Doxygen style
~~~~~~~~~~~~~

* Multi-line comments

.. code-block:: cpp

  /**
   * Short description
   *
   * Further details, if necesary
   */

.. note::

    Functions and classes should use the multi-line style even for single line comments.

* Single or two line comments to use with variables:

.. code-block:: cpp

   //! The maximum delay of the simulation
   long may_delay;


* If a short comment is needed for variables, you can add a comment to the right of the code:

.. code-block:: cpp

  long max_delay_; //!< The maximum delay of the simulation

C++ style
~~~~~~~~~

* Multi-line comments:

.. code-block:: cpp

   //
   //
   //
   //

  * Single or two line comments:

.. code-block:: cpp

 //
