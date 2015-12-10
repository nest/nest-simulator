include(BlueGeneQ_Base)
# add path to ppc gcc
set(ENV{PATH} "/bgsys/drivers/ppcfloor/gnu-linux/bin:$ENV{PATH}")

# set the compiler
set(CMAKE_C_COMPILER powerpc64-bgq-linux-gcc)
set(CMAKE_CXX_COMPILER powerpc64-bgq-linux-g++)

