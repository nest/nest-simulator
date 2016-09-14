# Environment variables

NEST's behavior can be influenced by several environment variables in
order to allow easy embedding of simulations into scripts for multiple
runs.

`DELAY_PYNEST_INIT`
  : If set to 1, this prevents the call of PyNEST's `init()` function
    when importing the `nest` module. This is used in advanced
    scenarions, e.g. when additional commandline arguments have to be
    added to the `argv` variable of the NEST kernel. When using this,
    `init()` has to be called manually.

`NEST_DATA_PATH`
  : The directory, where all recording files of NEST will be written
    to. This makes automated runs of NEST easier, as the result files
    of different runs can be sorted to different directories without
    changing the script.

`NEST_MODULE_PATH`
  : This defines the serach path for extension modules. The content of
    this variable is passed to libltdl's `lt_dlsetsearchpath()`
    function.  See the documentation of libltdl for details.

`NEST_MODULES`
  : A colon separated list of extensions that will be loaded upon
    startup using the `Install` SLI command. The path of the modules
    has to be set using `NEST_MODULE_PATH`

`NESTRCFILENAME`
  : The name of NEST's configuration file (default: `~/.nestrc`). This
    can be used to set different configurations for different
    installations or to accomodate different machine setups.

`SLI_PATH`
  : A colon separated list of paths in which NEST searches for SLI
    files. These are added to the search path.

In addition to the NEST specific environment variables above, NEST
also respects the following standard POSIX shell variables:

`COLUMNS`
  : A number of columns to which NEST adapt the width of its textual
    output (errors, warnings, etc.).

`EDITOR`
  : The editor to be used when editing files on the fly.

`PAGER`
  : The pager to be used for displaying the help paged.

`TERM`
  : The terminal program to be used when forking an external program.
