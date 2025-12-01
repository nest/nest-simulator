# -*- coding: utf-8 -*-
#
# test_issue_665.py
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
Regression test for Issue #665 (GitHub).

Test ported from SLI regression test
This test ensures that ConnectLayers correctly handles devices with thread
siblings as sources and targets.

See also
- https://github.com/nest/nest-simulator/issues/665
- https://github.com/nest/nest-simulator/pull/666
"""

import nest
import pytest

pytestmark = pytest.mark.skipif_missing_threads


@pytest.fixture(autouse=True)
def reset_kernel():
    nest.ResetKernel()


@pytest.mark.parametrize(
    "connspec",
    [
        {"rule": "pairwise_bernoulli", "p": 1, "use_on_source": True},
        {"rule": "pairwise_bernoulli", "p": 1, "use_on_source": False},
        {"rule": "fixed_indegree", "indegree": 1, "allow_multapses": False},
        {"rule": "fixed_outdegree", "outdegree": 4, "allow_multapses": False},
    ],
)
def test_connect_generator_layer(connspec):
    """
    Confirm that devices can be connected as sources to layers with arbitrary connection rules.
    Each neuron should appear exactly once as a connection target.
    """

    nest.local_num_threads = 4

    gen_layer = nest.Create("poisson_generator", positions=nest.spatial.grid(shape=[1, 1]))
    nrn_layer = nest.Create("parrot_neuron", positions=nest.spatial.grid(shape=[2, 2]))

    nest.Connect(gen_layer, nrn_layer, connspec)
    targets = nest.GetConnections(source=gen_layer).get("target")

    assert sorted(targets) == list(nrn_layer.global_id)


@pytest.mark.parametrize(
    "connspec,pass_expected",
    [
        [{"rule": "pairwise_bernoulli", "p": 1, "use_on_source": True}, True],
        [{"rule": "pairwise_bernoulli", "p": 1, "use_on_source": False}, False],
        [{"rule": "fixed_indegree", "indegree": 4, "allow_multapses": False}, False],
        [{"rule": "fixed_outdegree", "outdegree": 1, "allow_multapses": False}, False],
    ],
)
def test_connect_recorder_layer(connspec, pass_expected):
    """
    Confirm that neurons can be connected to a recorder with pairwise bernoulli used on source.
    Other connection rules shall trigger an error. If connections are created, each neuron shall
    appear exactly once as a connection source.
    """

    nest.local_num_threads = 4

    nrn_layer = nest.Create("parrot_neuron", positions=nest.spatial.grid(shape=[2, 2]))
    rec_layer = nest.Create("spike_recorder", positions=nest.spatial.grid(shape=[1, 1]))

    if pass_expected:
        nest.Connect(nrn_layer, rec_layer, connspec)
        sources = nest.GetConnections(target=rec_layer).get("source")
        assert sorted(sources) == list(nrn_layer.global_id)
    else:
        with pytest.raises(nest.kernel.NESTErrors.IllegalConnection):
            nest.Connect(nrn_layer, rec_layer, connspec)
