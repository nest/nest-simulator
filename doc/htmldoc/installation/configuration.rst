.. _config_options:

Configuration Options
=====================

The behavior of the NEST executable can be tweaked by supplying it
with command line switches, SLI scripts, and additional parameters for
the scripts.


Command line switches for the nest executable
---------------------------------------------

Type

::

   nest --help

to find out about NEST's command-line parameters.

::

  usage: nest [options] [ - | file [file ...] ]
    file1 file2 ... filen     read SLI code from file1 to filen in ascending order
                              Quits with exit code 126 on error.
    -   --batch               read SLI code from stdin/pipe.
                              Quits with exit code 126 on error.
    -c cmd                    Execute cmd and exit
    -h  --help                print usage and exit
    -v  --version             print version information and exit
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

   <nest_install_dir>/nest <file>

If you are a Vim user and require support for SLI files, please refer to
our :ref:`vim_sli`.

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
"="-symbol. It then converts the first element (lambda, gamma) to a
literal and the second argument (the number) to an integer. Using
``mark`` and ``>>``, the content of the userargs array is added to a
dictionary, which is stored under the name ``args``. The second line
just prints the content of the lambda variable.
