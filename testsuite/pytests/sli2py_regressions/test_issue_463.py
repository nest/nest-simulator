# -*- coding: utf-8 -*-
#
# test_issue_463.py
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

"""This test creates connections between neurons with parameter arrays and connection rule fixed_indegree,
with more than one virtual process, and checks that created connection parameters coincide with array elements."""

import nest
import numpy as np
import pytest


@pytest.mark.skipif_missing_threads
def test_simulation_completes():
    """
    Create identical source and target population and connect with fixed indegree and generated weights.
    Check whether the sorted output weights of each neuron match the original input.
    """
    nest.ResetKernel()
    nest.local_num_threads = 4
    nest.set_verbosity("M_ERROR")

    population_size = 10
    population_type = "iaf_psc_alpha"
    in_degree = 5

    source_population = nest.Create(population_type, population_size)
    target_population = nest.Create(population_type, population_size)

    generated_weights = np.arange(1, population_size * in_degree + 1).reshape((population_size, in_degree))

    nest.Connect(
        source_population,
        target_population,
        {
            "rule": "fixed_indegree",
            "indegree": in_degree,
        },
        {"synapse_model": "static_synapse", "weight": generated_weights},
    )

    weights = np.array([sorted(nest.GetConnections(target=t).weight) for t in target_population])

    assert np.all(weights == generated_weights)
