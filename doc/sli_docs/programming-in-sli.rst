Programming in SLI
==================

Overview
--------

A procedure is a sequence of SLI objects whose execution is delayed
until the procedure is executed. Because procedures are objects, they
can be:

-  placed on the stack
-  bound to a name
-  executed repeatedly
-  executed by another procedure

A program is a sequence of SLI objects and procedures which are defined
in a file. This section introduces the fundamentals of SLI programming.

This chapter covers the basic programming concepts of the SLI language
features.

Entering and executing programs
-------------------------------

A program is a sequence of SLI objects and procedures which are defined
in a file. Program files are ordinary ASCII text files, which can be
created and modified with an editor of your choice (e.g. GNU Emacs).

SLI programs usually have the file ending “sli”, for example
``hello_world.sli``.

The ``run`` command is used to execute a program file.

Example: “Hello World!”
~~~~~~~~~~~~~~~~~~~~~~~

Write the program ``hello_world.sli`` according to the example, given
above.

1. Create an empty file ``hello_world.sli`` in the directory from which
   the interpreter was started.

2. Copy the *Hello World* example to the file and save it.

3. Enter ``(hello_world.sli) run`` at the command prompt.

   /HelloWorld { (Hello World !) = } def

   SLI ] (hello_world.sli) run SLI ] HelloWorld Hello World !

Note that the procedure is not immediately executed by ``run``. Rather,
all objects which are contained in the file are read and executed.

Using local variables
---------------------

Usually, all names you define are globally accessible. But, if you use a
lot of procedures that define their own variables, there is an
increasing danger that two procedures use the same name for different
purposes. This problem can be solved by keeping variable *local* to the
procedure that defines them.

SLI uses dictionaries to store and resolve variables.

Example 2
~~~~~~~~~

Compute the alpha-function of *t* according to *a(t)=t*Exp(-t/tau)*

::

   /alpha
   {
     << /tau -1.0 >> % create dictionary for local variables
     begin           % open local name space
       /t exch def     % store argument in local variable t
       t tau div exp   % compute formula
       t mul
     end             % close local name space
   } def

Conditionals
------------

Conditional expressions allow a program to ask questions and make
decisions:

-  Comparisons and logical functions

-  Conditional structures which test a certain condition and use the
   result to make a decision.

In general, conditional structures take a *boolean* object as well as
one or more procedure objects as argument and evaluate one of the
procedures, depending on the value of the boolean.

Example 3
~~~~~~~~~

The program in this example implements the faculty function according to
the definition:

::

   fac(1) := 1
   fac(n) := n*fac(n-1), for n>1

The program expects the argument on the stack and replaces it by the
result. Here, we use the ``if`` command to test whether the argument is
greater than 1. The ``if`` command takes two arguments, a boolean and a
procedure object. The boolean is supplied by the ``gt`` command which
test if the object at stack level 1 is greater than the object at level
0.

::

   /fac
   {
     dup    % duplicate the argument, since we still need it.
     1 gt   % If n > 1 we call fac recursively
     {      % according to fac(n)=n*fac(n-1)
       dup
       1 sub fac % call fac with n-1 as argument
       mul       % multiply the result with the argument
     } if
   } def

This example also shows how procedures can be called *recursively*. It
is, however, important to supply a *termination condition* for the
recursion like in this example.

Comparison functions
--------------------

Comparison functions are used to compare objects. The result of
comparison functions are of type ``/booltype`` and can be used for
logical functions and conditional structures.

Command Description ``eq`` Test whether two objects are equal. ``ne``
Test whether two objects are not equal. ``gt`` Test whether the object
at level 1 is greater than the object at level 0. ``lt`` Test whether
the object at level 1 is less than the object at level 0. ``leq`` Test
whether the object at level 1 is less than or equal to the object at
level 0. ``geq`` Test whether the object at level 1 is greater than or
equal to the object at level 0.

Logical functions
-----------------

Command Description ``not`` Negates a bool. ``and`` Returns true if both
arguments are true. ``or`` Returns true if at least one of the arguments
is true. ``xor`` Returns true if and only if one of the arguments is
true.  

The *if-ifelse* structure
-------------------------

