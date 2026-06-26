pynest folder
=============

This directory contains the source code of PyNEST, the Python bindings
to the NEST kernel. A detailed explanation of PyNEST can be found in
Zaytsev and Morrison (2014) :footcite:p:`Zaytsev2014` and Eppler et al. (2009) :footcite:p:`Eppler2009`.

PyNEST will be compiled together with NEST by default. If you want to
disable it, pass

.. code-block::

   -Dwith-python=OFF

as an argument to ``cmake``. By default, ``make install`` will install PyNEST
into the currently active Python environment (system, or virtual).  This is
usually under one of the prefixes in the list ``python -c "import sys;
print(sys.path)"``, in a directory like ``$(pyexecdir)``, which is often
expanded as follows:

.. code-block::

   $(prefix)/lib{,64}/pythonX.Y/site-packages/nest

All required files will automatically be installed in the right location inside
the environment, so no further setup is required.

In very special cases, you may have specified a ``CMAKE_INSTALL_PREFIX``
different from the current Python environment, which makes it necessary to set
the ``PYTHONPATH`` variable explicitly to the corresponding ``site-packages``
directory in the install location. Note however, that it is the users
responsibility to keep the Python loaded at run-time in sync with the Python
that was loaded at compile time.


References
----------

.. footbibliography::
