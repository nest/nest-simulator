# -*- coding: utf-8 -*-
#
# test_ticket_903.py
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


"""This test ensures that delays drawn from continuous distribution are not rounded up strictly."""

import nest
import numpy as np
import scipy.stats


def test_correct_rounding_distributions():
    nest.ResetKernel()
    nest.resolution = 1.0

    population = nest.Create("iaf_psc_alpha")
    indegree = 100
    significance = 0.01

    nest.Connect(
        population,
        population,
        syn_spec={"delay": nest.random.uniform(1.1, 1.9)},
        conn_spec={"rule": "fixed_indegree", "indegree": indegree},
    )

    delays = nest.GetConnections().delay

    assert set(delays) == {1, 2}
    assert scipy.stats.binomtest(sum(np.array(delays) == 2.0), indegree).pvalue > significance
