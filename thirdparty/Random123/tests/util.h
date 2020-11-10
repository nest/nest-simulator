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
#ifndef UTIL_H__
#define UTIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <math.h>
#include <Random123/features/compilerfeatures.h>

extern const char *progname;
extern int debug;
extern int verbose;

#if defined(_MSC_VER)
#define NOMINMAX /* tells Windows.h to NOT define min() & max() */
#include <Windows.h>
R123_STATIC_INLINE double now(){
    LARGE_INTEGER f; // ticks per second
    LARGE_INTEGER t;
    QueryPerformanceFrequency(&f);
    QueryPerformanceCounter(&t); // ticks since epoch
    return ((double)t.QuadPart)/((double)f.QuadPart);
}
#else // _MSC_VER
#include <sys/time.h>
R123_STATIC_INLINE double now(){
    struct timeval tv; 
    gettimeofday(&tv, 0); 
    return 1.e-6*tv.tv_usec + tv.tv_sec;
}
#endif // _MSC_VER

/* timer returns difference between current time and *d, also updates *d with current time. */
R123_STATIC_INLINE double timer(double *d) {
    double dold = *d;
    *d = now();
    return *d - dold;
}

#define WHITESPACE " \t\f\v\n"
char *nameclean(char *s)
{
    char *cp = s, *cp2, *cpend = s + strlen(s);
    size_t i;

    cp2 = s;
    while ((cp2 = strstr(cp2, "(R)")) != NULL) {
	*cp2++ = ' ';
	*cp2++ = ' ';
	*cp2++ = ' ';
    }
    cp2 = s;
    while ((cp2 = strstr(cp2, "CPU")) != NULL) {
	*cp2++ = ' ';
	*cp2++ = ' ';
	*cp2++ = ' ';
    }
    cp2 = s;
    while ((cp2 = strchr(cp2, '@')) != NULL) {
	*cp2++ = ' ';
    }
    while ((i = strcspn(cp, WHITESPACE)) > 0) {
	cp += i;
	i = strspn(cp, WHITESPACE);
	if (i > 0) {
	    cp2 = cp + i;
	    *cp++ = ' ';
	    if (cp2 > cp) {
		memmove(cp, cp2, cpend - cp);
		cpend -= cp2 - cp;
	    }
	}
    }
    return s;
}

#undef WHITESPACE

/* strdup may or may not be in string.h, depending on the value
   of the pp-symbol _XOPEN_SOURCE and other arcana.  Just
   do it ourselves.
   Mnemonic:  "ntcs" = "nul-terminated character string" */
char *ntcsdup(const char *s){
    char *p = (char *)malloc(strlen(s)+1);
    strcpy(p, s);
    return p;
}

/* MSVC doesn't know about strtoull.  Strictly speaking, strtoull
   isn't standardized in C++98, either, but that seems not to be a
   problem so we blissfully ignore it and use strtoull (or its MSVC
   equivalent, _strtoui64) in both C and C++.  If strtoull in C++
   becomes a problem, we can adopt the prtu strategy (see below) and
   write C++ versions of strtouNN, that use an istringstream
   instead. */
#ifdef _MSC_FULL_VER
#define strtoull _strtoui64
#endif
uint32_t strtou32(const char *p, char **endp, int base){
    uint32_t ret;
    errno = 0;
    ret = strtoul(p, endp, base);
    assert(errno==0);
    return ret;
}
uint64_t strtou64(const char *p, char **endp, int base){
    uint64_t ret;
    errno = 0;
    ret = strtoull(p, endp, base);
    assert(errno==0);
    return ret;
}

#if defined(__cplusplus)
/* Strict C++98 doesn't grok %llx or unsigned long long, and with
   aggressive error-checking, e.g., g++ -pedantic -Wall, will refuse
   to compile code like:

     fprintf(stderr, "%llx", (R123_ULONG_LONG)v);

   On the other hand, when compiling to a 32-bit target, the only
   64-bit type is long long, so we're out of luck if we can't use llx.
   A portable, almost-standard way to do I/O on uint64_t values in C++
   is to use bona fide C++ I/O streams.  We are still playing
   fast-and-loose with standards because C++98 doesn't have <stdint.h>
   and hence doesn't even guarantee that there's a uint64_t, much less
   that the insertion operator<<(ostream&) works correctly with
   whatever we've typedef'ed to uint64_t in
   <features/compilerfeatures.h>.  Hope for the best... */
#include <iostream>
#include <limits>
template <typename T>
void prtu(T val){
    using namespace std;
    cerr.width(std::numeric_limits<T>::digits/4);
    char prevfill = cerr.fill('0');
    ios_base::fmtflags prevflags = cerr.setf(ios_base::hex, ios_base::basefield);
    cerr << val;
    cerr.flags(prevflags);
    cerr.fill(prevfill);
    assert(!cerr.bad());
}
void prtu32(uint32_t v){ prtu(v); }
void prtu64(uint64_t v){ prtu(v); }

#else /* __cplusplus */
/* C should be easy.  inttypes.h was standardized in 1999.  But Microsoft
   refuses to recognize the 12-year old standard, so: */
