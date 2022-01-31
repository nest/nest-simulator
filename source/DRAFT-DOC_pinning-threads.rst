Optimize NEST performace on high performance computers (HPC)
============================================================


For large simulations, optimizing your system will make running NEST faster and more efficient.

One way to optimize performance is to minimize interference between computations being performed on multicore and
multi processor systems. This is done by selecting the best parameters for the hardware you're working with.


See also Anno et al. [1]_ for detailed study on this topic.



.. image:: _static/img/hpc_ware.png





.. note::

  This is just one configuration for hardware setup. A particular system may use other components, but for configuring HPC systems,
  nodes and cores are the most important terms to know.

In the image above, on the left side we have a representation of a typical hardware setup.

* In a supercomputer or cluster, there are many, many nodes.

* Each node contains CPUs (or processors) and each CPU has its own cores and L1, L2, L3 cache.

* Each node is basically its own computer and comprises other components not shown here.



* The cores are where the computations are performed.

* The cache is the local memory store for that processor (referred to as L1, L2, L3 cache).

* Data and instructions (your code) from your simulation are retrieved from the RAM (not shown) and passed to the cache and allocated to the cores.

* The core executes the instruction and returns output to the cache.

On the right side, we see how the data and instructions are allocated through software.

* A set of data and instructions that belong together is referred to as a task or process. This can be your entire simulation
  script or a subset of it.

* We typically use the standard Message Passing Interface (MPI) to instruct how processes work in parallel.

* The smallest unit of executable program is known as a thread. The thread is a virtual component. A single core can
  have one or two threads.

To efficiently run your large and complex simulation, we need to configure the optimal number of threads and processes for
your simulation and the given hardware of the HPC system you are using.




Example Slurm script
--------------------

`Slurm <https://slurm.schedmd.com/documentation.html>`_ is a job scheduler used on many/most high performance computing systems.

Typically, you specify system parameters for the job you want to run in a job script.

This is an example job script  that shows common settings useful when running NEST on HPC systems. The settings are applicable
to other job schedulers other than Slurm but the syntax will be different.
Always consult the documentation of the system you are running on to find out exactly what you need provide in your script.

You will likely need to alter the job script to optimize the performance on the system your're using.
Finding the optimum parameters for your script may require some trial and error.

.. code-block:: sh

   #!/bin/bash -l
   #SBATCH --job-name="my_nest_simulation"
   #SBATCH --account="my_account"
   #SBATCH --partition=normal
   #SBATCH --time=01:00:00
   #SBATCH --nodes=1
   #SBATCH --ntasks-per-node=1
   #SBATCH --cpus-per-task=64
   #SBATCH --hint=nomultithread

   export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
   export OMP_PROC_BIND=TRUE

   srun python my_nest_simulation.py


----


Let's break this script down line by line.

::

  #!/bin/bash -l

You are submitting a shell script to Slurm. The "shebang" line must be the first line of shell script

|

::

   #SBATCH --job-name="my_nest_simulation"

The name of the job should be unique and descriptive enough to differentiate it from other jobs.

|

::

   #SBATCH --account="my_account"

Name of your account

|

::

   #SBATCH --partition=normal

The partition describes what architecture or hardware specifications(?) your job will use.
For example, some systems have GPU and CPU partitions.
The actual partition name will depend on the HPC system and what it has available.

|

::

   #SBATCH --time=01:00:00

This is also known as wall time, which is the time allocated to your job. Your job will be killed if you do not set
it long enough.

|

::

   #SBATCH --nodes=1

The number of nodes needed for your job. The number of nodes will depend on your memory needs and if you're
trying to increase the speed of the simulation (see benchmarking or further/ advanced configuration).

How much memeory does your simulation need? To get a rough estimate of the memory requirements for your simulation, you can
use the number of synapses.  One synapse is roughly equivalent to X.
For examples: The microcircuit model requires around 16 Gb of memory and the multi-area-model requires 1.4 Tb.
If a node has 128 Gb of memory then one node is more than sufficient for the microcircuit model but the multi-area model
will need 12 nodes to run.

|

The next two lines specify the process(task) and threading settings of the system. For NEST, we recommend a hybrid approach for
large simulations. This approach combines distributed computing (openMPI) along with thread parallel (OpenMP) simulations.

In this job script, we can state the number of processes (or tasks) and threads we use using with the ``ntasks-per-node`` and ``cpus-per-task``
options, respectively. Multiplied together, the values should equal the total number of cores in a node. (The number of cores
varies depending on what HPC system you are using).


``ntasks-per-node * cpus-per-task = number of cores in the node`` .

.. note::

    In NEST, the above calculation is the same one you would do to determine the  number of Virtual processes in a given simulation.
    See the guide to parallel_computing for more details.


::

   #SBATCH --ntasks-per-node=1

   #SBATCH --cpus-per-task=64

In this example, we are assuming there are 64 cores. We are using 1 MPI process (``ntasks-per-node``) and 64 threads
(``cpus-per-task``). We can increase the ``ntasks-per-node``
to 2, but then we would decrease the ``cpus-per-task`` to 32 (because we want the total to be 64).

|

::

   #SBATCH --hint=nomultithread

We suggest you include the line ``nomultithread`` to avoid the system from assigning 2 threads to a core.
Two threads per core can lead to slower performance.
The ``--hint=nomultithread`` causes Slurm to allocate only one thread from each core to this job.

