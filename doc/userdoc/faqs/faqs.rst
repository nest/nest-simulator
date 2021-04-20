Frequently Asked Questions
==========================

Installation
------------

1. **If I compile NEST with MPI support, I get errors about
   ``SEEK_SET``, ``SEEK_CUR`` and ``SEEK_END`` being defined** This is a
   known issue in some MPI implementations. A solution is to add
   --with-debug="-DMPICH\_IGNORE\_CXX\_SEEK" to the configure command
   line. More details about this problem can be found
   `here <http://www-unix.mcs.anl.gov/mpi/mpich/faq.htm#cxxseek>`__

2. **Configure warns that Makefile.in seems to ignore the --datarootdir
   setting and the installation fails because of permission errors**
   This problem is due to a change in autoconf 2.60, where the prefix
   directory for the NEST documentation can end up being empty during
   the installation. This leads to wrong installation paths for some
   components of NEST. If you have the GNU autotools installed, you can
   run ``./bootstrap.sh`` in the source directory followed by
   ``./configure``. If you don't have the autotools, appending
   ``--datadir=PREFIX/share/nest`` with the same PREFIX as in the
   ``--prefix`` option should help.

3. **I get 'Error: /ArgumentType in validate' when compiling an
   extension** This is a known bug that has been fixed. Ask your local
   NEST dealer for a new pre-release. You need at least nest-1.9-7320.

4. **I get 'collect2: ld returned 1 exit status, ld: -rpath can only be
   used when targeting Mac OS X 10.5 or later** Please try to set the
   environment variable MACOSX\_DEPLOYMENT\_TARGET to 10.5 (export
   MACOSX\_DEPLOYMENT\_TARGET=10.5)

5. **Ipython crashes with a strange error message as soon as I import
   ``nest``** If ipython crashes on ``import nest`` complaining about a
   ``Non-aligned pointer being freed``, you probably compiled NEST with
   a different version of g++ than Python. Take a look at the
   information ipython prints when it starts up. That should tell you
   which compiler was used. Then re-build NEST with the same compiler
   version.

6. **I get a segmentation fault wher I use SciPy in the same script
   together with PyNEST**. We recently observed that if PyNEST is used
   with some versions of SciPy, a segmentation fault is caused. A
   workaround for the problem is to import SciPy before PyNEST. See
   https://github.com/numpy/numpy/issues/2521 for the official bug
   report in NumPy.


Where does data get stored
~~~~~~~~~~~~~~~~~~~~~~~~~~

By default, the data files produced by NEST are stored in the directory
from where NEST is called. The location can be changed by changing the
property ``data_path`` of the root node using
``nest.SetKernelStatus({"data_path": "/path/to/data"})``. This property
can also be set using the environment variable ``NEST_DATA_PATH``.
Please note that the directory ``/path/to/data`` has to exist. A common
prefix for all data files can be set using the property ``data_prefix``
of the root node by calling
``nest.SetKernelStatus({"data_prefix": "prefix"})`` or setting the
environment variable ``NEST_DATA_PREFIX``.

Neuron models
-------------

1. **I cannot see any of the conductance based models. Where are they?**
   Some neuron model need the GNU Scientific Library (GSL) to work. The
   conductance based models are among those. If your NEST installation
   does not have these models, you probably have no GSL or GSL
   development packages installed. To solve this problem, install the
   GSL and its development headers. Then reconfigure and recompile NEST.

Connections
-----------

1. **How can I create connections to multicompartment neurons?** You
   need to create a synapse type with the proper receptor\_type as in
   this example, which connects all 100 neurons in n to the first neuron
   in n:

   ::

       syns = nest.GetDefaults('iaf_cond_alpha_mc')['receptor_types']
       nest.CopyModel('static_synapse', 'exc_dist_syn', {'receptor_type': syns['distal_exc']})
       n = nest.Create('iaf_cond_alpha_mc', 100)
       nest.Connect(n, n[:1], sync_spec={'model'='exc_dist_syn'})
       nest.Simulate(10)


