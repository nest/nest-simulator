.. _release_3.11:

What's new in NEST 3.11
=======================

This page contains a summary of important breaking and non-breaking
changes from NEST 3.10 to NEST 3.11.

If you transition from a NEST 2.x version, please see our extensive
:ref:`transition guide from NEST 2.x to 3.0 <refguide_2_3>` and the
:ref:`list of updates for previous releases in the 3.x series <whats_new>`.

The release of 3.11 brought a lot of changes with over ?? PRs
including many bug fixes and enhancements as detailed in the `release notes
on GitHub <https://github.com/nest/nest-simulator/releases/>`_. Here are some
of the more interesting developments that happened:


Axonal delays
-------------

NEST now supports specifying separate axonal and dendritic delays for synapses.
Up to now, NEST has treated all transmission delays as purely dendritic. This
distinction matters for spike-timing dependent plasticity (STDP), because the
weight update depends on when pre- and postsynaptic spikes arrive at the
synapse. If the axonal delay is larger than the dendritic delay, NEST now
corrects weight changes retroactively when a postsynaptic spike arrives later.

The new synapse model ``stdp_pl_synapse_hom_ax_delay`` accepts the parameters
``axonal_delay`` and ``dendritic_delay`` instead of ``delay``:

.. code-block:: python

   nest.Connect(pre, post, syn_spec={"synapse_model": "stdp_pl_synapse_hom_ax_delay",
                                     "axonal_delay": 1.0, "dendritic_delay": 1.0})

Supplying ``delay`` to this model, or ``axonal_delay`` and ``dendritic_delay``
to any other synapse model, raises an error. The neuron models
``iaf_psc_alpha``, ``iaf_psc_exp``, ``iaf_psc_delta`` and ``parrot_neuron``
support STDP with predominantly axonal delays. All other synapse models
continue to treat ``delay`` as purely dendritic, so existing simulations are
not affected. Axonal delay support does not increase the memory used per
synapse.

See :ref:`delays` and the :doc:`axonal delays example </auto_examples/axonal_delays>`
for details.


New models
----------

- The new :doc:`ou_noise_generator </models/ou_noise_generator>` injects a
  temporally correlated noise current that follows an Ornstein-Uhlenbeck
  process. It is set by the parameters ``mean``, ``std`` (stationary standard
  deviation), ``tau`` (correlation time constant) and ``dt`` (update
  interval). As with ``noise_generator``, each target receives an independent
  noise current.


Faster ``GetConnections()``
---------------------------

``GetConnections()`` with a ``source`` or ``target`` selection is now much
faster and scales with the number of threads and MPI processes. In a benchmark
selecting 5 million out of 125 million connections, the time dropped from about
10 s to well under 1 s.


Random parameters aligned with NumPy
------------------------------------

The non-spatial random parameters now behave like their ``numpy.random``
counterparts: ``nest.random.normal()`` and ``nest.random.lognormal()`` accept
``std=0``, and ``nest.random.exponential()`` accepts ``beta=0``, while a
negative ``beta`` raises an error. Spatial random parameters still require
strictly positive values.

In addition, NEST now raises an error if you create parameter objects before
setting the number of threads, because such parameters would not work
correctly.


Changes that may affect simulation results
------------------------------------------

- NEST now uses the ``std::expm1()`` function if available, instead of a
  "homegrown" implementation (``std::expm1()`` should be available on almost
  all systems). This may lead to different results than with earlier versions
  of NEST, especially when using precise-spiking neurons. The old code is still
  available; undefine ``HAVE_EXPM1`` in ``<build_dir>/libnestutil/config.h``
  to use it on systems where ``std::expm1()`` is available.

- Random selection of synaptic elements in structural plasticity now uses the
  C++ standard library function ``std::sample()`` instead of custom code. The
  results are statistically equivalent but not identical to earlier versions
  of NEST.


CMake configuration modernization
---------------------------------

A large number of irrelevant checks have been removed, so CMake completes
considerably faster. Several CMake configuration options have been removed,
renamed, or extended. Unrecognised ``-D`` options now produce a hard error
during configuration. On macOS, ``libomp`` installed with Homebrew is now
detected automatically.

**Python bindings are always built.**
The ``-Dwith-python`` option has been removed. Python 3.10 or later is now an
unconditional build requirement. To select a specific Python installation use
``-DPython_EXECUTABLE=/path/to/python``.

**Time resolution is now a runtime-only setting.**
The compile-time options ``-Dtics_per_ms`` and ``-Dwith-tics-per-ms`` (and
their ``tics_per_step`` equivalents) have been removed. The default values
(1000 tics per ms, 100 tics per step) are compiled in, and can be changed at
runtime as before via ``nest.set(tics_per_ms=...)``.

**Library options are now plain booleans; use** ``<Package>_ROOT`` **to pin a location.**
All ``-Dwith-<library>`` options are now simple ``ON``/``OFF`` switches.
To restrict CMake's search to a specific installation prefix, set the
corresponding ``<Package>_ROOT`` variable separately. For example::

    cmake -Dwith-gsl=ON -DGSL_ROOT=/opt/homebrew/opt/gsl ...

When ``<Package>_ROOT`` is set, CMake restricts its search to that prefix and
reports an error if the library is not found there, preventing silent fallback
to a system-wide installation. The affected variables are ``GSL_ROOT``,
``Boost_ROOT``, ``MPI_ROOT``, ``OpenMP_ROOT``, ``LTDL_ROOT``, ``HDF5_ROOT``,
``Music_ROOT``, and ``SIONlib_ROOT``.

**Renamed options.**
The following options have been renamed for consistency. The old names produce
a hard error with a pointer to the new name.

.. list-table::
   :header-rows: 1
   :widths: 40 40

   * - Old option
     - New option
   * - ``-Dstatic-libraries=[OFF|ON]``
     - ``-Dwith-static-linking=[OFF|ON]``
   * - ``-Dtarget-bits-split=['standard'|'hpc']``
     - ``-Dwith-target-bits-split=['default'|'hpc']``
   * - ``-Dwith-intel-compiler-flags=<flags>``
     - ``-Dwith-intel-compiler-strict-math=[ON|OFF]``
   * - ``-Dcythonize-nest=[ON|OFF]``
     - ``-DNESTKERNEL_API_CXX=<path>``

For the full list of available options see :ref:`cmake_options`.


ConnectionGenerator and support for libneurosim removed
-------------------------------------------------------

``ConnectionGenerator`` and the support for ``libneurosim``, which depended on
it, have been been removed in NEST 3.11,
including the ``-Dwith-libneurosim`` option for CMake.


NEST Server now ships separately
--------------------------------

As of NEST 3.11, NEST Simulator no longer includes NEST Server in its source code or Python package.
NEST Server is now available as a separate Python package from `pypi.org <https://pypi.org/project/nest-server/>`_ or as source code from https://github.com/nest/nest-server.

For more information, see https://github.com/nest/nest-server.


Documentation
-------------

- The :ref:`PyNEST tutorial <pynest_tutorial>` can now be downloaded as a
  Python script or Jupyter notebook.
- All references in the documentation now come from a single central
  bibliography, so citations are consistent across pages.
- New `developer documentation <https://nest.github.io/nest-simulator>`_ for
  the C++ code base is now available.
