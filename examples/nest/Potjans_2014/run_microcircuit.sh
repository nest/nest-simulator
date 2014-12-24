# -*- coding: utf-8 -*-
#
# run_microcircuit.sh
#
# This file is part of NEST.
#
# Copyright (C) 2004 The NEST Initiative
#
# NEST is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# NEST is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with NEST.  If not, see <http://www.gnu.org/licenses/>.

#!/bin/bash

# creates output folder if it does not exist yet, creates sim_script.sh, 
# and submits it to the queue
#
# adapt to your system as necessary

# read in parameters and paths from user_params.sli and sim_params.sli
# output directory
path=`egrep '/output_path' user_params.sli|cut -f 2 -d '('|cut -f 1 -d ')'`
# number of nodes
n_nodes=`egrep '/n_mpi_procs' sim_params.sli|cut -f 2 -d ' '`
# number of processors per node
n_procs_per_node=`egrep '/n_threads_per_proc' sim_params.sli|cut -f 2 -d ' '`
# walltime
walltime=`egrep '/walltime' sim_params.sli|cut -f 2 -d '('|cut -f 1 -d ')'`
# memory allocation
memory=`egrep '/memory' sim_params.sli|cut -f 2 -d '('|cut -f 1 -d ')'`

# path for mpi 
mpi_path=`egrep '/mpi' user_params.sli|cut -f 2 -d '('|cut -f 1 -d ')'`
# path for nest
nest_path=`egrep '/nest_path' user_params.sli|cut -f 2 -d '('|cut -f 1 -d ')'`
# standard output file name
output=`egrep '/std_out' sim_params.sli|cut -f 2 -d '('|cut -f 1 -d ')'`
# error output file name
errors=`egrep '/error_out' sim_params.sli|cut -f 2 -d '('|cut -f 1 -d ')'`

# copy simulation scripts to output directory
mkdir -p $path
cp 'user_params.sli' $path
cp 'sim_params.sli' $path
cp 'network_params.sli' $path
cp 'microcircuit.sli' $path
cd $path

# create sim_script.sh
echo > sim_script.sh
chmod 755 sim_script.sh

echo "#PBS -o $path/$output" >> sim_script.sh
echo "#PBS -e $path/$errors" >> sim_script.sh
echo "#PBS -l walltime=$walltime" >> sim_script.sh
echo "#PBS -l mem=$memory" >> sim_script.sh
echo ". $mpi_path" >> sim_script.sh
echo "cd $path/" >> sim_script.sh
echo -n "mpirun -machinefile \$PBS_NODEFILE " >> sim_script.sh
echo "$nest_path $path/microcircuit.sli" >> sim_script.sh

# The variable PBS_NODEFILE is set dynamically when invoking qsub
qsub -l nodes=$n_nodes:ppn=$n_procs_per_node sim_script.sh

