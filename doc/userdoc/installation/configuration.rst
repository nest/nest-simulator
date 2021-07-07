Configuration Options
=====================

There are two main ways of configuring NEST at runtime, via the configuration file or command line switches.

NEST configuration file
-----------------------

Upon importing the NEST Python module or starting the ``nest`` executable from the command line for the first time, NEST will create a
configuration file called ``.nestrc`` in your home directory.

By adapting this file, you can set a number of options:

* The browser for displaying the helpdesk
* The pager for showing the built-in help in the terminal
* The ``mpirun`` command for :doc:`parallel execution <../guides/parallel_computing>` of the testsuite

In case your MPI Implementation requires special options (e.g. ``--oversubscribe`` to allow the use of more
processes than available compute cores in OpenMPI versions above 3.0), you can add them to the ``mpirun`` command as shown in
the following example:

::

    /mpirun
    [/integertype /stringtype /stringtype]
    [/numproc     /executable /scriptfile]
    {
     () [
      (mpirun --oversubscribe -np ) numproc cvs ( ) executable ( ) scriptfile
     ] {join} Fold
    } Function def

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
our :doc:`../contribute/styleguide/vim_support_sli`.

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
