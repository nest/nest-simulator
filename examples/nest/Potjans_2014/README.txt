/*
 *  README.txt
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

Cortical microcircuit simulation: SLI version

This is an implementation of the multi-layer microcircuit model of early
sensory cortex published by Potjans and Diesmann (2014) The cell-type specific
cortical microcircuit: relating structure and activity in a full-scale spiking
network model. Cerebral Cortex: doi:10.1093/cercor/bhs358.


Files:
	- network_params.sli
	Script containing model parameters

        - sim_params.sli
        Script containing simulation and recording parameters

	- microcircuit.sli
	Simulation script

	- run_microcircuit.sh
	Bash script. Creates sim_script.sh and submits it to the queue

	- spike_analysis.py
	Python script for basic analysis

The bash script is designed for a cluster with a queuing system that uses qsub.
The actual simulation script 'microcircuit.sli' does not need to be changed.


Instructions:

1. Download NEST (https://www.nest-simulator.org/download)

2. Compile NEST: https://www.nest-simulator.org/installation
   Use the --with-mpi flag to configure with MPI support

3. In sim_params.sli adjust the following parameters:

   - 'run_mode': test or production
   - 'n_compute_nodes': the number of compute nodes
   - 'n_mpi_procs_per_node': the number of processes per compute node
   - 'n_threads_per_mpi_proc': the number of threads per mpi process
   - 'walltime_limit': a run rime time limit for the queuing system
   - 'memory_limit' a memory limit for the queuing system
   - 't_sim': the simulation time
   - 'nest_path': the base directory of the NEST installation
   - 'output_dir': a directory for the result data

   and choose recordables: cortical spikes, thalamic spikes, voltages

4. In network_params.sli:

   - Choose the network 'area', which scales the numbers of neurons
   - When down-scaling: Choose whether full-scale in-degrees should be used.
     Setting 'preserve_K' to true preserves most of the dynamics of the
     full-size network, as long as 'area' is not too small
   - Choose the external input: Poissonian noise 'bg_rate' and/or DC current
     'dc_amplitude'
   - Set any thalamic inputs parameters

5. Run the simulation by typing ./run_microcircuit.sh in your terminal
   (microcircuit.sli and the parameter files need to be in the same folder)

6. Output files and basic analysis:

   - Spikes are written to .gdf files containing node IDs of the recorded neurons
     and corresponding spike times in ms. The node IDs are unordered.
     Separate files are written out for each population and virtual process.
     File names are formed as spike detector label + layer index + population
     index + spike detector node ID + virtual process + .gdf
   - population_node IDs.dat contains the first and last global ID (node ID) of the
     neurons in each population in the order 2/3e, 2/3i, 4e, 4i, 5e, 5i, 6e, 6i
   - Voltages are written to .dat files containing node IDs, times in ms, and the
     corresponding membrane potentials in mV. File names are formed as
     voltmeter label + layer index + population index + spike detector node ID +
     virtual process + .dat

   - Run 'spike_analysis.py' with the variable 'datapath' set to the output
     folder in order to merge the spike files of each population (including
     thalamic ones, if present), sort node IDs, and produce dot plots and firing
     rate plots.
   - The analysis script does not currently cover voltages.

The simulation was successfully tested with MPI 1.4.3.
The analysis script works with Python 2.6.6 including packages numpy 1.3.0,
matplotlib 0.99.1.1, and glob.

---------------------------------------------------

Simulation on a single process:

1. After compiling NEST (not necessarily with MPI), go to the folder that
   includes microcircuit.sli and the parameter files and type 'nest' in your
   terminal.

2. Adjust 'area' and 'preserve_K' in network_params.sli such that the network
   is small enough to fit on your system.

3. Set the 'output_path' in user_params.sli to an existing directory.

5. Set 'n_compute_nodes' to 1, and 'n_mpi_procs_per_compute_node' and
   'n_threads_per_proc' in sim_params.sli to a suitable value for your computer.

4. Type '(microcircuit) run' to start the simulation on a single process.

A downscaled version ('area' = 0.1) of the network was tested on a single
MPI process with two threads with 'preserve_K' = true.
