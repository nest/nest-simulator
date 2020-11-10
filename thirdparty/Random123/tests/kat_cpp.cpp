/*
Copyright 2010-2011, D. E. Shaw Research.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

* Redistributions of source code must retain the above copyright
  notice, this list of conditions, and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions, and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

* Neither the name of D. E. Shaw Research nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "kat_main.h" 

// With C++, it's a little trickier to create the mapping from
// method-name/round-count to functions
// because the round-counts are template arguments that have to be
// specified at compile-time.  Thus, we can't just do #define RNGNxW_TPL
// and #include "rngNxW.h".  We have to build a static map from:
//  pair<generator, rounds> to functions that apply the right generator
// with the right number of rounds.

#ifdef _MSC_FULL_VER
// Engines have multiple copy constructors, quite legal C++, disable MSVC complaint
#pragma warning (disable : 4521)
#endif

#include <map>
#include <cstring>
#include <utility>
#include <stdexcept>
#include "Random123/MicroURNG.hpp"
#include "Random123/conventional/Engine.hpp"

using namespace std;

typedef map<pair<method_e, unsigned>, void (*)(kat_instance *)> genmap_t;
genmap_t genmap;

void dev_execute_tests(kat_instance *tests, unsigned ntests){
    unsigned i;
    for(i=0; i<ntests; ++i){
        kat_instance *ti = &tests[i];
        genmap_t::iterator p = genmap.find(make_pair(ti->method, ti->nrounds));
        if(p == genmap.end())
            throw std::runtime_error("pair<generator, nrounds> not in map.  You probably need to add more genmap entries in kat_cpp.cpp");

        p->second(ti);
        // TODO: check that the corresponding Engine and MicroURNG
        //  return the same values.  Note that we have ut_Engine and
        //  ut_MicroURNG, which check basic functionality, but they
        //  don't have the breadth of the kat_vectors.
    }
}

static int murng_reported;
static int engine_reported;

template <typename GEN>
void do_test(kat_instance* ti){
    GEN g;
    struct gdata{
        typename GEN::ctr_type ctr;
        typename GEN::ukey_type ukey;
        typename GEN::ctr_type expected;
        typename GEN::ctr_type computed;
    };
    gdata data;
    // use memcpy.  A reinterpret_cast would violate strict aliasing.
    memcpy(&data, &ti->u, sizeof(data));
    data.computed = g(data.ctr, data.ukey);

    // Before we return, let's make sure that MicroURNG<GEN,1> and
    // Engine<GEN> work as expeccted.  This doesn't really "fit" the
    // execution model of kat.c, which just expects us to fill in
    // ti->u.computed, so we report the error by failing to write back
    // the computed data item in the (hopefully unlikely) event that
    // things don't match up as expected.
    int errs = 0;

    // MicroURNG:  throws if the top 32 bits of the high word of ctr
    // are non-zero.
    typedef typename GEN::ctr_type::value_type value_type;

    value_type hibits = data.ctr[data.ctr.size()-1]>>( std::numeric_limits<value_type>::digits - 32 );
    try{
        r123::MicroURNG<GEN> urng(data.ctr, data.ukey);
        if(hibits)
            errs++; // Should have thrown.
        for (size_t i = 0; i < data.expected.size(); i++) {
	    size_t j = data.expected.size() - i - 1;
	    if (data.expected[j] != urng()) {
                errs++;
	    }
	}
    }catch(std::runtime_error& /*ignored*/){
        // A runtime_error is expected from the constructor
        // when hibit is set.
        if(!hibits)
            errs++;
    }
    if(errs && (murng_reported++ == 0))
        cerr << "Error in MicroURNG<GEN>, will appear as \"computed\" value of zero in error summary\n";

    // Engine
    // N.B.  exercising discard() arguably belongs in ut_Engine.cpp
    typedef r123::Engine<GEN> Etype;
    typedef typename GEN::ctr_type::value_type value_type;
    Etype e(data.ukey);
    typename GEN::ctr_type c = data.ctr;
    value_type c0;
    if( c[0] > 0 ){
        c0 = c[0]-1;
    }else{
        // N.B.  Assume that if c[0] is 0, then so are all the
        // others.  Arrange to "roll over" to {0,..,0} on the first
        // counter-increment.  Alternatively, we could just
        // skip the test for this case...
        c.fill(std::numeric_limits<value_type>::max());
        c0 = c[0];
    }
    c[0] /= 3;
    e.setcounter(c, 0);
    if( c0 > c[0] ){
        // skip one value by calling  e()
        (void)e();
        if (c0 > c[0]+1) {
	    // skip many values by calling discard()
	    R123_ULONG_LONG ndiscard = (c0 - c[0] - 1);
            // Take care not to overflow the long long
            if( ndiscard >= std::numeric_limits<R123_ULONG_LONG>::max() / c.size() ){
                for(size_t j=0; j<c.size(); ++j){
                    e.discard(ndiscard);
                }
            }else{
                ndiscard *= c.size();
                e.discard(ndiscard);
            }
	}
	// skip a few more by calling e().
	for (size_t i = 1; i < c.size(); i++) {
	    (void) e();
	}
        // we should be back to where we started...
    }
    for (size_t i = 0; i < data.expected.size(); i++) {
	value_type val = e();
	size_t j = data.expected.size() - i - 1;
	if (data.expected[j] != val) {
            cerr << hex;
            cerr << "Engine check, j=" << j << " expected: " << data.expected[j] << " val: " << val << "\n";
	    errs++;
            if(engine_reported++ == 0)
                cerr << "Error in Engine<GEN, 1>, will appear as \"computed\" value of zero in error summary\n";
	}
    }

    // Signal an error to the caller by *not* copying back
    // the computed data object into the ti
    if(errs == 0)
        memcpy(&ti->u, &data, sizeof(data));
}

