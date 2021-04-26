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
#ifndef __Engine_dot_hpp_
#define __Engine_dot_hpp_

#include "../features/compilerfeatures.h"
#include "../array.h"
#include <limits>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <vector>
#if R123_USE_CXX11_TYPE_TRAITS
#include <type_traits>
#endif

namespace r123{
/**
  If G satisfies the requirements of a CBRNG, and has a ctr_type whose
  value_type is an unsigned integral type, then Engine<G> satisfies
  the requirements of a C++11 "Uniform Random Number Engine" and can
  be used in any context where such an object is expected.

  Note that wrapping a counter based RNG with a traditional API in
  this way obscures much of the power of counter based PRNGs.
  Nevertheless, it may be of value in applications that are already
  coded to work with the C++11 random number engines.

  The MicroURNG template in MicroURNG.hpp
  provides the more limited functionality of a C++11 "Uniform
  Random Number Generator", but leaves the application in control
  of counters and keys and hence may be preferable to the Engine template.
  For example, a MicroURNG allows one to use C++11 "Random Number
  Distributions"  without giving up control over the counters
  and keys.
*/ 

template<typename CBRNG>
struct Engine {
    typedef CBRNG cbrng_type;
    typedef typename CBRNG::ctr_type ctr_type;
    typedef typename CBRNG::key_type key_type;
    typedef typename CBRNG::ukey_type ukey_type;
    typedef typename ctr_type::value_type result_type;

protected:
    cbrng_type b;
    key_type key;
    ctr_type c;
    ctr_type v;

    void fix_invariant(){
        if( v.back() != 0 ) {
            result_type vv = v.back();
            v = b(c, key);
            v.back() = vv;
	}
    }        
public:
    explicit Engine() : b(), c() {
	ukey_type x = {{}};
        v.back() = 0;
        key = x;
    }
    explicit Engine(result_type r) : b(), c() {
        ukey_type x = {{typename ukey_type::value_type(r)}};
        v.back() = 0;
        key = x;
    }
    // 26.5.3 says that the SeedSeq templates shouldn't particpate in
    // overload resolution unless the type qualifies as a SeedSeq.
    // How that is determined is unspecified, except that "as a
    // minimum a type shall not qualify as a SeedSeq if it is
    // implicitly convertible to a result_type."  
    //
    // First, we make sure that even the non-const copy constructor
    // works as expected.  In addition, if we've got C++11
    // type_traits, we use enable_if and is_convertible to implement
    // the convertible-to-result_type restriction.  Otherwise, the
    // template is unconditional and will match in some surpirsing
    // and undesirable situations.
    Engine(Engine& e) : b(e.b), key(e.key), c(e.c){
        v.back() = e.v.back();
        fix_invariant();
    }
    Engine(const Engine& e) : b(e.b), key(e.key), c(e.c){
        v.back() = e.v.back();
        fix_invariant();
    }

    template <typename SeedSeq>
    explicit Engine(SeedSeq &s
#if R123_USE_CXX11_TYPE_TRAITS
                    , typename std::enable_if<!std::is_convertible<SeedSeq, result_type>::value>::type* =0
#endif
                    )
        : b(), c() {
        ukey_type ukey = ukey_type::seed(s);
        key = ukey;
        v.back() = 0;
    }
    void seed(result_type r){
        *this = Engine(r);
    }
    template <typename SeedSeq>
    void seed(SeedSeq &s
#if R123_USE_CXX11_TYPE_TRAITS
                    , typename std::enable_if<!std::is_convertible<SeedSeq, result_type>::value>::type* =0
#endif
              ){ 
        *this = Engine(s);
    }
    void seed(){
        *this = Engine();
    }
    friend bool operator==(const Engine& lhs, const Engine& rhs){
        return lhs.c==rhs.c && lhs.v.back() == rhs.v.back() && lhs.key == rhs.key;
    }
    friend bool operator!=(const Engine& lhs, const Engine& rhs){
        return lhs.c!=rhs.c || lhs.v.back()!=rhs.v.back() || lhs.key!=rhs.key;
    }

    friend std::ostream& operator<<(std::ostream& os, const Engine& be){
        return os << be.c << " " << be.key << " " << be.v.back();
    }