|

We want to control the placement of the threads using OpenMP (also known as, pinning threads).

::

   export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

   export OMP_PROC_BIND=TRUE

The first line sets the number of threads to match what we stated earlier and then want to set ``OMP_PROC_BIND`` to ``True``. This
will prevent the threads from moving around.

|

::

   srun python my_nest_simulation.py

You can then tell the job script to schedule your simulation

----

Here is an example of how  `my_nest_simulation.py` could look:

.. code-block:: python

   import nest
   from nest import Create, Connect, Simulate

   # You must set the ``local_num_threads`` in your script.
   # It should match the ``cpus-per-task`` in the job script
   nest.local_num_threads = 64

   # In this example, we set the number of neurons to match the
   # number of threads. In this scenario each neuron would  be
   # placed on its own thread. In most setups, the number of
   # neurons would be different than the number of of threads.
   n = Create("iaf_psc_alpha", 64)
   pg = Create("poisson_generator", params={"rate": 50000.0})
   sr = Create("spike_recorder", params={"record_to": "ascii"})
   nest.Connect(pg, n, 'all_to_all', syn_spec={'weight': 100})
   nest.Connect(n, sr)
   nest.Simulate(100.)


Understanding parallelism in NEST
---------------------------------

To understand more how NEST utilizes thread parallel and distributed computing in simulations, see our guide on
:ref:`parallel_computing`.
We explain how neurons, devices, and synapses in NEST intersect with threads and processes in parallel setups.

Additional configuation options
-------------------------------

* Additional ways to optimize performance
* Useful for benchmarking


These configuration options may help improve performance, but are not necessary to include in your script.
If you are performing benchmarks or if you think your simulation can perform better, take a look at the following options.


.. list-table:: Additional Slurm settings
   :header-rows: 1

   * - Setting
     - Description
   * - `export CPU_AFFINITY=True`
     - make stuff do something
   * - `--exclusive`
     - Always set this to avoid other processes on the same node


MPI
~~~

It's usually unclear whether MPI pins threads to cores.
After allocation of resources on which one wants to run the MPI processes, one also needs to export environemnt
variables related the implementation of the multiprocessing API.

* OpenMPI
   * it appears that to use OpenMPI and Multithreading together one needs to, when building OpenMPI configure ``--enable-mpi-threads``
   * boost > 2.26(?) automatically contains an MPI implementation that can interfere

.. list-table:: OpenMPI settings
   :header-rows: 1

   * - Setting
     - Description
   *  - `exoort MPI_=True`
      - make it work
   *  - --map-by socket/node
      - map MPI processes to socket/node
   *  - --bind-to socket/node
      - bind MPI processes to socket/node
   *  - --rankfile rank.file
      - pass rankfile to precisely define threads on which to run a process (length of characters processed in rankfile is limited)
   *  - --report-bindings
      - report bindings of launched processes
   *  - --display-allocation
      - display detected allocation


Threading
~~~~~~~~~

* hardware threading settings
* power capping

.. list-table:: OpenMP settings
   :header-rows: 0

   * - export OMP_NUM_THREADS=#CPUSPERTASK#
     - variable telling OpenMP how many threads are used on a MPI process
   * - export OMP_PROC_BIND=true
     - no movement of threads between OpenMP threads and OpenMP places
   * - export OMP_PROC_BIND=close
     - no movement of threads between OpenMP threads and OpenMP places and OpenMP places are 'close' in a hardware sense
   * - export OMP_PLACES=threads/cores
     - each OpenMP place corresponds to a hardware thread/core
   * - export OPM_PLACES="{a : b : c}"
     - OpenMP places are a, a+b, a+2c, ... a+nc=b (numbering usually relates to cores/hardware threads)
   * - export OPM_DISPLAY_ENV=true
     - display OpenMP variables


Pinning threads
~~~~~~~~~~~~~~~

Pinning threads allows you to control the distribution of threads across available cores on your system, and is particularly
useful in high performance computing (HPC) systems.



Sequential pinning scheme
`````````````````````````

Description of what you're looking at

.. figure:: _static/img/CPUs-lin-lin.gif

   sequential placing - cores closely packed

   fill cores of 1 processor before going to next

- possible disadvantage
   - threads need to fight for resources - slower
- possible advantages
   - could shut off 2nd processor - save energy
   - could share memory prefetching

Distant pinning scheme
``````````````````````

.. figure:: _static/img/CPUs-lin-sparse.gif

   distant placing

   maximizes distance between threads in hardware



What are the commands here to set sequential / distant

Helpful commands
----------------

Here are some commands that can help you understand your system better.
You can run them locally on any Linux machine as well as on HPC systems.

* ``lstopo``
     lstopo and lstopo-no-graphics are capable of displaying a topological map of the system

* ``htop``
      It is similar to top, but allows you to scroll vertically and horizontally, so you can see all the processes
      running on the system, along with their full command lines.


This depends on the system you are running and the resources available to  you
But in general  we recommend

* 1 thread per core (in Slurm ``tasks-per-core``)
* --exclusive, (use the full node and do not share resources with another job).


References
----------

.. [1] Kurth AC, et al. 2021. Sub-realtime simulation of a neuronal network of natural density. arXiv
       Web. https://arxiv.org/abs/2111.04398



