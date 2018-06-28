# `doc` folder

The documentation of NEST. This includes design documents and user 
documentation. The online help is generated during the [installation][install] 
of NEST and installed to the installation directory. We use [doxygen][doxygen] 
comments extensively throughout NEST. `make doc` will extract them from the 
source code and install the developer documentation into the doc directory.

[install]: http://nestwwwdev.inm.kfa-juelich.de/nest-simulator/installation-2/ "Installation documentation"
[doxygen]: http://doxygen.org/ "doxygen Homepage"

## Local install

### Requirements

To keep everything simple and easy to maintain, the installation with conda
is described here.
So all you need is conda. If you don't have it yet,
[miniconda](https://conda.io/miniconda.html) is recommended.

### Installation

With the following steps a full development environment is installed - even 
NEST 2.14.0.

Cd to the doc/ folder (if you're not already there).

    cd ./doc

Create and activate the environment.

    conda env create -f environment.yml
    conda update -n base conda
    source activate doc

Now generate the html files. They are then located in ./docs/build/html.
    
    make html

> 'make pdf', 'make latex' etc. are possible, too. For more information do a
  'make help'.

If you want to deactivate and/or delete the build environment:

    source deactivate
    conda remove --name documentation --all