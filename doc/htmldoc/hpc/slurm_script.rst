.. _slurm_script:

Example Slurm script
====================

`Slurm <https://slurm.schedmd.com/documentation.html>`_ is a job scheduler used on many high performance computing systems.

Typically, you specify system parameters for the job you want to run in a job script.

This is an example job script that shows common settings useful when running NEST on HPC systems. The settings are applicable
to other job schedulers other than Slurm but the syntax will be different.

.. note::

   The example script is compatible with Slurm version <22.05.
   Always consult the documentation of the system you are running on to find out exactly what you need to provide in your script.

You will likely need to alter the job script to optimize the performance on the system your're using.
Finding the optimal parameters for your script may require some trial and error.


.. seealso::

    * :ref:`overview_hardware`
    * :ref:`threads`
    * :ref:`mpi_process`

In this example, we are using 1 node, which contains 2 sockets and 64 cores per socket.

.. code-block:: sh

   #!/bin/bash -l
   #SBATCH --job-name=<job-name>
   #SBATCH --account=<account-name>
   #SBATCH --partition=<partition-type>
   #SBATCH --time=01:00:00
   #SBATCH --nodes=1
   #SBATCH --ntasks-per-node=2
   #SBATCH --cpus-per-task=64
   #SBATCH --hint=nomultithread

   export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
   export OMP_PROC_BIND=TRUE

   # Optional
   python -c "import nest, subprocess as s, os; s.check_call(['/usr/bin/pldd', str(os.getpid())])" 2>&1 | tee -a "pldd-nest.out"

   # On some systems, MPI is run by SLURM
   srun --exclusive python3 my_nest_simulation.py

   # On other systems, you must explicitly call MPI:
   mpirun -n <num_of_processes> --hostfile <hostfile> python3 my_nest_simulation.py



----

.. note::

    Slurm can set pinning to specific CPUs for you with the environment variable ``CPU_AFFINITY``.
    Setting this to ``True`` may lead to problems with any pinning settings you
    have set manually. We recommend setting this to ``None``.


Let's break this script down line by line.

::

  #!/bin/bash -l

You are submitting a shell script to Slurm. The "shebang" line must be the first line of shell script

|

::

   #SBATCH --job-name=<job-name>

The name of the job should be unique and descriptive enough to differentiate it from other jobs.

|

::

   #SBATCH --account=<account-name>

Name of your account

|

::

   #SBATCH --partition=<partition-type>

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

.. note::

   How many nodes do you need for your simulations?
   This depends on how much memory is available for each node.

   For example: The :doc:`microcircuit model <../auto_examples/Potjans_2014/index>` requires around 16 GB of memory and the `multi-area-model <https://github.com/INM-6/multi-area-model>`_ requires 1.4 TB.
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

    In NEST, the above calculation is the same one you would do to determine the number of `virtual processes` in a given simulation.
    See the :ref:`parallel_computing` for more details.


::

   #SBATCH --ntasks-per-node=2

   #SBATCH --cpus-per-task=64

In this example, we are assuming there are 128 cores in a node. We are using 2 MPI processes (``ntasks-per-node``) and 64 threads
(``cpus-per-task``). We can increase the ``ntasks-per-node``
to 4, but then we would want to decrease the ``cpus-per-task`` to 32 (because we want the total to be 128).
This ensures we are fully utilizing the resources.

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

::

   python -c "import nest, subprocess as s, os; s.check_call(['/usr/bin/pldd', str(os.getpid())])" 2>&1 | tee -a "pldd-nest.out"

Prints out the linked libraries into a file with name ``pldd-nest.out``.
In this way, you can check whether dynamically linked librariries for
the execution of ``nest`` is indeed used. For example, if you want to check if ``jemalloc`` is used for the network construction
in highly parallel simulations.

.. note::

   The above command uses ``pldd`` which is commonly available in Linux distributions. However, you might need to change
   the path, which you can find with the command ``which pldd``.

|

You can then tell the job script to schedule your simulation.
Setting the ``exclusive`` option prevents other processes or jobs from doing work on the same node.

::

   srun --exclusive python my_nest_simulation.py

Or, if you are using multiple MPI processes, you can invoke the MPI software explicitly:

::

  mpirun -n <num_of_processes> python3 my_nest_simulation.py




----

Set ``local_num_threads`` in your NEST script
---------------------------------------------

Here is a simple example of the NEST script ``my_nest_simulation.py``.

To ensure the correct number of threads are used, you have to set ``local_num_threads`` in your script!
It should match the number of ``cpus-per-task``.

.. code-block:: python

   import nest

   # Set the local_num_threads to match the value in your job script.
   nest.local_num_threads = 64

   # In this example, we set the number of neurons to match the
   # number of threads. In this scenario each neuron would  be
   # placed on its own thread. In most setups, the number of
   # neurons would be different than the number of of threads.
   n = nest.Create("iaf_psc_alpha", 64)
   pg = nest.Create("poisson_generator", params={"rate": 50000.0})
   sr = nest.Create("spike_recorder", params={"record_to": "ascii"})
   nest.Connect(pg, n, 'all_to_all', syn_spec={'weight': 100})
   nest.Connect(n, sr)
   nest.Simulate(100.)

.. seealso::

    :ref:`parallel_computing`
