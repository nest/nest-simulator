# Microcircuit Readme

Authors: Hendrik Rothe, Hannah Bos, Sacha van Albada   
May 2016

## Description ##
This is a PyNEST implementation of the microcircuit model by Potjans and Diesmann (2014): The cell-type specific
cortical microcircuit: relating structure and activity in a full-scale spiking
network model. Cerebral Cortex: doi:10.1093/cercor/bhs358

* Files:
	* `network.py`  
	File which gathers all parameters and connects the different nodes with each other.
	* `network_params.py`  
	Contains the parameters for the network.
	* `sim_params.py`  
	Contains the simulation parameters.
	* `stimulus_params.py`  
	Contains the parameters for the stimuli.
	* `example.py`  
   Use this script to try out the microcircuit.
   
How to use the example:

To run the microcircuit on a local machine, adjust the variables `N_scaling` and `K_scaling` in `network_params.py` to 0.1 and create an output directory called `data`. `N_scaling` adjusts the number of neurons and `K_scaling` the number of connections to be simulated. The full network can be run by adjusting these values to 1. For running, use `python example.py`. The output will be saved in the directory `data`.
The code can be parallelized using OpenMP and MPI, if NEST has been built with these applications [Parralel computing with NEST](http://www.nest-simulator.org/parallel_computing/). The number of threads (per MPI process) can be chosen by adjusting `local_num_threads` in `sim_params.py`. The number of MPI process can be set by running the script with the command `mpirun -n num_MPI_processes` `python` `example.py`.

Tested configuration:
This version has been tested with NEST 2.10.0, Python 2.7.12, NumPy 1.11.1
