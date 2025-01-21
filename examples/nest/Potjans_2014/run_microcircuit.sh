#!/bin/bash
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

# Create output folder (if it does not exist yet) and run the simulation
# Adapt this file to your HPC queueing system as needed
set -euo pipefail

# Use these variables to submit your job
N_COMPUTE_NODES="$(grep '/n_compute_nodes' sim_params.sli | cut -d' ' -f2)"
N_MPI_PER_COMPUTE_NODE="$(grep '/n_mpi_procs_per_compute_node' sim_params.sli | cut -d' ' -f2)"
WALLTIME_LIMIT="$(grep '/walltime_limit' sim_params.sli | cut -d'(' -f2 | cut -d ')' -f1)"
MEMORY_LIMIT="$(grep '/memory_limit' sim_params.sli | cut -d'(' -f2 | cut -d ')' -f1)"
STDOUT="$(grep '/std_out' sim_params.sli | cut -d'(' -f2 | cut -d')' -f1)"
STDERR="$(grep '/std_err' sim_params.sli | cut -d'(' -f2 | cut -d')' -f1)"

# Resolve paths
OUTPUT_PATH="$(grep '/output_path' sim_params.sli | cut -d'(' -f2 | cut -d ')' -f1)"
OUTPUT_PATH="$(cd "$OUTPUT_PATH" || exit 1; pwd)"
NEST_PATH="$(grep '/nest_path' sim_params.sli | cut -d '(' -f2 | cut -d ')' -f1)"

# Prepare output directory
mkdir -p "$OUTPUT_PATH"
cp 'sim_params.sli' "$OUTPUT_PATH"
cp 'network_params.sli' "$OUTPUT_PATH"
cp 'microcircuit.sli' "$OUTPUT_PATH"
cd "$OUTPUT_PATH"

# Run
# shellcheck source=bin/nest_vars.sh.in
. "$NEST_PATH/bin/nest_vars.sh"
NEST_DATA_PATH="$OUTPUT_PATH"
export NEST_DATA_PATH STDOUT STDERR MEMORY_LIMIT WALLTIME_LIMIT N_MPI_PER_COMPUTE_NODE N_COMPUTE_NODES
mpirun nest "$OUTPUT_PATH/microcircuit.sli"
unset NEST_DATA_PATH
