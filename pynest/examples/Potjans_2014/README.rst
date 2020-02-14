PyNEST Microcircuit
===================

Description
###########

This is a PyNEST implementation of the microcircuit model by Potjans and Diesmann [1]_.
The network model represents 4 layers of cortex, L2/3, L4, L5, and L6, each consisting of 2 populations of excitatory and inhibitory neurons.
The original sli version can be found `here <https://github.com/nest/nest-simulator/tree/master/examples/nest/Potjans_2014>`__.

The current example contains several files:

* ``run_microcircuit.py``: an example script to try out the microcircuit
* ``network.py``: the main Network class with functions to build and simulate the network
* ``helpers.py``: helper functions for network construction, simulation and evaluation
* ``network_params.py``: network and neuron parameters
* ``stimulus_params.py``: parameters for optional external stimulation
* ``sim_params.py``: simulation parameters
* ``reference_data``: reference data and figures obtained by executing ``run_microcircuit.py`` with ``N_scaling`` and ``K_scaling`` set to `1`

Instructions
############

By default, the variables ``N_scaling`` and ``K_scaling`` in ``network_params.py`` are set to
`0.1` which is a good choice for running the microcircuit on a local machine.
``N_scaling`` adjusts the number of neurons and ``K_scaling`` adjusts the number of connections to be simulated.
The full network can be run by adjusting these values to `1`.
If this is done, the option to print the time progress should be switched off: ``'print_time': False`` in ``sim_params.py``.

To run the simulation, simply use:

.. code-block:: bash

   python run_microcircuit.py

The output will be saved in the directory ``data``.

The code can be `parallelized <https://www.nest-simulator.org/parallel-computing/>`_ using OpenMP and MPI, if NEST has been built with these applications.
The number of threads (per MPI process) can be chosen by adjusting ``local_num_threads`` in ``sim_params.py``.
The number of MPI processes can be set by choosing a reasonable value for ``num_mpi_prc`` and then running the script with the following command:

.. code-block:: bash

   mpirun -n num_mpi_prc python run_microcircuit.py


By default, the simulation uses external Poissonian input to excite all neuronal populations of the microcircuit, see ``poisson_input': True`` in ``network_params.py``.
If set to ``False``, the Poissonian input is turned off and compensated approximately by calculated DC input.
In addition to this ongoing external drive, a thalamic stimulation can be switched on in ``stimulus_params.py``; the default is ``'thalamic_input': False``.
Also for the thalamic stimulation, it is possible to replace the default Poissonian input by DC input.

The default random initialization of membrane voltages in this simulation uses population-specific means and standard deviations to reduce an initial activity burst in the network: ``'V_type': 'optimized'`` in ``network_params.py``.
Previous implementations used the same mean and standard deviation for all populations which corresponds to setting ``'V_type': 'original'``.

References
##########


.. [1]  Potjans TC. and Diesmann M. 2014. The cell-type specific cortical
        microcircuit: relating structure and activity in a full-scale spiking
        network model. Cerebral Cortex. 24(3):785–806. DOI: 10.1093/cercor/bhs358.

Authors: Hendrik Rothe, Hannah Bos, Sacha van Albada


