An Introduction to SLI
======================

Introduction
------------

NEST can be started by typing

::

   <nest_install_dir>

at the command prompt. You should then see something like this:

::

   gewaltig@jasmin-vm:~$ nest
               -- N E S T 2 beta --

     Copyright 1995-2009 The NEST Initiative
      Version 1.9-svn Feb  6 2010 00:33:50

   This program is provided AS IS and comes with
   NO WARRANTY. See the file LICENSE for details.

   Problems or suggestions?
     Website     : https://www.nest-initiative.org
     Mailing list: users@nest-simulator.org

   Type 'help' to get more information.
   Type 'quit' or CTRL-D to quit NEST.

Command line switches
---------------------

Type

::

   nest --help

to find out about NEST’s command-line parameters.

::

   gewaltig@jasmin-vm:~$ nest --help
   usage: nest [options] [file ..]
     -h  --help                print usage and exit
     -v  --version             print version information and exit.

     -   --batch               read input from a stdin/pipe
         --userargs=arg1:...   put user defined arguments in statusdict::userargs
     -d  --debug               start in debug mode (implies --verbosity=ALL)
         --verbosity=ALL       turn on all messages.
         --verbosity=DEBUG|STATUS|INFO|WARNING|ERROR|FATAL
                               show messages of this priority and above.
         --verbosity=QUIET     turn off all messages.

SLI scripts
-----------

Scripts can be run by typing:
::

   <nest_install_dir> <file>

If you are a Vim user and require support for SLI files, please refer to
our :doc:`../contribute/templates_styleguides/vim_support_sli`.

Supplying SLI scripts with parameters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Using the ``--userargs=arg1:...`` command line switch, it is possible to
supply a SLI script with parameters from the outside of NEST. A common
use case for this are parameter sweeps, where the parameters are defined
in a bash script and multiple instances of NEST are used to test one
parameter each. A bash script for this could look like this:

::

   for lambda in `seq 1 20`; do
     for gamma in `seq 1 5`; do
       nest --userargs=lambda=$lambda:$gamma=$gamma simulation.sli
     done
   done

The corresponding SLI script ``simulation.sli`` could use the supplied
parameters like this:

::

   /args mark statusdict/userargs :: {(=) breakup} Map { arrayload pop int exch cvlit exch } forall >> def
   args /lambda get ==

The first line first gets the array of user supplied arguments
(``userargs``) from the ``statusdict`` and breaks each element at the
“=”-symbol. It then converts the first element (lambda, gamma) to a
literal and the second argument (the number) to an integer. Using
``mark`` and ``>>``, the content of the userargs array is added to a
dictionary, which is stored under the name ``args``. The second line
just prints the content of the lamda variable.

SLI user manual
---------------

This manual gives a brief overview of the SLI programming language.

1. `First Steps <first-steps.md>`__
2. `Objects and data types <objects-and-data-types.md>`__
3. `Programming in SLI <programming-in-sli.md>`__
4. `Using files and keyboard
   input <using-files-and-keyboard-input.md>`__
5. `Neural simulations <neural-simulations.md>`__
