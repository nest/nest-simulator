:orphan:

First Steps
===========

Overview
--------

SLI is the simulation language interface of NEST. It is a stack language
where each command expects to find its arguments on the stack.

A stack is a place where data can be stored. The stack is organized into
levels and the data can be thought of being on top of each other. As new
data is entered, it is placed on the *top* of the stack. If a piece of
data is removed or manipulated, this is usually done to the top level of
the stack. The levels of the stack are numbered from zero onwards. Level
zero corresponds to the top of the stack.

Initially the stack is empty. In general, you enter data onto the stack
and then execute commands to manipulate the data.

Each command expects to find its arguments on the stack. When a SLI
command is executed, it usually removes all arguments from the stack and
pushes one or more results back on the stack. The basic concepts of
stack operation are:

-  Commands that require *arguments* take their arguments from the
   stack. Thus, this data must be present before you execute the
   command.

-  The arguments are then removed by the command as it is executed.

-  Any results which are produced by the command are returned to the
   stack, so you can use them in other operations.

Commands with one argument
--------------------------

Commands which need one argument take their argument from the top of the
stack. If the command produces a result, it is placed on top of the
stack, after the argument has been removed. Thus, the argument is
replaced by the result:

Example
~~~~~~~

::

   SLI ] 10 log =
   1

Here, the command ``log`` is used to compute the decadic logarithm of
10. Then, the command ``=`` is used to display the result of this
computation.

Commands with more arguments
----------------------------

Commands which need more than one argument, take their arguments from
level 0, 1, 2, and so forth and return their result to level 0, the top
of the stack. Examples are the arithmetic functions ``add``, ``sub``,
``mul``, and ``div``, which take two arguments and return one result.

::

   SLI ] 1 2 add =
   3
   SLI ] 1 2. div =
   0.5

So far, we have used the command ``=`` to display the top object on the
stack. In addition, this command removes the object. You can also list
the contents of the stack without changing it.

::

   SLI ] 1 2
   SLI [2] stack
   2
   1
   SLI [2] add
   SLI [1] stack
   3
   SLI [1]

Using previous results
----------------------

Chain calculations are calculations which involve more than one
operation. A stack is particularly useful for chaining
operations,because it retains intermediate results.

This example shows, how the stack can be used for chain calculations.
Calculate (10+13) \\(cdot\) (8-12)

::

   SLI [1] 10 13 add 8 12 sub
   SLI [2] stack
   -4
   23
   SLI [2] mul =
   -92                                                                              

Notice that the results of the fist two operations remain on the stack,
until they are used in the multiplication.

Exchanging the first two stack levels
-------------------------------------

The command ``exch`` exchanges the contents of the levels 0 and 1. This
is useful, if the order of objects on the stack does not match the order
required by the desired command.

Example 1
~~~~~~~~~

Calculate 1/ln(2).

::

   SLI ] 2 ln
   SLI [1] 1
   SLI [2] exch div
   SLI [1] =
   1.4427

Removing stack elements
-----------------------

The command ``pop`` removes the top object (level 0) of the stack. The
remaining items move up on the stack, so that the object which was at
level 1 is now at level 0.

The command ``clear`` clears the entire stack.

Duplicating the top element
---------------------------

The command ``dup`` duplicates the contents of the object at level 0 and
pushes the other element down one level. This command is useful if the
result of an operation is needed more than once in a chain calculation.

Example 2
~~~~~~~~~

Calculate (1+4/2) + exp(1+4/2)

::

   SLI ] 1 4 2.0 div add
   SLI [1] dup
   SLI [2] exp
   SLI [2] add
   SLI [1] =
   23.0855

Important stack commands
------------------------

Command Description ``=`` Print the object at level 0. ``==`` Print the
object at level 0 in syntax form. ``count`` Count the number of objects
on the stack. ``patsck`` Display the stack in syntax form. ``stack``
Display the stack. ``pop``, ``;`` Pop object from stack. ``npop`` Pop
``n`` objects from stack. ``dup`` Duplicate top object of stack.
``copy`` Copy the first n objects of the stack. ``index`` Copy the
``n``\ ’th object of the stack. ``roll`` Roll a portion of ``n`` stack
levels ``k`` times. ``exec`` Execute the top element on the stack.
