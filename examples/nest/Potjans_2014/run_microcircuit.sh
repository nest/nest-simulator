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

# Create output folder (if it does not exist yet) and sim_script.sh
# and submit to the queue. Adapt to your system as necessary

OUTPUT_PATH=$(grep '/output_path' sim_params.sli | cut -d'(' -f2 | cut -d ')' -f1)
OUTPUT_PATH=$(cd $OUTPUT_PATH; pwd)
N_COMPUTE_NODES=$(grep '/n_compute_nodes' sim_params.sli | cut -d' ' -f2)
N_MPI_PER_COMPUTE_NODE=$(grep '/n_mpi_procs_per_compute_node' sim_params.sli | cut -d' ' -f2)
WALLTIME_LIMIT=$(grep '/walltime_limit' sim_params.sli | cut -d'(' -f2 | cut -d ')' -f1)
MEMORY_LIMIT=$(egrep '/memory_limit' sim_params.sli | cut -d'(' -f2 | cut -d ')' -f1)

NEST_PATH=$(grep '/nest_path' sim_params.sli | cut -d '(' -f2 | cut -d ')' -f1)
STDOUT=$(grep '/std_out' sim_params.sli | cut -d'(' -f2 | cut -d')' -f1)
STDERR=$(grep '/std_err' sim_params.sli | cut -d'(' -f2 | cut -d')' -f1)

mkdir -p $OUTPUT_PATH
cp 'sim_params.sli' $OUTPUT_PATH
cp 'network_params.sli' $OUTPUT_PATH
cp 'microcircuit.sli' $OUTPUT_PATH
cd $OUTPUT_PATH

echo > sim_script.sh
chmod 755 sim_script.sh

echo "#PBS -o $OUTPUT_PATH/$STDOUT" >> sim_script.sh
echo "#PBS -e $OUTPUT_PATH/$STDERR" >> sim_script.sh
echo "#PBS -l walltime=$WALLTIME_LIMIT" >> sim_script.sh
echo "#PBS -l mem=$MEMORY_LIMIT" >> sim_script.sh
echo ". $NEST_PATH/bin/nest_vars.sh" >> sim_script.sh
echo "NEST_DATA_PATH=$OUTPUT_PATH" >> sim_script.sh
echo "mpirun -machinefile \$PBS_NODEFILE nest $OUTPUT_PATH/microcircuit.sli" >> sim_script.sh
echo "unset NEST_DATA_PATH" >> sim_script.sh

echo qsub -l nodes=$N_COMPUTE_NODES:ppn=$N_MPI_PER_COMPUTE_NODE sim_script.sh