void host_execute_tests(kat_instance *tests, unsigned ntests){
    // In C++1x, this could be staticly declared with an initializer list.
    genmap[make_pair(threefry2x32_e, 13u)] = do_test<r123::Threefry2x32_R<13> >;
    genmap[make_pair(threefry2x32_e, 20u)] = do_test<r123::Threefry2x32_R<20> >;
    genmap[make_pair(threefry2x32_e, 32u)] = do_test<r123::Threefry2x32_R<32> >;
#if R123_USE_64BIT
    genmap[make_pair(threefry2x64_e, 13u)] = do_test<r123::Threefry2x64_R<13> >;
    genmap[make_pair(threefry2x64_e, 20u)] = do_test<r123::Threefry2x64_R<20> >;
    genmap[make_pair(threefry2x64_e, 32u)] = do_test<r123::Threefry2x64_R<32> >;
#endif

    genmap[make_pair(threefry4x32_e, 13u)] = do_test<r123::Threefry4x32_R<13> >;
    genmap[make_pair(threefry4x32_e, 20u)] = do_test<r123::Threefry4x32_R<20> >;
    genmap[make_pair(threefry4x32_e, 72u)] = do_test<r123::Threefry4x32_R<72> >;
#if R123_USE_64BIT
    genmap[make_pair(threefry4x64_e, 13u)] = do_test<r123::Threefry4x64_R<13> >;
    genmap[make_pair(threefry4x64_e, 20u)] = do_test<r123::Threefry4x64_R<20> >;
    genmap[make_pair(threefry4x64_e, 72u)] = do_test<r123::Threefry4x64_R<72> >;
#endif

    genmap[make_pair(philox2x32_e, 7u)] = do_test<r123::Philox2x32_R<7> >;
    genmap[make_pair(philox2x32_e, 10u)] = do_test<r123::Philox2x32_R<10> >;
    genmap[make_pair(philox4x32_e, 7u)] = do_test<r123::Philox4x32_R<7> >;
    genmap[make_pair(philox4x32_e, 10u)] = do_test<r123::Philox4x32_R<10> >;

#if R123_USE_PHILOX_64BIT
    genmap[make_pair(philox2x64_e, 7u)] = do_test<r123::Philox2x64_R<7> >;
    genmap[make_pair(philox2x64_e, 10u)] = do_test<r123::Philox2x64_R<10> >;
    genmap[make_pair(philox4x64_e, 7u)] = do_test<r123::Philox4x64_R<7> >;
    genmap[make_pair(philox4x64_e, 10u)] = do_test<r123::Philox4x64_R<10> >;
#endif

#if R123_USE_AES_NI
    genmap[make_pair(aesni4x32_e, 10u)] = do_test<r123::AESNI4x32 >;
    genmap[make_pair(ars4x32_e, 7u)] = do_test<r123::ARS4x32_R<7> >;
    genmap[make_pair(ars4x32_e, 10u)] = do_test<r123::ARS4x32_R<10> >;
#endif

    dev_execute_tests(tests, ntests);
}
