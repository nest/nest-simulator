Simpler handling of random number generators
--------------------------------------------

All random number generators managed by the NEST kernel are now seeded by
providing a single seed :math:`s`  with :math:`1\leq s \leq 2^{31}-1`. The
kernel automatically seeds the random number streams on the various parallel
threads and processes:

  +---------------------------------------------+---------------------------------------+
  | NEST 2.x                                    | NEST 3.0                              |
  +=============================================+=======================================+
  |                                             |                                       |
  | ::                                          | ::                                    |
  |                                             |                                       |
  |     msd = n_threads * seed + 1              |     nest.SetKernelStatus({            |
  |     nest.SetKernelStatus({                  |                    'rng_seed': seed}) |
  |        'grng_seed': msd,                    |                                       |
  |        'rng_seeds': range(msd+1,            |                                       |
  |                           msd+1+n_threads)  |                                       |
  |        })                                   |                                       |
  |                                             |                                       |
  +---------------------------------------------+---------------------------------------+

Changing the type of random number generator to use is easy now:

  +---------------------------------------------+---------------------------------------+
  | NEST 2.x                                    | NEST 3.0                              |
  +=============================================+=======================================+
  |                                             |                                       |
  | ::                                          | ::                                    |
  |                                             |                                       |
  |     # too difficult to show here            |     nest.SetKernelStatus({            |
  |                                             |               'rng_type': 'mt19937'}) |
  |                                             |                                       |
  +---------------------------------------------+---------------------------------------+

Which random number generator types are available can be checked by:

::

    nest.GetKernelStatus('rng_types')

Details about the new random number generators can be found in the guide on :doc:`random number generators <../random_numbers>`.

Counter-based random number generators
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In addition to the conventional random number generator types, there is added support for
"counter-based" random number generators (CBRNGs) with the Random123 library. Where
conventional RNGs use N iterations of a stateful transformation to find the Nth random
number, the Nth random number of a CBRNG can be obtained by applying a stateless mixing
function to N.

The Random123 library is included in NEST, and random number generator types like Philox and
Threefry are available out of the box.

.. note::

   On some systems or with some compilers, the CBRNGs may not give reliable results. In these
   cases, they are automatically disabled during compilation and will not be available.
