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
/* Known Answer Test */

/* We use the same source files to implement the Known Answer Test
   (KAT) in C, C++, OpenCL and CUDA.  Supporting all four environments
   with a single source file, and getting it all to work with 'make'
   is a bit involved:

   There are five "top-level" files:
     kat_c.c
     kat_cpp.cpp
     kat_cuda.c
     kat_opencl.c
     kat_metal.m

   These correspond to make targets: kat_c, kat_cpp, kat_cuda,
   kat_opencl and kat_metal.

   Those files are relatively simple.  First, they #include this file,
   which contains all the machinery for reading test vectors,
   complaining about errors, etc..  Then they implement the function
   host_execute_tests() in the appropriate environment.  host_execute_tests
   looks very different in C/C++/CUDA/OpenCL/Metal.

   host_execute_tests contrives to call/launch "dev_execute_tests"
   on the device.  Except for a few environment-specific keywords,
   (e.g., __global, __kernel), which are #defined in kat_XXX.c,
   dev_execute_tests is obtained by including a common source file:
      #include <kat_dev_execute.h>

   One final complication:  in order to fully "bake" the source code
   into the binary at compile-time, dev_execute_tests for opencl is implemented in
   kat_opencl_kernel.ocl, which is processed by gencl.sh into
   kat_opencl_kernel.i, which is thein #include-ed by kat_opencl.c.
   
*/
#include "util.h"
#include "kat.h"

#define LINESIZE 1024

int have_aesni = 0;
int verbose = 0;
int debug = 0;
unsigned nfailed = 0;
const char *progname;

extern void host_execute_tests(kat_instance *tests, unsigned ntests);
                
/* A little hack to keep track of the test vectors that we don't know how to deal with: */
int nunknowns = 0;
#define MAXUNKNOWNS 20
const char *unknown_names[MAXUNKNOWNS];
int unknown_counts[MAXUNKNOWNS];

void register_unknown(const char *name){
    int i;
    for(i=0; i<nunknowns; ++i){
        if( strcmp(name, unknown_names[i]) == 0 ){
            unknown_counts[i]++;
            return;
        }
    }
    if( i >= MAXUNKNOWNS ){
        fprintf(stderr, "Too many unknown rng types.  Bye.\n");
        exit(1);
    }
    nunknowns++;
    unknown_names[i] = ntcsdup(name);
    unknown_counts[i] = 1;
}

void report_unknowns(){
    int i;
    for(i=0; i<nunknowns; ++i){
        printf("%d test vectors of type %s skipped\n", unknown_counts[i], unknown_names[i]);
    }
}

