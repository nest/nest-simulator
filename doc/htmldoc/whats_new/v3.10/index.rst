.. _release_3.10:

What's new in NEST 3.10
=======================

This page contains a summary of important breaking and non-breaking
changes from NEST 3.9 to NEST 3.10.

If you transition from a NEST 2.x version, please see our extensive
:ref:`transition guide from NEST 2.x to 3.0 <refguide_2_3>` and the
:ref:`list of updates for previous releases in the 3.x series <whats_new>`.

The release of 3.10 brought a lot of changes with 70 PRs
including 8 bug fixes and 10 enhancements as detailed in the `release notes
on GitHub <https://github.com/nest/nest-simulator/releases/>`_. Here are some
of the more interesting developments that happened:


pip install nest-simulator
--------------------------

Now with the removal of SLI, detailed below, you can ``pip install nest-simulator``.
For details, see our :ref:`install guide <pip_install>`.

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

This has been a huge change for us and we would be very interested in hearing about your experiences!


What does this mean for you as a NEST user?
...........................................

Verbosity settings
++++++++++++++++++

If you have implemented your models using PyNEST, the only change you will need to make is to replace

.. code-block:: python

   nest.set_verbosity("M_WARNING")

with

.. code-block:: python

   nest.verbosity = nest.VerbosityLevel.WARNING

Configuration information
+++++++++++++++++++++++++

Various information about NEST capabilities and properties that were previously accessible
in varied ways are now collected in the ``nest.build_info`` dictionary:

.. code-block::

   {'built': 'Dec  4 2025 05:31:32',
    'datadir': '<path>',
    'docdir': '<path>',
    'exitcode': 0,
    'have_boost': True,
    'have_gsl': True,
    'have_hdf5': True,
    'have_libneurosim': False,
    'have_mpi': True,
    'have_music': False,
    'have_sionlib': False,
    'have_threads': True,
    'host': 'arm64-apple-darwin',
    'hostcpu': 'arm64',
    'hostos': 'darwin',
    'hostvendor': 'apple',
    'mpiexec': '/opt/homebrew/bin/mpiexec',
    'mpiexec_max_numprocs': '12',
    'mpiexec_numproc_flag': '-n',
    'mpiexec_postflags': '',
    'mpiexec_preflags': '',
    'ndebug': False,
    'prefix': '<path>',
    'test_exitcodes': {'abort': 134,
     'exception': 125,
     'fatal': 127,
     'scripterror': 126,
     'segfault': 139,
     'skipped': 200,
     'skipped_have_mpi': 202,
     'skipped_no_gsl': 204,
     'skipped_no_mpi': 201,
     'skipped_no_music': 205,
     'skipped_no_threading': 203,
     'success': 0,
     'unknownerror': 10,
     'userabort': 15},
    'threads_model': 'openmp',
    'version': '3.10'}


Message mechanism
+++++++++++++++++

The :py:func:`.message` function has received a slightly different user interface. Where you previously would write

.. code-block:: python

   nest.message("M_INFO", "Building network")

you now simply write

.. code-block:: python

   nest.message("Building network")

and the info-level severity is implicitly set. To control the severity-level of a message, use

.. code-block:: python

   nest.message("The next operation may take a very long time", nest.VerbosityLevel.WARNING)


Deprecated functions
++++++++++++++++++++

We have deprecated :py:func:`.GetStatus` and :py:func:`.SetStatus`, so over time you may want to replace

.. code-block:: python

   nest.GetStatus(node_coll)

with

.. code-block:: python

   node_coll.get()

and correspondingly for connection collections and use :py:func:`.get` and :py:func:`.set` for kernel status parameters.

We also deprecated :py:func:`.GetLocalNodeCollection`, because working only on nodes local to a given MPI
rank carries a high risk of writing incorrect code. If you think you need rank-specific code, rather get
in touch with us to see if there is a better solution using (or extending) NEST's built-in mechanisms.


No more SLI functions
+++++++++++++++++++++

If you have used direct interaction with SLI from the Python level through ``sli_func()``, ``sli_run()``
or ``sr()`` you will need to now use the direct interface. Please get in touch via the NEST User
mailing list for advice.

