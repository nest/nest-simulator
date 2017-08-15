# Getting started

[Documentation](documentation.md)
This page contains the steps that you should follow right after you
[installed NEST](installation.md). Another good
starting point is the help page, which is available as command `help` in SLI and
`nest.help()` in PyNEST

## Set up the integrated helpdesk

The command `helpdesk` needs to know which browser to launch in order to display
the help pages. The browser is set as an option of `helpdesk`. Please see the
file `~/.nestrc` for an example setting `firefox` as browser. Please note that
the command `helpdesk` does not work if you have compiled NEST with MPI support,
but you have to enter the address of the helpdesk
(`file://$PREFIX/share/doc/nest(`) manually into the browser. Please
replace `$PREFIX` with the prefix you chose during the configuration of NEST.
If you did not explicitly specify one, it is most likely set to `/usr` or
`/usr/local` depending on what system you are.

## Tell NEST about your MPI setup

If you compiled NEST with support for distributed computing via MPI, you have to
 tell it how your `mpirun`/`mpiexec` command works by defining the function
 mpirun in your `~/.nestrc` file. This file already contains an example
 implementation that should work with [OpenMPI](http://www.openmpi.org) library.

## Creating Models with NEST

After NEST is installed and configured properly, you can start to build your
model.

### Examples

A good starting point to learn more about modeling in NEST are the
example networks that come together with NEST.

### Where does data get stored

By default, the data files produced by NEST are stored in the directory from
where NEST is called. The location can be changed by changing the property
`data_path` of the root node using
`nest.SetKernelStatus("data_path", "/path/to/data")`. This property can also be
set using the environment variable `NEST_DATA_PATH`. Please note that the
directory `/path/to/data` has to exist. A common prefix for all data files can
be set using the property `data_prefix` of the root node by calling
`nest.SetKernelStatus("data_prefix", "prefix")` or setting the environment
variable `NEST_DATA_PREFIX`.