/* read_<GEN>NxW */
#define RNGNxW_TPL(base, N, W) \
int read_##base##N##x##W(const char *line, kat_instance* tinst){        \
    size_t i;                                                           \
    int nchar;                                                          \
    const char *p = line;                                               \
    char *newp;                                                         \
    size_t nkey = sizeof(tinst->u.base##N##x##W##_data.ukey.v)/sizeof(tinst->u.base##N##x##W##_data.ukey.v[0]); \
    tinst->method = base##N##x##W##_e;                                  \
    sscanf(p,  "%u%n", &tinst->nrounds, &nchar);                        \
    p += nchar;                                                         \
    for(i=0;  i<N; ++i){                                                \
        tinst->u.base##N##x##W##_data.ctr.v[i] = strtou##W(p, &newp, 16); \
        p = newp;                                                       \
    }                                                                   \
    for(i=0; i<nkey; ++i){                                              \
        tinst->u.base##N##x##W##_data.ukey.v[i] = strtou##W(p, &newp, 16); \
        p = newp;                                                       \
    }                                                                   \
    for(i=0;  i<N; ++i){                                                \
        tinst->u.base##N##x##W##_data.expected.v[i] = strtou##W(p, &newp, 16); \
        p = newp;                                                       \
    }                                                                   \
    /* set the computed to 0xca.  If the test fails to set computed, we'll see cacacaca in the FAILURE notices */ \
    memset(tinst->u.base##N##x##W##_data.computed.v, 0xca, sizeof(tinst->u.base##N##x##W##_data.computed.v));                  \
    return 1;                                                           \
}
#include "rngNxW.h"
#undef RNGNxW_TPL

/* readtest:  dispatch to one of the read_<GEN>NxW functions */
static int readtest(const char *line, kat_instance* tinst){
    int nchar;
    char name[LINESIZE];
    if( line[0] == '#') return 0;                                       
    sscanf(line, "%s%n", name, &nchar);
    if(!have_aesni){
        /* skip any tests that require AESNI */ 
        if(strncmp(name, "aes", 3)==0 ||
           strncmp(name, "ars", 3)==0){
            register_unknown(name);
            return 0;
        }
    }
#define RNGNxW_TPL(base, N, W) if(strcmp(name, #base #N "x" #W) == 0) return read_##base##N##x##W(line+nchar, tinst);
#include "rngNxW.h"
#undef RNGNxW_TPL

    register_unknown(name);
    return 0;
}

#define RNGNxW_TPL(base, N, W) \
void report_##base##N##x##W##error(const kat_instance *ti){ \
 size_t i;                                                     \
 size_t nkey = sizeof(ti->u.base##N##x##W##_data.ukey.v)/sizeof(ti->u.base##N##x##W##_data.ukey.v[0]); \
 fprintf(stderr, "FAIL:  expected: ");                                \
 fprintf(stderr, #base #N "x" #W " %d", ti->nrounds);                   \
 for(i=0; i<N; ++i){                                                    \
     fprintf(stderr, " "); prtu##W(ti->u.base##N##x##W##_data.ctr.v[i]); \
 }                                                                      \
 for(i=0; i<nkey; ++i){                                                 \
     fprintf(stderr, " "); prtu##W(ti->u.base##N##x##W##_data.ukey.v[i]); \
 }                                                                      \
 for(i=0; i<N; ++i){                                                    \
     fprintf(stderr, " "); prtu##W(ti->u.base##N##x##W##_data.expected.v[i]); \
 }                                                                      \
 fprintf(stderr, "\n");                                                 \
                                                                        \
 fprintf(stderr, "FAIL:  computed: ");                                \
 fprintf(stderr, #base #N "x" #W " %d", ti->nrounds);                   \
 for(i=0; i<N; ++i){                                                    \
     fprintf(stderr, " "); prtu##W(ti->u.base##N##x##W##_data.ctr.v[i]); \
 }                                                                      \
 for(i=0; i<nkey; ++i){                                                 \
     fprintf(stderr, " "); prtu##W(ti->u.base##N##x##W##_data.ukey.v[i]); \
 }                                                                      \
 for(i=0; i<N; ++i){                                                    \
     fprintf(stderr, " "); prtu##W(ti->u.base##N##x##W##_data.computed.v[i]); \
 }                                                                      \
 fprintf(stderr, "\n");                                                 \
 nfailed++;                                                             \
}
#include "rngNxW.h"
#undef RNGNxW_TPL

// dispatch to one of the report_<GEN>NxW() functions
void analyze_tests(const kat_instance *tests, unsigned ntests){
    unsigned i;
    char zeros[512] = {0};
    for(i=0; i<ntests; ++i){
        const kat_instance *ti = &tests[i];
        switch(tests[i].method){
#define RNGNxW_TPL(base, N, W) case base##N##x##W##_e: \
            if (memcmp(zeros, ti->u.base##N##x##W##_data.expected.v, N*W/8)==0){ \
                fprintf(stderr, "kat expected all zeros?   Something is wrong with the test harness!\n"); \
                nfailed++; \
            } \
            if (memcmp(ti->u.base##N##x##W##_data.computed.v, ti->u.base##N##x##W##_data.expected.v, N*W/8)) \
		report_##base##N##x##W##error(ti); \
	    break;
#include "rngNxW.h"
#undef RNGNxW_TPL
        case last: ;
        }
    }
}

#define NTESTS 1000

int main(int argc, char **argv){
    kat_instance *tests;
    unsigned t, ntests = NTESTS;
    char linebuf[LINESIZE];
    FILE *inpfile;
    const char *p;
    const char *inname;
    char filename[LINESIZE];
    
    progname = argv[0];

    /* If there's an argument, open that file.
       else if getenv("srcdir") is non-empty open getenv("srcdir")/kat_vectors
       else open "./kat_vectors" */
    if( argc > 1 )
        inname = argv[1];
    else{
        const char *e = getenv("srcdir");
        if(!e)
            e = ".";
        sprintf(filename, "%s/kat_vectors", e);
        inname = filename;
    }

    if (strcmp(inname, "-") == 0) {
	inpfile = stdin;
    } else {
	inpfile = fopen(inname, "r");
	if (inpfile == NULL) {
	    fprintf(stderr, "%s: error opening input file %s for reading: %s\n",
		    progname, inname, strerror(errno));
	    exit(1);
	}
    }
    if ((p = getenv("KATC_VERBOSE")) != NULL) {
	verbose = atoi(p);
    }
    if ((p = getenv("KATC_DEBUG")) != NULL) {
	debug = atoi(p);
    }

#if R123_USE_AES_NI
    have_aesni = haveAESNI();
#else
    have_aesni = 0;
#endif

    tests = (kat_instance *) malloc(sizeof(tests[0])*ntests);
    if (tests == NULL) {
	fprintf(stderr, "Could not allocate %lu bytes for tests\n",
		(unsigned long) ntests);
	exit(1);
    }
    t = 0;
    while (fgets(linebuf, sizeof linebuf, inpfile) != NULL) {
        if( t==ntests ){
	    ntests *= 2;
	    tests = (kat_instance *)realloc(tests, sizeof(tests[0])*ntests);
	    if (tests == NULL) {
		fprintf(stderr, "Could not grow tests to %lu bytes\n",
			(unsigned long) ntests);
		exit(1);
	    }
        }
        if( readtest(linebuf, &tests[t]) )
            ++t;
    }
    if(t==ntests){
	fprintf(stderr, "No more space for tests?  Recompile with a larger NTESTS\n");
	exit(1);
    }
    tests[t].method = last; // N.B  *not* t++ - the 'ntests' value passed to host_execute_tests does not count the 'last' one.

    report_unknowns();
    printf("Perform %lu tests.\n", (unsigned long)t);
    host_execute_tests(tests, t);

    analyze_tests(tests, t);
    free(tests);
    if(nfailed != 0){
        printf("FAILED %u out of %u\n", nfailed, t);
        return 1;
    }else{
        printf("PASSED %u known answer tests\n", t);
        return 0;
    }
}
