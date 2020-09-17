pynest folder
=============

This directory contains the source code of PyNEST, the Python bindings
to the NEST kernel. A detailed explanation of PyNEST can be found in
Zaytsev and Morrison (2014) [1]_ and Eppler et al. (2009) [2]_.

PyNEST will be compiled together with NEST by default. If you want to
disable it, pass

.. code-block::

   -Dwith-python=OFF

as an argument to ``cmake``. By default, ``make install`` will install
PyNEST to ``$(pyexecdir)``, which is often expanded as follows:

.. code-block::

   $(prefix)/lib{,64}/pythonX.Y/site-packages/nest

.. note::

   Since NEST 3.0, support for Python 2 has been dropped. Please use
   Python 3 instead.

To make the PyNEST module available to the Python interpreter, source
``nest_vars.sh`` from the NEST installation directory (``source
<nest_install_dir>/bin/nest_vars.sh``). This will add the PyNEST installation
path to the ``PYTHONPATH`` environment variable.

For help on a NEST object OBJ in PyNEST, type ``nest.help(OBJ)``. To find
out more about NEST, type ``nest.helpdesk()``.

References
----------

.. [1] Zaytsev YV and Morrison A (2014) CyNEST: a maintainable
       Cython-based interface for the NEST simulator. Front.
       Neuroinform. 8:23. http://dx.doi.org/10.3389/fninf.2014.00023

.. [2] Eppler JM, Helias M, Muller E, Diesmann M and Gewaltig M-O PyNEST
       (2009) A convenient interface to the NEST simulator. Front.
       Neuroinform. 2:12. http://dx.doi.org/10.3389/neuro.11.012.2008
