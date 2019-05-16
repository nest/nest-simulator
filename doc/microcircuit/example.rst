pynest microcircuit example
==============================

Example file to run the microcircuit.

Hendrik Rothe, Hannah Bos, Sacha van Albada; May 2016

The microcirucit model is described Potjans and Diesmann [1]_.
This example uses the function GetNodes, which is deprecated. A
deprecation warning is therefore issued. For details about deprecated
functions, see documentation.

To run the microcircuit on a local machine, adjust the variables
``N_scaling`` and ``K_scaling`` in :doc:`network_params.py <../../Potjans_2014/network_parms.py>` \
to ``0.1``. ``N_scaling`` adjusts the number of neurons and ``K_scaling`` the number
of connections to be simulated. The full network can be run by adjusting
these values to 1. If this is done, the option to print the time
progress should be set to False in the file ``sim_params.py``. For
running, use ``python example.py``. The output will be saved in the
directory ``data``.

The code can be parallelized using OpenMP and MPI, if NEST has been
built with these applications `(Parallel computing with
NEST) <https://www.nest-simulator.org/parallel_computing/>`__. The number
of threads (per MPI process) can be chosen by adjusting
``local_num_threads`` in ``sim_params.py``. The number of MPI process
can be set by choosing a reasonable value for ``num_mpi_prc`` and then
running the script with the command ``mpirun -n num_mpi_prc`` ``python``
``example.py``.

The default version of the simulation uses Poissonian input, which is
defined in the file ``network_params.py`` to excite neuronal populations
of the microcircuit. If no Poissonian input is provided, DC input is
calculated which should approximately compensate the Poissonian input.
It is also possible to add thalamic stimulation to the microcircuit or
drive it with constant DC input. This can be defined in the file
``stimulus_params.py``.

References
------------

.. [1]  Potjans TC. and Diesmann M. 2014. The cell-type specific cortical
        microcircuit: relating structure and activity in a full-scale spiking
        network model. Cerebral Cortex. 24(3):785–806. DOI: 10.1093/cercor/bhs358.


.. code-block:: python

  import time
  import numpy as np
  import network
  from network_params import net_dict
  from sim_params import sim_dict
  from stimulus_params import stim_dict


  # Initialize the network and pass parameters to it.
  tic = time.time()
  net = network.Network(sim_dict, net_dict, stim_dict)
  toc = time.time() - tic
  print("Time to initialize the network: %.2f s" % toc)
  # Connect all nodes.
  tic = time.time()
  net.setup()
  toc = time.time() - tic
  print("Time to create the connections: %.2f s" % toc)
  # Simulate.
  tic = time.time()
  net.simulate()
  toc = time.time() - tic
  print("Time to simulate: %.2f s" % toc)
  # Plot a raster plot of the spikes of the simulated neurons and the average
  # spike rate of all populations. For visual purposes only spikes 100 ms
  # before and 100 ms after the thalamic stimulus time are plotted here by
  # default. The computation of spike rates discards the first 500 ms of
  # the simulation to exclude initialization artifacts.
  raster_plot_time_idx = np.array(
      [stim_dict['th_start'] - 100.0, stim_dict['th_start'] + 100.0]
      )
  fire_rate_time_idx = np.array([500.0, sim_dict['t_sim']])
  net.evaluate(raster_plot_time_idx, fire_rate_time_idx)

