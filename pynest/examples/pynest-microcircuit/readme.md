# Microcircuit Readme

Authors: Hendrik Rothe, Hannah Bos, Sacha van Albada   
May 2016

## Description ##
This is a PyNEST implementation of the SLI cortical microcircuit from the microcircuit model by Potjans and Diesmann (2014): The cell-type specific
cortical microcircuit: relating structure and activity in a full-scale spiking
network model. Cerebral Cortex: doi:10.1093/cercor/bhs358  

* Files:
	* network.py  
	file which gathers all parameters and connects the different nodes with each other
	* network\_params.py  
	contains the parameters for the network
	* sim\_params.py  
	contains the simulation parameters
	* stimulus\_params.py  
	contains the parameters for the stimuli
	* example.py  
   use this script to try out the microcircuit
   
How to use the example:

To run the microcircuit on a local machine, adjust the variables N\_scaling and K\_scaling in network_params.py to 0.1 and create an output directory called '/data'. Then run 'example.py' with python. The output will be saved in the directory '/data'.
The code can be parallelized using OpenMP and MPI, if NEST has been build with these applications. The number of threads (per MPI process) can be chosen by adjusting local\_num\_threads in sim_params.py. The number of MPI process can be set by running the script with the command 'mpirun -n num\_MPI\_processes python example.py'.

Tested configuration:
This version has been tested with NEST 2.10.0, Python 2.7.12, NumPy 1.11.1
