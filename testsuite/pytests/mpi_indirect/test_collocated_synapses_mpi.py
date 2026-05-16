# -*- coding: utf-8 -*-
#
# test_collocated_synapses_mpi.py
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
@MPITestAssertEqual([1, 2, 4], debug=False)
def test_collocated_synapses():
    """
    Test that lists of parameter dictionaries for collocated synapses are handled correctly.

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

    src = nest.Create("parrot_neuron", num_src)
    tgt = nest.Create("parrot_neuron", num_tgt)

    nest.Connect(src, tgt, "one_to_one", coll_syns)

    conns = pd.DataFrame.from_dict(nest.GetConnections().get(["source", "target", "weight", "delay", "synapse_model"]))

    conns.to_csv(OTHER_LABEL.format(nest.num_processes, nest.Rank()), index=False, sep="\t")  # noqa: F821
