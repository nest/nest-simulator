# -*- coding: utf-8 -*-
#
# test_parallel_conn_and_rand.py
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

import numpy as np
import pandas as pd
import pytest
from mpi_test_wrapper import MPITestAssertEqual

# Functions to be passed via decorators must be in module namespace without qualifiers
from numpy import meshgrid


@pytest.mark.skipif_incompatible_mpi
@pytest.mark.skipif_missing_threads
@MPITestAssertEqual([1, 2, 4], debug=False)
def test_parallel_cond_and_rand():
    """
    Parallel Connect and Randomize Test Script

    This script creates a neuron population to itself, randomizing
    weight, delay, receptor type and one synaptic parameter. It runs
    with a fixed number of virtual processes, and checks that connections
    are invariant on executing with a varying number of MPI processes.
    """

    import nest
    import pandas as pd  # noqa: F811

    nest.total_num_virtual_procs = 16
    nest.rng_seed = 123

    num_neurons = 100
    tau_syns = np.linspace(0.2, 1.0, 9)
    receptor_min = 1
    receptor_max = len(tau_syns)

    pop = nest.Create("iaf_psc_alpha_multisynapse", num_neurons, {"tau_syn": tau_syns})

    nest.Connect(
        pop,
        pop,
        {"rule": "fixed_indegree", "indegree": 23},
        {
            "synapse_model": "stdp_synapse",
            "delay": nest.random.uniform(0.5, 1.5),
            "weight": nest.math.max(nest.random.normal(10, 5), 0),
            "receptor_type": 1 + nest.random.uniform_int(receptor_max + 1 - receptor_min),
            "alpha": nest.random.uniform(0.1, 2.3),
            "tau_plus": nest.random.uniform(1.5, 5.0),
        },
    )

    conns = pd.DataFrame.from_dict(
        nest.GetConnections().get(["source", "target", "weight", "delay", "alpha", "tau_plus"])
    )

    conns.to_csv(OTHER_LABEL.format(nest.num_processes, nest.Rank()), index=False, sep="\t")  # noqa: F821
