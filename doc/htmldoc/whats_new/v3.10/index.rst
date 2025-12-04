.. _release_3.10:

What's new in NEST 3.10
=======================

This page contains a summary of important breaking and non-breaking
changes from NEST 3.9 to NEST 3.10.

If you transition from a NEST 2.x version, please see our extensive
:ref:`transition guide from NEST 2.x to 3.0 <refguide_2_3>` and the
:ref:`list of updates for previous releases in the 3.x series <whats_new>`.

The release of 3.10 brought a lot of changes with XXX PRs
including YYY bug fixes and ZZZ enhancements as detailed in the `release notes
on GitHub <https://github.com/nest/nest-simulator/releases/>`_. Here are some
of the more interesting developments that happened:

Removal of the SLI interpreter
------------------------------

The SLI interpreter was the original user interface of NEST since its beginnings as SYNOD
in the mid-1990s. When we added a Python user interface in the late 2000s, we built it on
top of the SLI interface. Today, all practical use of NEST appears to be via the Python
interface and SLI is only used as an intermediate layer. Therefore, we decided to eliminate
this layer and to directly interface PyNEST with the NEST simulator kernel. All SLI-related
code is therefore removed from NEST.

This has been a huge effort over several years. The original design and implementation of the
new interface was done by Jochen Eppler and Håkon Mørk. Many developers through several hackathons
contributed to porting all SLI-based tests to Python to ensure we did not break anything in
the process.

What does this mean for you as a NEST user?
...........................................

Fortunately, almost nothing, except that you can now `pip install nest-simulator` and that scripts might
construct your networks slightly faster.

If you have implemented your models using PyNEST, the only change you will need to make is to replace

.code::
  nest.set_verbosity("M_WARNING")

with

.. code-block::
  nest.verbosity = nest.VerbosityLevel.WARNING

We have also deprecated `nest.GetStatus()` and `nest.SetStatus()`, so over time you may want to replace

.code::
  nest.GetStatus(node_coll)

with

.code::
  node_coll.get()

and correspondingly for connection collections and use `nest.get()/set()` for kernel status parameters.

If you have used direct interaction with SLI from the Python level through `sli_func()`, `sli_run()`
or `sr()` you will need to now use the direct interface. Please get in touch via the NEST User
mailing list for advice.

If you still have your models implemented in SLI, it is time now to migrate to PyNEST.

This has been a huge change for us and we would be very interested in hearing about your experiences!


What does this mean for you as a developer?
...........................................

The key change from a developer perspective are that the entire SLI interpreter code has been
removed, noticeably reducing compile times. We therefore no longer have the `SLIModule`` concept.
Also, `Dictionary`, `Datum`, and `Token` are a matter of the past. Instead, we now
have class `dictionary` based directly on `std::map` using `boost::any` to store entries of
arbitrary type. Instead of our own `lockPTR`, we now use `std::unique_ptr` to manage objects with
reference counting. Where changes are necessary in code for neuron or synapse models, they will
likely be limited to slightly different notation in the `set()/get()` methods to support the new
`dictionary` class.

To test whether certain errors are raised when writing tests, instead of

.code::
  with pytest.raises(nest.kernel.NESTErrors.IllegalConnection):

you now have

.code::
  with pytest.raises(nest.NESTErrors.IllegalConnection):

When


Model improvements
------------------


Documentation additions
-----------------------
