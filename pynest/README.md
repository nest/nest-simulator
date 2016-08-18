# `pynest` folder

This directory contains the source code of PyNEST, the Python bindings
to the NEST kernel. A detailed explanation of PyNEST can be found in

    Eppler JM, Helias M, Muller E, Diesmann M and Gewaltig M-O
    PyNEST: A convenient interface to the NEST simulator
    Front. Neuroinform. (2009) 2:12. doi:10.3389/neuro.11.012.2008

and

    

PyNEST will be compiled together with NEST by default. If you want to
disable it, pass

    -Dwith-python=OFF

as an argument to `cmake`. By default, `make install` will install
PyNEST to `$(pyexecdir)`, which is often expanded as follows:

    $(prefix)/lib{,64}/pythonX.Y/site-packages/nest


To force the usage of a specific Python version pass

    -Dwith-python=2  or  -Dwith-python=3

as an argument to `cmake`.


To make the PyNEST module available to the Python interpreter, add the
PyNEST installation path (without the final '/nest') to the PYTHONPATH
environment variable.

For help on a NEST object OBJ in PyNEST, type nest.help(OBJ). To find
out more about NEST, type nest.helpdesk().
