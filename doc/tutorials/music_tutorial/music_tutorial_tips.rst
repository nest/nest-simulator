Practical Tips
========================

Start MUSIC using mpirun
--------------------------

    There is an alternative way to start a MUSIC simulation without the ``music``
    binary. The logic for parsing the configuration file is built into
    the library itself. So we can start each binary explicitly using
    mpirun. We give the config file name and the corresponding app label
    as command line options:

    .. code-block:: sh

          mpirun -np 2 <binary> --music_config <config file> --app-label <label> : ...

    So to start a simulation with the sendsimple.py and recv programs,
    we can do:

    .. code-block:: sh

          mpirun -np 2 ./sendsimple.py --music-config simplepy.music --app-label from :/
             -np 2 ./recv --music-config simplepy.music --app-label to

    This looks long and cumbersome, of course, but it can be useful.
    Since it’s parsed by the shell you are not limited to what the
    ``music`` launcher can parse, but the binary can be
    anything the shell can handle, including an explicit interpreter
    invocation or a shell script.

    As a note, the config file no longer needs to contain the right
    binary names. But it *does* need to have a non-empty
    ``binary=<something>`` line for each process. The
    parser expects it and will complain (or crash) otherwise. Also, if
    you try to process comand line options in your Pynest script, it is
    very likely you will confuse MUSIC.

Disable Messages
-----------------

    NEST can be quite chatty as it connects things, especially with large
    networks. If we don’t want all that output, we can tell it to display only
    error messages:

    .. code:: python

        nest.sli_run("M_ERROR setverbosity")

    There is unfortunately no straightforward way to suppress the
    initial welcome message. That is somewhat unfortunate, as they add
    up quickly in the output of a simulation when you use more than a
    few hundred cores.

Comma as decimal point
------------------------

    Sorting output spikes may fail if you, like the authors, come from a
    country that uses a comma as decimal separator and runs your computer in
    your native language. The problem is that sort respects the language
    settings and expects the decimal separator to be a comma. When it sees the
    decimal point in the input it assumes the numeral has ended and sorts only
    on the integer part.

    The way to fix this is to set ``LC\_ALL=C`` before
    running the sort command. In a script or in the terminal you can do:

    .. code-block:: sh

          export LC_ALL=C
          cat output-*|sort -k 2 -n >output.spikes

    Or, if you want to do this temporarily for only one command:

    .. code-block:: sh

          cat output-*|LC_ALL=C sort -k 2 -n >output.spikes

Build Autotool-enable project
------------------------------

    To build an Autotool-enabled C/C++ project, you don’t actually need to
    be in the main directory. You can create a subdirectory and build
    everything from there. For instance, with the simple C++ MUSIC project
    in section :doc:`C++ build <music_tutorial_3>`, we can do this:

    .. code-block:: sh

          mkdir build
          cd build
          ../configure
          make

    Why do that? Because all files you generate when building the
    project ends up under the ``build`` subdirectory,
    keeping the source directories completely clean and untouched. You
    can have multiple builds ``debug``,
    ``noMPI`` and so on with different build options
    enabled, and you can completely clean out a build simply by deleting
    the directory.

    This is surely completely obvious to many of you, but this author is
    almost ashamed to admit just how many years it took before I
    realized you could do this. I sometimes actually kept two copies of
    projects checked out just so I could build a separate debug version.