#if defined(_MSC_FULL_VER)
#define PRIx32 "x"
#define PRIx64 "I64x"
#else /* _MSC_FULL_VER */
#include <inttypes.h>
#endif /* _MSVC_FULL_VER */
void prtu32(uint32_t v){ fprintf(stderr, "%08" PRIx32, v); }
void prtu64(uint64_t v){ fprintf(stderr, "%016" PRIx64, v); }

#endif /* __cplusplus */

/*
 * Convert a hexfloat string of the form "0xA.BpN" to a double,
 * where A and B are hex integers and N is a decimal integer
 * exponent
 */
double
hextod(const char *cp)
{
    uint64_t whole = 0, frac = 0;
    int exponent = 0, len = 0;
    double d;
    char *s;
    if (cp[0] == '0' && (cp[1] == 'x'||cp[1] == 'X'))
	cp += 2;
    whole = strtou64(cp, &s, 16);
    if (s[0] == '.') {
	char *cp = ++s;
	frac = strtou64(s, &s, 16);
	len = s - cp;
    }
    if (s[0] == 'p') {
	s++;
	exponent = atoi(s);
    }
    frac += whole<<(4*len);
    exponent -= 4*len;
    d = frac;
    d = ldexp(d, exponent);
    return d;
}

#define CHECKNOTEQUAL(x, y)  do { if ((x) != (y)) ; else { \
    fprintf(stderr, "%s: %s line %d error %s == %s (%s)\n", progname, __FILE__, __LINE__, #x, #y, strerror(errno)); \
    exit(1); \
} } while (0)
#define CHECKEQUAL(x, y)  do { if ((x) == (y)) ; else { \
    fprintf(stderr, "%s: %s line %d error %s != %s (%s)\n", progname, __FILE__, __LINE__, #x, #y, strerror(errno)); \
    exit(1); \
} } while (0)
#define CHECKZERO(x)  CHECKEQUAL((x), 0)
#define CHECKNOTZERO(x)  CHECKNOTEQUAL((x), 0)

#define dprintf(x) do { if (debug < 1) ; else { printf x; fflush(stdout); } } while (0)

#define ALLZEROS(x, K, N) \
do { \
    int allzeros = 1; \
    unsigned xi, xj; \
    for (xi = 0; xi < (unsigned)(K); xi++)      \
	for (xj = 0; xj < (unsigned)(N); xj++)          \
	    allzeros = allzeros & ((x)[xi].v[xj] == 0); \
    if (allzeros) fprintf(stderr, "%s: Unexpected, all %lu elements of %ux%u had all zeros!\n", progname, (unsigned long)K, (unsigned)N, 8/*CHAR_BITS*/*(unsigned)sizeof(x[0].v[0])); \
} while(0)

/* Read in N words of width W into ARR */
#define SCANFARRAY(ARR, NAME, N, W) \
do { \
    int xi, xj; \
    unsigned long long xv; \
    for (xi = 0; xi < (N); xi++) { \
        /* Avoid any cleverness with SCNx##W because Microsoft (as of Visual Studio 10.x) silently trashes the stack by pretending that %hhx is %x). */ \
	const char *xfmt = " %llx%n"; \
	ret = sscanf(cp, xfmt, &xv, &xj); \
	ARR.v[xi] = (uint##W##_t)xv; \
	if (debug > 1) printf("line %d: xfmt for W=%d is \"%s\", got ret=%d xj=%d, %s[%d]=%llx cp=%s", linenum, W, xfmt, ret, xj, #ARR, xi, (unsigned long long) ARR.v[xi], cp); \
	if (ret < 1) { \
	    fprintf(stderr, "%s: ran out of words reading %s on line %d: " #NAME #N "x" #W " %2d %s", \
		    progname, #ARR, linenum, rounds, line); \
	    errs++; \
	    return; \
	} \
	cp += xj; \
    } \
} while(0)

#define PRINTARRAY(ARR, fp) \
do { \
    char ofmt[64]; \
    size_t xj; \
    /* use %lu and the cast (instead of z) for portability to Microsoft, sizeof(v[0]) should fit easily in an unsigned long.  Avoid inttypes for the same reason. */ \
    sprintf(ofmt, " %%0%lullx", (unsigned long)sizeof(ARR.v[0])*2UL); \
    for (xj = 0; xj < sizeof(ARR.v)/sizeof(ARR.v[0]); xj++) { \
	fprintf(fp, ofmt, (unsigned long long) ARR.v[xj]); \
    } \
} while(0)

#define PRINTLINE(NAME, N, W, R, ictr, ukey, octr, fp) \
do { \
    fprintf(fp, "%s %d ", #NAME #N "x" #W, R); \
    PRINTARRAY(ictr, fp); \
    putc(' ', fp); \
    PRINTARRAY(ukey, fp); \
    putc(' ', fp); \
    PRINTARRAY(octr, fp); \
    putc('\n', fp); \
    fflush(fp); \
} while(0)

#endif /* UTIL_H__ */
