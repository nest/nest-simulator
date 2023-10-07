# -*- coding: utf-8 -*-
#
# test_tripartite_connect.py
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

import nest


def test_connect_all():
    n_pre, n_post, n_third = 4, 2, 3
    pre = nest.Create("parrot_neuron", n_pre)
    post = nest.Create("parrot_neuron", n_post)
    third = nest.Create("parrot_neuron", n_third)

    nest.TripartiteConnect(
        pre, post, third, {"rule": "tripartite_bernoulli_with_pool", "p_primary": 1.0, "p_cond_third": 1}
    )

    n_primary = n_pre * n_post
    assert len(nest.GetConnections(pre, post)) == n_primary
    assert len(nest.GetConnections(pre, third)) == n_primary
    assert len(nest.GetConnections(third, post)) == n_primary


def test_connect_astro():
    n_pre, n_post, n_third = 4, 2, 3
    pre = nest.Create("aeif_cond_alpha_astro", n_pre)
    post = nest.Create("aeif_cond_alpha_astro", n_post)
    third = nest.Create("astrocyte_lr_1994", n_third)

    nest.TripartiteConnect(
        pre,
        post,
        third,
        {"rule": "tripartite_bernoulli_with_pool", "p_primary": 1.0, "p_cond_third": 1},
        {"third_out": {"synapse_model": "sic_connection"}},
    )

    n_primary = n_pre * n_post
    assert len(nest.GetConnections(pre, post)) == n_primary
    assert len(nest.GetConnections(pre, third)) == n_primary
    assert len(nest.GetConnections(third, post)) == n_primary


def test_explicit_random_pool():
    n_pre, n_post, n_third = 4, 2, 3
    pre = nest.Create("parrot_neuron", n_pre)
    post = nest.Create("parrot_neuron", n_post)
    third = nest.Create("parrot_neuron", n_third)

    nest.TripartiteConnect(
        pre, post, third, {"rule": "tripartite_bernoulli_with_pool", "pool_type": "random", "pool_size": 2}
    )

    n_primary = n_pre * n_post
    assert len(nest.GetConnections(pre, post)) == n_primary
    assert len(nest.GetConnections(pre, third)) == n_primary
    assert len(nest.GetConnections(third, post)) == n_primary


def test_block_pool_single():
    n_pre, n_post, n_third = 4, 4, 2
    pre = nest.Create("parrot_neuron", n_pre)
    post = nest.Create("parrot_neuron", n_post)
    third = nest.Create("parrot_neuron", n_third)

    nest.TripartiteConnect(
        pre, post, third, {"rule": "tripartite_bernoulli_with_pool", "pool_type": "block", "pool_size": 1}
    )

    n_primary = n_pre * n_post
    assert len(nest.GetConnections(pre, post)) == n_primary
    assert len(nest.GetConnections(pre, third)) == n_primary
    assert len(nest.GetConnections(third, post)) == n_primary


def test_block_pool_wide():
    n_pre, n_post, n_third = 4, 2, 8
    pre = nest.Create("parrot_neuron", n_pre)
    post = nest.Create("parrot_neuron", n_post)
    third = nest.Create("parrot_neuron", n_third)

    nest.TripartiteConnect(
        pre, post, third, {"rule": "tripartite_bernoulli_with_pool", "pool_type": "block", "pool_size": 4}
    )

    n_primary = n_pre * n_post
    assert len(nest.GetConnections(pre, post)) == n_primary
    assert len(nest.GetConnections(pre, third)) == n_primary
    assert len(nest.GetConnections(third, post)) == n_primary
