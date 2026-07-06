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
