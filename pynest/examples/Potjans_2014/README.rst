Microcircuit Example
=====================

Description
-----------

 This is a PyNEST implementation of the microcircuit model by Potjans and
 Diesmann [1]_ The original sli version can be found `here <https://github.com/nest/nest-simulator/tree/master/examples/nest/Potjans_2014>`__.

-  This example contains several files:

   -  ``helpers.py``
      Helper functions for the simulation and evaluation of the
      microcircuit.
   -  ``network.py``
      Gathers all parameters and connects the different nodes with each
      other.
   -  ``network_params.py``
      Contains the parameters for the network.
   -  ``sim_params.py``
      Contains the simulation parameters.
   -  ``stimulus_params.py``
      Contains the parameters for the stimuli.
   -  ``example.py``
      Use this script to try out the microcircuit.

**How to use the Microcircuit model example:**

To run the microcircuit on a local machine, we have to first check that the
variables ``N_scaling`` and ``K_scaling`` in ``network_params.py`` are set to
`0.1`. ``N_scaling`` adjusts the number of neurons and ``K_scaling`` adjusts
the number of connections to be simulated. The full network can be run by
adjusting these values to 1. If this is done, the option to print the time
progress should be set to False in the file ``sim_params.py``. For running, use
``python example.py``. The output will be saved in the directory ``data``.

The code can be parallelized using OpenMP and MPI, if NEST has been built with
these applications `(Parallel computing with NEST) <https://www.nest-simulator.org/parallel_computing/>`__.
The number of threads (per MPI process) can be chosen by adjusting
``local_num_threads`` in ``sim_params.py``. The number of MPI processes can be
set by choosing a reasonable value for ``num_mpi_prc`` and then running the
script with the following command.

.. code-block:: python

   mpirun -n num_mpi_prc python example.py


The default version of the simulation uses Poissonian input, which is defined
in the file ``network_params.py`` to excite neuronal populations of the
microcircuit. If no Poissonian input is provided, DC input is calculated, which
should approximately compensate the Poissonian input. It is also possible to
add thalamic stimulation to the microcircuit or drive it with constant DC
input. This can be defined in the file ``stimulus_params.py``.

Authors
--------

Hendrik Rothe, Hannah Bos, Sacha van Albada