Command Description ``bool proc if`` Executes ``proc`` if the boolean is
true. ``bool proc_1 proc_2 ifelse`` Executes ``proc_1`` if the boolean
is true and ``proc_2`` otherwise.

Example
~~~~~~~

::

   SLI ] 1 2 eq {(Equal!) = } { (Not equal !) =} ifelse
   Not equal !
   SLI ] 2 2 eq {(Equal!) = } { (Not equal !) =} ifelse
   Equal!

The *case-switch* structure
---------------------------

While the commands ``if`` and ``ifelse`` test only one condition, the
*case-switch* structure can be used to test a number of different
conditions.

The *case-switch* structure has the general form:

::

   mark
    bool_1 proc_1 case
    bool_2 proc_2 case
           :
    bool_n proc_n case
   switch

In this structure, ``proc_i`` is executed, if the corresponding value of
``bool_i`` is true.

Sometimes it is necessary to provide a default procedure, which is
evaluated if none of the boolean is true.

The *case-switchdefault* structure has the general form

::

   mark
    bool_1 proc_1 case
    bool_2 proc_2 case
           :
    bool_n proc_n case
    procdefault
   switchdefault

Here, ``procdefault`` is executed if none of the booleans was true.

Loops
-----

Loops and control structures are commands that take procedure objects as
arguments.

Infinite loops
~~~~~~~~~~~~~~

The simplest loop is performed by the command ``loop``:

::

   SLI ] {(Hello World) =} loop
   Hello World
   Hello World
   Hello World
   Hello World
   Hello World
   Hello World
   Hello World
   Hello World
      :

``loop`` performs the procedure repeatedly and thus in the example, an
infinite succession of the words “Hello World” is printed. The only way
to leave a ``loop``-structure is to call the command ``exit`` somewhere
inside the loop:

::

   SLI ] 0
   SLI [1] { 1 add dup  (Hello World) = 10 eq {exit} if }
   SLI [2] loop

it prints ten times ‘Hello World’. First the initial value 0 is pushed
on the operand stack. The procedure adds 1 in each cycle and takes care
that one copy of the counter stays on the stack to serve as the initial
value for the next cycle. After the message has been printed, the stop
value 10 is pushed and is compared with the counter. If the counter is
equal to 10, the nested procedure s executed. This procedure then
executes the command ``exit``, and interrupts the loop.

Command Description ``proc loop`` Repeatedly execute procedure ``proc``.
``exit`` Exit the innermost loop structure.

Finite loops
~~~~~~~~~~~~

The last example can be implemented much easier, using a ``repeat``
loop. ``repeat`` takes two arguments: An integer, and a procedure
object. The integer determines how often the procedure is executed.
Thus, in order to print ten times “Hello World” we write:

::

   SLI ] 10 { (Hello World) = } repeat

Sometimes, one needs to know the counter of the loop and one may also be
interested in influencing the step-size of the iterations. For this
purpose SLI offers the ``for``-loop. ``for`` is called like this:

::

   start step stop proc for

``for`` executes the procedure ``proc`` as long as the counter is
smaller than the stop-value (for positive step values) (please refer to
reference *RedBook* for the exact termination conditions).

In each cycle, the current value of the counter is pushed automatically.
This value can be consumed by the procedure. Actually, in very long
running loops, the counter must be removed by the procedure in order to
avoid stack overflow. The following example prints the first ten cubic
numbers:

::

   SLI ] 1 1 10 { dup mul = } for
   1
   4
   9
   16
   25
   36
   49
   64
   81
   100                                                                             
   SLI ]

Command Description ``n proc repeat`` Execute procedure proc n times.
``i s e proc for`` Execute procedure proc for all values from i to e
with steps. ``array proc forall`` Execute procedure proc for all
elements of ``array``. ``array proc forallindexed`` Execute procedure
proc for all elements of ``array``. ``array proc Map`` Apply ``proc`` to
all elements of ``array``. ``array proc MapIndexed`` Apply ``proc`` to
all elements of ``array``. ``x proc n NestList`` Gives a list of the
results of applying ``proc`` to\ ``x`` 0 through ``n`` times.
