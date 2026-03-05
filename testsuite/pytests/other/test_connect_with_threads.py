# -*- coding: utf-8 -*-
#
# test_connect_with_threads.py
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
This test goes through all the connection rules and test connectivity for
a range of threads with the given rule. For each rule, we check that
connectivity works when total number of threads ranges from 1 to 25. We should
have more threads than targets after a while.

"""

import nest
import pytest

pytestmark = pytest.mark.skipif_missing_threads


@pytest.fixture(autouse=True)
def prepare():
    nest.ResetKernel()


def create_network(num_threads, conn_dict):
    nest.total_num_virtual_procs = num_threads
    pop1 = nest.Create("iaf_psc_alpha", 10)
    pop2 = nest.Create("iaf_psc_alpha", 10)

    syn_dict = {"synapse_model": "static_synapse"}
    nest.Connect(pop1, pop2, conn_dict, syn_dict)


@pytest.mark.parametrize("num_threads", range(1, 26))
def test_one_to_one(num_threads):
    conn_dict = {"rule": "one_to_one"}
    create_network(num_threads, conn_dict)

    assert nest.num_connections == 10


@pytest.mark.parametrize("num_threads", range(1, 26))
def test_all_to_all(num_threads):
    conn_dict = {"rule": "all_to_all"}
    create_network(num_threads, conn_dict)

    assert nest.num_connections == 100


@pytest.mark.parametrize("num_threads", range(1, 26))
def test_fixed_indegree(num_threads):
    conn_dict = {"rule": "fixed_indegree", "indegree": 5}
    create_network(num_threads, conn_dict)

    assert nest.num_connections == 50


@pytest.mark.parametrize("num_threads", range(1, 26))
def test_fixed_outdegree(num_threads):
    conn_dict = {"rule": "fixed_outdegree", "outdegree": 5}
    create_network(num_threads, conn_dict)

    assert nest.num_connections == 50


@pytest.mark.parametrize("num_threads", range(1, 26))
def test_fixed_total_number(num_threads):
    conn_dict = {"rule": "fixed_total_number", "N": 5}
    create_network(num_threads, conn_dict)

    assert nest.num_connections == 5


@pytest.mark.parametrize("num_threads", range(1, 26))
def test_pairwise_bernoulli(num_threads):
    nest.rng_seed = 100
    conn_dict = {"rule": "pairwise_bernoulli", "p": 0.1}
    create_network(num_threads, conn_dict)

    num_conn = nest.num_connections

    diff = abs(num_conn - 10)

    assert num_conn <= diff + 10
    assert num_conn >= 10 - diff
