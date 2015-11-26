## README for the Neural Simulation Tool NEST [![Build Status](https://travis-ci.org/nest/nest-simulator.svg?branch=master)](https://travis-ci.org/nest/nest-simulator)
Generic installation instructions can be found in the file INSTALL that
you received with the NEST sources.

Inside NEST, you can run the command `help` to find documentation and
learn more about the available commands.

Please see `${prefix}/share/doc/nest/README.md` for information about
the Python bindings to NEST.

For information on the NEST Initiative, please visit it's homepage at
http://www.nest-initiative.org

For copyright information please refer to the file LICENSE and to the
information header in the source files.

Emacs users may use the SLI mode, which provides syntax highlighting
for SLI. To install it, add the following lines to your `.emacs` file:
```
  (load-library "${prefix}/share/nest/extras/emacs/postscript-sli")
  (load-library "${prefix}/share/nest/extras/emacs/sli")
```
