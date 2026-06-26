# -*- coding: utf-8 -*-
#
# test_collocated_synapses_spatial_mpi.py
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
def test_collocated_synapses_spatial():
    """
    Test that lists of parameter dictionaries for collocated synapses are handled correctly.

    This test is very similar to test_collocated_synapses_mpi, but for spatial networks.

    We assume that the correct handling of collocated synapses for the serial case is established.
    Comparing results with that case for the parallel case is thus sufficient, no need to test
    expected values explicitly here.
    """

    import nest
    import pandas as pd  # noqa: F811

    num_src = 4
    num_tgt = 4
    num_conns = 4

    coll_syns = nest.CollocatedSynapses(
        {"weight": -3, "delay": 1, "synapse_model": "static_synapse"},
        {"weight": 4, "delay": 3, "synapse_model": "static_synapse"},
        {"weight": 2, "delay": 1, "synapse_model": "stdp_synapse"},
    )

    # Since we create probabilistic connections, we need to fix the number of VPs
    nest.total_num_virtual_procs = 4

    src = nest.Create("parrot_neuron", positions=nest.spatial.grid(shape=[3, 3], extent=[1.25, 1.25]))
    tgt = nest.Create("parrot_neuron", positions=nest.spatial.grid(shape=[3, 3], extent=[1.25, 1.25]))

    nest.Connect(src, tgt, {"rule": "fixed_indegree", "indegree": num_conns}, coll_syns)

    conns = pd.DataFrame.from_dict(nest.GetConnections().get(["source", "target", "weight", "delay", "synapse_model"]))

    conns.to_csv(OTHER_LABEL.format(nest.num_processes, nest.Rank()), index=False, sep="\t")  # noqa: F821
