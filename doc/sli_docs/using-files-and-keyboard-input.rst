Using files and keyboard input
==============================

Overview
--------

SLI’s input/output fascilities differ from those of PostScript and are
close to the stream concept of C++. However, for compatibility some
PostScript output commands are implemented.

Like in C++, files are represented as *streams*, which can be put on the
stack. All i/o commands leave their stream argument on the stack, so
that operations can be chained.

Example
~~~~~~~

Print *Hello World* to the standard output.

::

   SLI ] cout (Hello World) <- endl
   Hello World
   SLI [1] ;
   SLI ]

``cout`` is the standard output of SLI. The command ``<-`` takes the
role of C++’s ``<<`` output operator and prints the ASCII representation
of the object at stack level 0 to the stream at level 1. After this, the
object is removed and the stream remains at level 0.

The command ``endl`` corresponds to the C++ manipulator of the same
name. It prints an end of line character to the stream at level 0.
Again, it leaves the stream argument on the stack.

Now, the abbreviated form of ``pop``, i.e. the command ``;``, is used to
remove the stream object from the stack.

Standard streams
----------------

The standard streams of a UNIX program are mapped to the following
names. Note that these streams should not be closed by a SLI program,
since the result is undefined.

Name Description ``cin`` Standart input stream. ``cout`` Standart output
stream. ``cerr`` Standart error output stream.

Opening and closing a stream
----------------------------

Streams are objects which handle the input and output of data to or from
some external target. The target of a stream can be a file, a string, a
devide, or another process.

Command Description ``(name) (r) file`` Open file for reading.
``(name) ifstream`` Open file for reading. ``(name) (w) file`` Open file
for writing. ``(name) ofstream`` Open file for writing.
``(string) istrstream`` Open string-stream for reading.
``(string) ostrstream`` Open string-stream for writing. ``strstream``
Extract a string from a string-stream. ``stream close`` Close the
stream.

Writing to streams
------------------

Command Description ``stream obj <-`` Print ASCII representation of
``obj`` to ``stream``. ``stream obj <--`` Print detailed ASCII
representation of ``obj`` to ``stream``. ``stream obj =`` Print ASCII
representation of ``obj`` to ``cout``. ``stream obj ==`` Print detailed
ASCII representation of ``obj`` to ``cout``.

.. _example-1:

Example
~~~~~~~

Print *Hello World* to a text file.

::

   SLI ] (test.txt) (w) file
   SLI [1] (Hello World!) <-
   SLI [1] endl
   SLI [1] ;

Manipulators
------------

Manipulators are used to manipulate the state of a stream object. Such
changes can, for instance, affect the precision with which numbers are
printed.

Manipulators take one or more arguments. In general, the manipulator
leaves the stream object at the top of the stack and removes all other
arguments.

Manipulator Description ``ofstream flush`` Write contents of buffer to
file. ``ofstream endl`` Line terminator. ``osstream ends`` char[] string
terminator. ``ifstream ws`` Eat white-spaces. ``ofstream boolalpha``
Prints bool as true/false. ``ofstream noboolalpha`` Opposite.
``fstream n setw`` Set width of input/output fields to ``n``.
``stream (c) setfill`` Defines a fill symbol ``c`` for the field.
``ostream left`` Allign to left of the field. ``ostream right`` Allign
to right of the field. ``ostream internal`` Sign left and number right.
``ostream showpos`` Print positive sign. ``ostream noshowpos`` Opposite.
``stream uppercase`` ``ostream nouppercase`` ``ostream oct`` Switch to
octal notation. ``ostream dec`` Switch to decimal notation.
``ostream hex`` Switch to hexadecimal notation. ``ostream showbase``
Show base according to use of oct/dec/hex. ``ostream noshowbase`` Don’t
show base according to use of oct/dec/hex. ``ostream showpoint`` Decimal
point is always printed. ``ostream noshowpoint`` Decimal point is never
printed. ``ostream n setprecision`` Set number of decimal places to
``n``. ``ostream fixed`` Use fixed point notation.
``ostream scientific`` Use scientific notation.
