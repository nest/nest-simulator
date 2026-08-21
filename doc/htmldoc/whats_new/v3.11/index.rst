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


Model improvements
------------------

- NEST now uses the ``std::expm1()`` function if available instead of a
  "homegrown" algorithm based on GSL (should be on almost all systems).
  This may lead to different results than with earlier versions of NEST,
  especially when using precise-spiking neurons. The old code is still
  available; undefine ``HAVE_EXPM1`` in ``<build_dir>/libnestutil/config.h``
  to use it on systems where ``std::expm1()`` is available.


CMake configuration modernization
---------------------------------

A large number of irrelevant checks have been removed, so CMake completes
considerably faster. Several CMake configuration options have been removed,
renamed, or extended. Unrecognised ``-D`` options now produce a hard error
during configuration.

**Python bindings are always built.**
The ``-Dwith-python`` has been removed. Python 3.10 or later is now an
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
``LibNeurosim_ROOT``, ``Music_ROOT``, and ``SIONlib_ROOT``.

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