    friend std::istream& operator>>(std::istream& is, Engine& be){
        is >> be.c >> be.key >> be.v.back();
        be.fix_invariant();
        return is;
    }

    // The <random> shipped with MacOS Xcode 4.5.2 imposes a
    // non-standard requirement that URNGs also have static data
    // members: _Min and _Max.  Later versions of libc++ impose the
    // requirement only when constexpr isn't supported.  Although the
    // Xcode 4.5.2 requirement is clearly non-standard, it is unlikely
    // to be fixed and it is very easy work around.  We certainly
    // don't want to go to great lengths to accommodate every buggy
    // library we come across, but in this particular case, the effort
    // is low and the benefit is high, so it's worth doing.  Thanks to
    // Yan Zhou for pointing this out to us.  See similar code in
    // ../MicroURNG.hpp
    const static result_type _Min = 0;
    const static result_type _Max = ~((result_type)0);

    static R123_CONSTEXPR result_type min R123_NO_MACRO_SUBST () { return _Min; }
    static R123_CONSTEXPR result_type max R123_NO_MACRO_SUBST () { return _Max; }

    result_type operator()(){
        if( c.size() == 1 )     // short-circuit the scalar case.  Compilers aren't mind-readers.
            return b(c.incr(), key)[0];
        result_type& elem = v.back();
        if( elem == 0 ){
            v = b(c.incr(), key);
            result_type ret = v.back();
            elem = c.size()-1;
            return ret;
        }
        return v[--elem];
    }

    void discard(R123_ULONG_LONG skip){
        // don't forget:  elem counts down
        size_t nelem = c.size();
	size_t sub = skip % nelem;
        result_type& elem  = v.back();
        skip /= nelem;
	if (elem < sub) {
	    elem += nelem;
	    skip++;
	}
	elem -= sub;
        c.incr(skip);
        fix_invariant();
    }
         
    //--------------------------
    // Some bonus methods, not required for a Random Number
    // Engine

    // Constructors and seed() method for ukey_type seem useful
    // We need const and non-const to supersede the SeedSeq template.
    explicit Engine(const ukey_type &uk) : key(uk), c(){ v.back() = 0; }
    explicit Engine(ukey_type &uk) : key(uk), c(){  v.back() = 0; }
    void seed(const ukey_type& uk){
        *this = Engine(uk);
    }        
    void seed(ukey_type& uk){
        *this = Engine(uk);
    }        

#if R123_USE_CXX11_TYPE_TRAITS
    template <typename DUMMY=void>
    explicit Engine(const key_type& k,
                    typename std::enable_if<!std::is_same<ukey_type, key_type>::value, DUMMY>::type* = 0)
        : key(k), c(){ v.back() = 0; }

    template <typename DUMMY=void>
    void seed(const key_type& k,
              typename std::enable_if<!std::is_same<ukey_type, key_type>::value, DUMMY>::type* = 0){
        *this = Engine(k);
    }
#endif

    // Forward the e(counter) to the CBRNG we are templated
    // on, using the current value of the key.
    ctr_type operator()(const ctr_type& c) const{
        return b(c, key);
    }

    key_type getkey() const{
        return key;
    }

    // N.B.  setkey(k) is different from seed(k) because seed(k) zeros
    // the counter (per the C++11 requirements for an Engine), whereas
    // setkey does not.
    void setkey(const key_type& k){
        key = k;
        fix_invariant();
    }

    // Maybe the caller want's to know the details of
    // the internal state, e.g., so it can call a different
    // bijection with the same counter.
    std::pair<ctr_type, result_type> getcounter() const {
        return std::make_pair(c, v.back());
    }

    // And the inverse.
    void setcounter(const ctr_type& _c, result_type _elem){
        static const size_t nelem = c.size();
        if( _elem >= nelem )
            throw std::range_error("Engine::setcounter called  with elem out of range");
        c = _c;
        v.back() = _elem;
        fix_invariant();
    }

    void setcounter(const std::pair<ctr_type, result_type>& ce){
        setcounter(ce.first, ce.second);
    }
};
} // namespace r123

#endif
