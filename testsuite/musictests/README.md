# `musictests` folder

This directory contains tests for the MUSIC interface of NEST [1].

Each test consists of the following parts:

* a MUSIC configuration file with the extension `.music`
* one or more `.sli` scripts specifying the simulation
* optionally a shell script with the extension `.sh`, which is run
  after the actual test simulation and can be used to test resulting
  data files for consistency

MUSIC tests are run using the `mpirun` function defined in `~/.nestrc`
and with the total number of processes that is requested in the MUSIC
configuration file.

Each test is expected to exit with an exit code of 0 if it
succeeds. All other exit codes indicate an error.

If the name of the test contains the word "failure", the test is
expected to fail. In this case, it is expected to exit with a non-zero
exit code and is considered a failed test otherwise.


## References
[1] Djurfeldt M, Hjorth J, Eppler JM, Dudani N, Helias M, Potjans TC, Bhalla US, Diesmann M, Kotaleski JH, Ekeberg O (2010) Run-time interoperability between neuronal network simulators based on the MUSIC framework. Neuroinformatics 8(1):43-60. doi:[10.1007/s12021-010-9064-z](http://dx.doi.org/10.1007/s12021-010-9064-z).