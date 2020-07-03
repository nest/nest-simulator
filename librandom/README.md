# `librandom` folder: `librandom 2.0`

Auxilliary library for random number generators.

<!-- NOTE: This file is somewhat outdated; HEP 2010-05-28. -->

## Key files

`randomgen.h` : all that's needed to draw random numbers
`gslrandomgen.h`  : required for creating rngs
`randomdev.h`     : non-uniform random deviates

## Usage

Files that only use random generators need only include randomgen.h. Files that implement RNG allocation and thus need access to the GslRandomGen class directly, need gslrandomgen.h as well.  This will work even if GSL is not installed.

    Class RandomGen      : abstract class allowing seeding and drawing
          GslRandomGen   : derived class implementing GSL-style RNGs

          RandomDev      : abstract class allowing drawing only
          GammaRandomDev : Gamma random dev
          NormalRandomDev: Normal random dev
          PoissonRanDev  : Poisson random dev
          ExpRanDev      : Exponential random dev

* __Note__: parameters influencing that shape of distributions, such as gamma order, can be set via `set_*` methods.  Pure scaling paramaters must be applied to the deviate by the calling routine.


## Accessing RNGs

All GSL and GSL-style RNGs are included in a list:

    librandom::GslRandomGen::RngList

Before this list is accessed for the first time, it has to initialized
by a call to:

    librandom::GslRandomGen::build_rng_list()

If this function is called more than once during a program run, only the first call will build the list, all further calls do nothing.  The list contains `gsl_rng_type*`, which can be used to access the pointers. The use is illustrated by `randomtest.cpp` (see also `sli/rangen.cc`):

    #include "gslrandomgen.h"
    librandom::GslRngListType::iterator t;
    for (t  = librandom::GslRandomGen::RngList.begin();
         t != librandom::GslRandomGen::RngList.end();
         ++t )
        {
          std::cout << std::setw(17) << (*t)->name << " : ";
          rng = new librandom::GslRandomGen(*t, seed);
          rungen(rng, Ngen);   // run generator test
          delete(rng);
        }

Note that functions using librandom do not have to worry about the presence or absence of the GSL: all GSL dependencies are completely contained within librandom.  The only difference will be that far fewer RNG are available when the GSL is missing or too old.

## Random Deviates

In non-threaded environments (SLI interpreter, kernel), random deviate sources should be created with an `RngPtr` to a random generator as argument, e.g., `new NormalRandomDev(myrng)`. A pointer to the RNG is then stored in the `RandomDev` object, and the RNG is used for all subsequent RNG generation.

In threaded environments (nest), each call to a random deviate source may come from a different thread, and consequently, a different `RngClient` has to be used on each call. Therefore, create the `RandomDev` object WITHOUT an rng pointer and rather pass the pointer to the threads `RngClient` object each time you draw a random number. See `nest/noise_generator.cpp` for an example.


## Verification

Mersenne Twister only. Generated four streams of 20MB ulrands each (using `gsl_rng_get`) in four-thread nest-kernel.  Ran DIEHARD on all streams.  None showed problems.  Computed covariance of streams (up to lag 1000) and found no correlations.

Exponential, gamma and normal deviates with MT backend tested: 1000x1000 random number Kolmogorov-Smirnov test with meta-test on resulting 1000 p-values.  The Poisson deviate was similarly tested with 1000x1000 Chi2 tests.  All tests were fine.


## Integration with GSL

GSL 1.2 or newer should be installed to get a choice of random generators.  If GSL is not installed, only the MersenneTwister RNG will be available from the GSL.  The MT is the default generator in any case.

To allow for smooth integration in presence/absence of the GSL, core GSL files have been copied to `librandom`.  These files are prefixed with `gsl_*`.  They have been slightly adapted from their GSL originals and are "emptied" by `#ifndef`'s if GSL is available.  GSL header files are placed in `synod2/include/gsl_gsl_*.h`, since they might be used by other parts of NEST that make use of the GSL.  The GSL-replacement code is compiled only if no acceptable GSL is available.

Random deviates cannot be implemented by linking against the GSL, since the pertaining GSL functions require a `gsl_rng*` as input.  This cannot be reconciled with the necessity of RNG streaming in multi-threaded NEST.  We therefore have to adapt GSL code to take RandomGen objects instead of `gsl_rng*`.  `gsl_gsl_errno.h` can be used to map all `GSL_ERROR` and `_WARNING` calls to failing asserts as a quick hack when making this adaptation.

## Adding Non-GSL Random Generators

Further RNGs can be added, but should have the same interface as GSL RNGs.  Such RNGs are called GSL-style RNGs.  An example is `gsl_knuthlfg`.  All GSL-style RNGs must provide an

    extern const gsl_rng_type *

pointer to the generator in their `*.h` file, and this pointer must be entered in `GslRandomGen::build_rng_list()` to make the RNG available.


## Integration with SLI

The SLI module RandomNumbers, defined in `rangen.*`, creates a dictionary `rngdict`, which contains all defined GSL `rng` types. Elements of this `dict` have type `RngTypeDatum`, with name `"rngtypetype"`, corresponding to or `_gt_` for tries.  The actual RNG objects created from `RngTypeDatum` and a seed are of type `RngDatum`, name `"rngtyp"`, or `_g_` for tries; they are `shared_ptr` objects.
