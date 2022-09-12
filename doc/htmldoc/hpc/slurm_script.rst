.. _slurm_script:

Example Slurm script
====================

`Slurm <https://slurm.schedmd.com/documentation.html>`_ is a job scheduler used on many high performance computing systems.

Typically, you specify system parameters for the job you want to run in a job script.

This is an example job script that shows common settings useful when running NEST on HPC systems. The settings are applicable
to other job schedulers other than Slurm but the syntax will be different.
Always consult the documentation of the system you are running on to find out exactly what you need provide in your script.

You will likely need to alter the job script to optimize the performance on the system your're using.
Finding the optimal parameters for your script may require some trial and error.


.. seealso::

    * :ref:`overview_hardware`
    * :ref:`threads`
    * :ref:`mpi_process`

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

   # For one MPI process (ntasks-per-node)
   srun python3 my_nest_simulation.py

   # For > 1 MPI process (ntasks-per-node)
   module load openmpi
   mpirun -n <num_of_processes> python3 my_nest_simulation.py



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

The partition describes what architecture or hardware specifications your job will use.
For example, some systems have GPU and CPU partitions.
The actual partition name will depend on the HPC system and what it has available.

|

::

   #SBATCH --time=01:00:00

This is also known as wall time, which is the time allocated to your job.

|

::

  #SBATCH --nodes=1

The number of nodes needed for your job (see :ref:`figure here <overview_hardware>`). The number of nodes will depend on your memory needs and if you're
trying to increase the speed of the simulation.

How much memory does your simulation need? To get a rough estimate of the memory requirements for your simulation, you can
use the number of synapses.  One synapse is roughly equivalent to X.
For example: The microcircuit model requires around 16 GB of memory and the multi-area-model requires 1.4 TB.
If a node has 128 GB of memory then one node is more than sufficient for the microcircuit model but the multi-area model
will need 12 nodes to run.

|

The next two lines specify the process (task) and threading settings of the system. For NEST, we recommend a hybrid approach for
large simulations. This approach combines distributed computing (openMPI) along with thread parallel (OpenMP) simulations.

In this job script, we can state the number of processes (or tasks) and threads we use using with the ``ntasks-per-node`` and ``cpus-per-task``
options, respectively. Multiplied together, the values should equal the total number of cores in a node. (The number of cores
varies depending on what HPC system you are using).


``ntasks-per-node * cpus-per-task = number of cores in the node`` .

.. note::

    In NEST, the above calculation is the same one you would do to determine the number of Virtual processes in a given simulation.
    See the guide to :ref:`parallel_computing` for more details.


::

   #SBATCH --ntasks-per-node=1

   #SBATCH --cpus-per-task=64

In this example, we are assuming there are 64 cores in a node. We are using 1 MPI process (``ntasks-per-node``) and 64 threads
(``cpus-per-task``). We can increase the ``ntasks-per-node``
to 2, but then we would need to decrease the ``cpus-per-task`` to 32 (because we want the total to be 64).

|

::

   #SBATCH --hint=nomultithread

We suggest you include the line ``--hint=nomultithread`` to avoid the system from assigning 2 threads to a core.
Two threads per core can lead to slower performance in NEST.

|

We want to control the placement of the threads using OpenMP. This is referred to as pinning threads. (See section
:ref:`pinning_threads` for further details.)

::

   export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

   export OMP_PROC_BIND=TRUE

The first line sets the number of threads to match what we stated earlier and then want to set ``OMP_PROC_BIND`` to ``True``. This
will prevent the threads from moving around.

|


You can then tell the job script to schedule your simulation.

::

   srun python my_nest_simulation.py

Or, if you are using multiple MPI processes, you can invoke the MPI software explicitly:

::

  module load openmpi
  mpirun -n <num_of_processes> python3 my_nest_simulation.py

.. note:: 

   ``openmpi`` is but one MPI software available. Always check what is available on the system you are using.
    The `Slurm documentation <https://slurm.schedmd.com/mpi_guide.html#open_mpi>`_  contains additional options for running MPI.


----

Here is an example of the NEST script  ``my_nest_simulation.py``.

Don't forget to set ``local_num_threads`` in your script!

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

----




.. list-table:: Additional Slurm settings
   :header-rows: 1

   * - Setting
     - Description
   * - `export CPU_AFFINITY=True`
     - make stuff do something
   * - `--exclusive`
     - Prevents other processes or jobs from doing work on the same node



