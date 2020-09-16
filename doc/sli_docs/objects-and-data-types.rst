Objects and data types
======================

Overview
--------

Anything that can be put on the stack is called object. There are
several different types of objects, which can store different types of
data. For a number of object types there exist two different states:
*executable* and *literal*. Some object types can change between these
two states. Literal objects are the majority and are simply pushed on
the stack when they are entered. By contrast, executable objects perform
some operation when they are entered and cannot easily be pushed on the
stack.

Numbers
-------

SLI distinguishes between real and integer numbers. Real numbers
correspond to the type ``double`` in C/C++. Integer numbers correspond
to the type ``long int`` of C/C++.

Examples 1
~~~~~~~~~~

Real numers are: ``1.4``, ``.5``, ``1.``, ``1e-2``

Integer numers are ``1``, ``2``, ``3000``

Arrays
------

Arrays are sequential containers which can hold any SLI object. The
elements of an array are indexed, starting with zero (0) as the first
index.

SLI arrays are heterogeneous. Objects of different type may be mixed
within one array. Arrays may also be nested to define matrices or
tensors of arbitrary rank.

Arrays are delimited by square brackets. The different elements of an
array are separated by white-spaces.

Examples 2
~~~~~~~~~~

::

   [1 2 3]   % a simple array with integers
   [1 a b]   % a mixed array
   [[1 2 3] [4 5 6]] % a 2x3 Matrix defined by nested array

Calculate the dot product of the vectors \\([1 2 3]cdot[4 5 6]\)

::

   SLI ] [1 2 3]
   SLI [1] [4 5 6]
   SLI [2] Dot =
   32                                                                              

Names
-----

Names are used to create variables and to identify specific SLI objects.
If a name is entered, it is immediately executed.

If you want to put a name on the stack without evaluating it, you need
to protect it by prepending a shash character (e.g. ``/a``). In this
case, the name is called a *literal name*.

The command ``def`` is used to create an association between a name an
an object.

Example 3
~~~~~~~~~

::

   SLI ] /pi 3.1415 def
   SLI ] pi
   SLI [1] =
   3.1415

The command ``who`` gives the list of names which have been defined
during the SLI session.

Example 4
~~~~~~~~~

Define the variables *a* and *b* with values 1 and 2 and calculate *a+b*

::

   SLI ] /a 1 def
   SLI ] /b 2 def
   SLI ] a b add
   SLI ] =
   3
   SLI ] who
   --------------------------------------------------
   Name                     Type                Value
   --------------------------------------------------
   pi                       doubletype          3.1415
   a                        integertype         1
   b                        integertype         2
   --------------------------------------------------
   Total number of dictionary entries: 3

Strings
-------

Strings are sequences of characters, delimited by parenthesis. In SLI,
characters are represented by interger numbers, e.g. 97 represents the
letter ‘a’, while 32 represents the *space* character.

The elements of a string are indexed, starting with zero (0) as the
first index.

Matched pairs of parentheses may be used inside strings.

Example 5
~~~~~~~~~

::

   SLI ] (Hello World!) =
   Hello World!                                                                    

Procedures
----------

Procedures are a sequence of SLI commands, enclosed by the delimiters
``{`` and ``}``. The delimiters prevent the objects from being executed
as you enter them. Instead, they will be executed when you evaluate the
procedure.

Bound to a name, procedures can be used like any builtin SLI command.

Example: Hello World
~~~~~~~~~~~~~~~~~~~~

Print the string *Hello World!*.

::

   /HelloWorld
   {
     (Hello World !) =
   } def

Dictionaries
------------

A dictionary is an associative array. It stores pairs of names and
objects, where the name acts as a key to access the object inside the
dictionary.

The pairs are delimited by the characters ``<<`` and ``>>``. Note that
the pairs are evaluated during the construction of the dictionary. Thus,
literal names have to be used here. The individual name/value pairs have
no defined sequential order (unlike a real dictionary where the keys are
ordered alphabetically).

Example 6
~~~~~~~~~

Create a dictionary which stores named parameters.

::

   SLI ] /parameters << /alpha 1.0 /beta 3.5 /tau 10.0 >> def
   SLI ] parameters /tau get =
   10

Example: Showing dictionaries
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The command ``info`` can be used to examine the contents of a
dictionary.

::

   SLI ] parameters info
   --------------------------------------------------
   Name                     Type                Value
   --------------------------------------------------
   alpha                    doubletype          1
   beta                     doubletype          3.5
   tau                      doubletype          10
   --------------------------------------------------
   Total number of dictionary entries: 3

Object types
------------

There are a number of different object types in SLI. Each type is
represented by a literal name (i.e. a name with a prepended slash). Here
is a list of the most important types:

\|—————–|——————-\| \| ``/integertype`` \| ``/doubletype`` \| \|
``/booltype`` \| ``/stringtype`` \| \| ``/nametype`` \| ``/literaltype``
\| \| ``/arraytype`` \| ``/proceduretype`` \| \| ``/modeltype`` \|
``/dictionarytype`` \| \| ``/ostreamtype`` \| ``/istreamtype`` \| \|
``/xistreamtype`` \| ``/trietype`` \|

Getting type information
~~~~~~~~~~~~~~~~~~~~~~~~

The command ``type`` returns the type-name of the top element of the
stack. Note that type removes the element.

The command ``typeinfo`` returns the type of an object without popping
it off the stack.

Conversion between types
~~~~~~~~~~~~~~~~~~~~~~~~

It is possible to perform conversions between types. However, unlike in
C or C++ this type conversion is never done implicitly. Some of the
conversion operators are inherited from PostScript. Others are special
to SLI.

Here, we list the most important conversion operators. Each conversion
operator gets one argument which can, in general, be of any type. If the
operator is meaningless for a given object, an argument type error is
raised.

Command Description ``cvd`` Convert a number to a double. ``cvi``
Convert a number to an integer. ``cvs`` Tries to convert the object to a
string. ``cst`` Convert a string to an array. ``cvx`` Convert an object
to an executable object, e.g. string to a procedure, or a literal name
to a name. ``cvlit`` Convert an object to a literal object, e.g. a
procedure to an array or a string to a literal name.