If you still have your network models implemented in SLI, it is time now to migrate to PyNEST.



What does this mean for you as a developer?
...........................................

The key changes from a developer perspective are that the entire SLI interpreter code has been
removed, noticeably reducing compile times. We therefore no longer have the ``SLIModule`` concept.
Also, ``Dictionary``, ``Datum``, and ``Token`` are a matter of the past. Instead, we now
have class ``Dictionary`` based directly on ``std::map`` using ``any_type`` to store entries of
arbitrary type. Instead of our own ``lockPTR``, we now use ``std::unique_ptr`` to manage objects with
reference counting.

Catching errors
+++++++++++++++

To test whether certain errors are raised when writing tests, instead of

.. code-block:: python

   with pytest.raises(nest.kernel.NESTErrors.IllegalConnection):

you now have

.. code-block:: python

   with pytest.raises(nest.NESTErrors.IllegalConnection):


Changes to neuron and synapse models (for developers)
+++++++++++++++++++++++++++++++++++++++++++++++++++++

When developing C++ level implementations of neuron or synapse models,
required changes in code will likely be limited to slightly different
notation in the ``set()`` and ``get()`` methods to use the new
``Dictionary`` class. As an example, consider ``aeif_cond_alpha``:

Old NEST
^^^^^^^^

.. code-block:: C++

   void
   nest::aeif_cond_alpha::Parameters_::get( DictionaryDatum& d ) const
   {
     def< double >( d, names::C_m, C_m );
     def< double >( d, names::V_th, V_th );
     // ...
   }

   void
   nest::aeif_cond_alpha::Parameters_::set( const DictionaryDatum& d, Node* node )
   {
     updateValueParam< double >( d, names::V_th, V_th, node );
     updateValueParam< double >( d, names::V_peak, V_peak_, node );
     // ...
   }

New NEST
^^^^^^^^

.. code-block:: C++

   void
   nest::aeif_cond_alpha::Parameters_::get( Dictionary& d ) const
   {
     d[ names::C_m ] = C_m;
     d[ names::V_th ] = V_th;
     // ...
   }

   void
   nest::aeif_cond_alpha::Parameters_::set( const Dictionary& d, Node* node )
   {
     update_value_param( d, names::V_th, V_th, node );
     update_value_param( d, names::V_peak, V_peak_, node );
     // ...
   }

Integer types for Dictionary elements
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Python does not support unsigned integer types. Therefore, integers in ``Dictionary``s must be of type ``long``. Other integer types, e.g., ``int``
or ``size_t`` are no longer supported. Therefore, values need to be cast to ``long`` upon assignment to a dictionary element, e.g.,

.. code-block:: C++

   d[ names::n_receptors ] = static_cast< long >( n_receptors() );

Given that long can hold up to 2^63-1 ≈ 9e18, it is considered safe to do such a typecast without checking the value that is converted.

When receiving integer values from the Python level through a ``Dictionary``, one needs to protect against negative values where only positive
values are acceptable, e.g.:

.. code-block:: C++

   long mbstd = 0;
   if ( dict.update_value( names::max_buffer_size_target_data, mbstd ) )
   {
     if ( mbstd < 0 )
     {
       throw BadProperty( "max_buffer_size_target_data ≥ 0 required." );
     }
     max_buffer_size_target_data_ = mbstd;
   };


NEST requires C++20
-------------------

From NEST 3.10 on, we use some C++20 features in NEST code. Therefore,
NEST needs to be built with a compiler that supports C++20. Most
recent C++ compilers should do so.


New PyNEST examples
-------------------

* :doc:`Examples based on Brette et al 2007 </auto_examples/brette_et_al_2007/index>`

  These examples provide a common framework for running the Brette et al. 2007
  simulator review benchmarks. The benchmarks create sparsely coupled networks
  of excitatory and inhibitory neurons which exhibit self-sustained activity
  after an initial stimulus.

* :doc:`Artificial synchrony example </auto_examples/artificial_synchrony>`

  An example of Artificial synchrony using discrete-time simulations in two implementations (precise and grid-constrained).
