Microcircuit Example
====================

Description
###########

This is a PyNEST implementation of the microcircuit model by Potjans and Diesmann (2014) `[1] <https://www.ncbi.nlm.nih.gov/pubmed/23203991>`__. The original sli version can be found `here <https://github.com/nest/nest-simulator/tree/master/examples/nest/Potjans_2014>`__.

The current example contains several files:

* ``helpers.py`` provides helper functions for the simulation and evaluates the microcircuit.
* ``network.py`` gathers all parameters and connects the different nodes with one another.
* ``network_params.py`` contains the parameters for the network.
* ``sim_params.py`` contains the simulation parameters.
* ``stimulus_params.py`` contains the parameters for the stimuli.
* ``example.py`` uses this script to try out the microcircuit.

Instructions
############

To run the microcircuit on a local machine, we first have to check that the
variables ``N_scaling`` and ``K_scaling`` in ``network_params.py`` are set to
`0.1`. ``N_scaling`` adjusts the number of neurons and ``K_scaling`` adjusts
the number of connections to be simulated. The full network can be run by
adjusting these values to 1. If this is done, the option to print the time
progress should be set to False in the file ``sim_params.py``.

Install required packages:

.. code-block:: python

  pip install --user -r requirements.txt

For running, use ``python example.py``. The output will be saved in the directory ``data``.

The code can be `parallelized <https://www.nest-simulator.org/parallel-computing/>`_ using OpenMP and MPI, if NEST has been built with
these applications. The number of threads (per MPI process) can be chosen by adjusting
``local_num_threads`` in ``sim_params.py``. The number of MPI processes can be
set by choosing a reasonable value for ``num_mpi_prc`` and then running the
script with the following command:

.. code-block:: python

   mpirun -n num_mpi_prc python example.py


By default, the simulation uses Poissonian input to excite neuronal populations of the microcircuit. The file ``network_params.py`` defines Poissonian input.
If no Poissonian input is provided, DC input is calculated and should approximately compensate the Poissonian input. It is also possible to
add thalamic stimulation to the microcircuit or drive it with constant DC input. This can be defined in the file ``stimulus_params.py``.

Authors
#######

Hendrik Rothe, Hannah Bos, Sacha van Albada


