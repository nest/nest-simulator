# -*- coding: utf-8 -*-
#
# test_issue_351.py
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

"""
Regression test for Issue #351 (GitHub).

This test ensures `Connect` raises exception if connecting to recording device with probabilistic connection rule.
"""

import nest
import pytest


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


ignore_model = [
    "multimeter",  # inverse order
    "voltmeter",  # inverse order
    "weight_recorder",  # attaches to synapses
    "correlation_detector",  # has proxies
    "correlomatrix_detector",  # has proxies
    "spin_detector",  # binary recorder
    "correlospinmatrix_detector",  # binary recorder
]

models = [m for m in nest.node_models if nest.GetDefaults(m, "element_type") == "recorder" and m not in ignore_model]


@pytest.mark.parametrize("model", models)
@pytest.mark.parametrize(
    "conn_spec",
    [
        {"rule": "fixed_indegree", "indegree": 1},
        {"rule": "fixed_outdegree", "outdegree": 1},
        {"rule": "fixed_total_number", "N": 1},
        {"rule": "pairwise_bernoulli", "p": 1.0},
    ],
)
def test_error_raised_for_illegal_connections_to_recording_device(model, conn_spec):
    """
    Test that an error is raised for prohibited connections to recording device.

    This test ensures that connections to devices without proxies are prohibited
    for probabilistic connection rules.
    """

    nrn = nest.Create("iaf_psc_alpha", 1)
    rec = nest.Create(model, 2)

    with pytest.raises(nest.NESTErrors.IllegalConnection):
        nest.Connect(nrn, rec, conn_spec=conn_spec)
