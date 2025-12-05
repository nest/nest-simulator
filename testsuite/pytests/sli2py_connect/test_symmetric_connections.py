# -*- coding: utf-8 -*-
#
# test_symmetric_connections.py
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
Name: testsuite::test_symmetric_connections - Tests that the functionality to create
symmetric connections works properly.

Synopsis: (test_symmetric_connections) run -> NEST exits if test fails

Description:
This test ensures that the functionality to create symmetric connections
via flag "make_symmetric" works properly. It also ensures that the
built-in property "requires_symmetric" for synapse models works properly.

In more detail the test ensures that
- the "make_symmetric" flag works properly with one-to-one connection rule
- the usage of the "make_symmetric" flag with any other connections throws
  an NotImplemented exception
- synapse models that "require_symmetric" cannot be created without
  "make_symmetric" except for suitable uniform all-to-all connections

Test ported from SLI regression test.

Author: Jan Hahne, 2016-04-22, updated 2016-11-02
"""

import nest
import pytest


@pytest.fixture
def setup_nodes():
    """Create two sets of neurons for testing."""
    nest.ResetKernel()
    set1 = nest.Create("iaf_psc_alpha", 5)
    set2 = nest.Create("iaf_psc_alpha", 5)
    return set1, set2


def test_make_symmetric_fails_with_all_to_all(setup_nodes):
    """Check that make_symmetric flag cannot be used with all_to_all rule."""
    set1, set2 = setup_nodes
    with pytest.raises(nest.kernel.NESTError, match="NotImplemented"):
        nest.Connect(
            set1,
            set2,
            conn_spec={"rule": "all_to_all", "make_symmetric": True},
            syn_spec={"synapse_model": "static_synapse"},
        )


def test_make_symmetric_fails_with_fixed_indegree(setup_nodes):
    """Check that make_symmetric flag cannot be used with fixed_indegree rule."""
    set1, set2 = setup_nodes
    with pytest.raises(nest.kernel.NESTError, match="NotImplemented"):
        nest.Connect(
            set1,
            set2,
            conn_spec={"rule": "fixed_indegree", "indegree": 3, "make_symmetric": True},
            syn_spec={"synapse_model": "static_synapse"},
        )


def test_make_symmetric_fails_with_fixed_outdegree(setup_nodes):
    """Check that make_symmetric flag cannot be used with fixed_outdegree rule."""
    set1, set2 = setup_nodes
    with pytest.raises(nest.kernel.NESTError, match="NotImplemented"):
        nest.Connect(
            set1,
            set2,
            conn_spec={"rule": "fixed_outdegree", "outdegree": 3, "make_symmetric": True},
            syn_spec={"synapse_model": "static_synapse"},
        )


def test_make_symmetric_fails_with_fixed_total_number(setup_nodes):
    """Check that make_symmetric flag cannot be used with fixed_total_number rule."""
    set1, set2 = setup_nodes
    with pytest.raises(nest.kernel.NESTError, match="NotImplemented"):
        nest.Connect(
            set1,
            set2,
            conn_spec={"rule": "fixed_total_number", "N": 3, "make_symmetric": True},
            syn_spec={"synapse_model": "static_synapse"},
        )


def test_make_symmetric_works_one_to_one(setup_nodes):
    """Check that make_symmetric flag works properly for basic one_to_one connections."""
    set1, set2 = setup_nodes
    nest.Connect(
        set1,
        set2,
        conn_spec={"rule": "one_to_one", "make_symmetric": True},
        syn_spec={"synapse_model": "static_synapse"},
    )

    # Get forward and backward connections
    fwd = nest.GetConnections(source=set1)
    bck = nest.GetConnections(source=set2)

    # Check that connection counts match (ensures no extra connections)
    assert len(fwd) == len(bck), f"Forward and backward connection counts must match: {len(fwd)} != {len(bck)}"

    # Check that connections are symmetric
    fwd_sources = fwd.get("source")
    fwd_targets = fwd.get("target")
    fwd_weights = fwd.get("weight")
    fwd_delays = fwd.get("delay")

    bck_sources = bck.get("source")
    bck_targets = bck.get("target")
    bck_weights = bck.get("weight")
    bck_delays = bck.get("delay")

    # For each forward connection, check there's a matching backward connection
    for i in range(len(fwd)):
        # Find matching backward connection
        match_idx = None
        for j in range(len(bck)):
            if bck_sources[j] == fwd_targets[i] and bck_targets[j] == fwd_sources[i]:
                match_idx = j
                break

        assert match_idx is not None, f"No matching backward connection for forward connection {i}"
        assert fwd_weights[i] == bck_weights[match_idx]
        assert fwd_delays[i] == bck_delays[match_idx]


def test_make_symmetric_fails_with_random_parameters_weight(setup_nodes):
    """Check that make_symmetric flag cannot be used with random weight parameters."""
    set1, set2 = setup_nodes
    with pytest.raises(nest.kernel.NESTError, match="NotImplemented|BadProperty"):
        nest.Connect(
            set1,
            set2,
            conn_spec={"rule": "one_to_one", "make_symmetric": True},
            syn_spec={
                "synapse_model": "stdp_synapse",
                "weight": nest.random.uniform(),
            },
        )


def test_make_symmetric_fails_with_random_parameters_alpha(setup_nodes):
    """Check that make_symmetric flag cannot be used with random alpha parameters."""
    set1, set2 = setup_nodes
    with pytest.raises(nest.kernel.NESTError, match="NotImplemented|BadProperty"):
        nest.Connect(
            set1,
            set2,
            conn_spec={"rule": "one_to_one", "make_symmetric": True},
            syn_spec={
                "synapse_model": "stdp_synapse",
                "alpha": nest.random.uniform(),
            },
        )


def test_make_symmetric_works_with_array_parameters(setup_nodes):
    """Check that make_symmetric flag works properly for one_to_one connections with array parameters."""
    set1, set2 = setup_nodes
    nest.Connect(
        set1,
        set2,
        conn_spec={"rule": "one_to_one", "make_symmetric": True},
        syn_spec={
            "synapse_model": "stdp_synapse",
            "weight": [1.0, 2.0, 3.0, 4.0, 5.0],
            "delay": [1.5, 2.5, 3.5, 4.5, 5.5],
            "alpha": [1.2, 2.2, 3.2, 4.2, 5.2],
        },
    )

    # Get forward and backward connections
    fwd = nest.GetConnections(source=set1)
    bck = nest.GetConnections(source=set2)

    # Check that connection counts match (ensures no extra connections)
    assert len(fwd) == len(bck), f"Forward and backward connection counts must match: {len(fwd)} != {len(bck)}"

    # Check that connections are symmetric
    fwd_sources = fwd.get("source")
    fwd_targets = fwd.get("target")
    fwd_weights = fwd.get("weight")
    fwd_delays = fwd.get("delay")
    fwd_alphas = fwd.get("alpha")

    bck_sources = bck.get("source")
    bck_targets = bck.get("target")
    bck_weights = bck.get("weight")
    bck_delays = bck.get("delay")
    bck_alphas = bck.get("alpha")

    # For each forward connection, check there's a matching backward connection
    for i in range(len(fwd)):
        # Find matching backward connection
        match_idx = None
        for j in range(len(bck)):
            if bck_sources[j] == fwd_targets[i] and bck_targets[j] == fwd_sources[i]:
                match_idx = j
                break

        assert match_idx is not None, f"No matching backward connection for forward connection {i}"
        assert fwd_weights[i] == bck_weights[match_idx]
        assert fwd_delays[i] == bck_delays[match_idx]
        assert fwd_alphas[i] == bck_alphas[match_idx]


@pytest.mark.skipif_without_gsl
def test_requires_symmetric_fails_without_make_symmetric_one_to_one():
    """Check that connections requiring symmetric cannot be created without make_symmetric=true for one_to_one."""
    nest.ResetKernel()
    set1 = nest.Create("hh_psc_alpha_gap", 2)
    set2 = nest.Create("hh_psc_alpha_gap", 2)
    with pytest.raises(nest.kernel.NESTError, match="BadProperty|requires_symmetric"):
        nest.Connect(
            set1,
            set2,
            conn_spec={"rule": "one_to_one", "make_symmetric": False},
            syn_spec={"synapse_model": "gap_junction"},
        )


@pytest.mark.skipif_without_gsl
def test_requires_symmetric_fails_without_make_symmetric_fixed_indegree():
    """Check that connections requiring symmetric cannot be created without make_symmetric for fixed_indegree."""
    nest.ResetKernel()
    set1 = nest.Create("hh_psc_alpha_gap", 2)
    set2 = nest.Create("hh_psc_alpha_gap", 2)
    with pytest.raises(nest.kernel.NESTError, match="BadProperty|requires_symmetric"):
        nest.Connect(
            set1,
            set2,
            conn_spec={"rule": "fixed_indegree", "indegree": 3, "make_symmetric": False},
            syn_spec={"synapse_model": "gap_junction"},
        )


@pytest.mark.skipif_without_gsl
def test_requires_symmetric_fails_without_make_symmetric_fixed_outdegree():
    """Check that connections requiring symmetric cannot be created without make_symmetric for fixed_outdegree."""
    nest.ResetKernel()
    set1 = nest.Create("hh_psc_alpha_gap", 2)
    set2 = nest.Create("hh_psc_alpha_gap", 2)
    with pytest.raises(nest.kernel.NESTError, match="BadProperty|requires_symmetric"):
        nest.Connect(
            set1,
            set2,
            conn_spec={"rule": "fixed_outdegree", "outdegree": 3, "make_symmetric": False},
            syn_spec={"synapse_model": "gap_junction"},
        )


@pytest.mark.skipif_without_gsl
def test_requires_symmetric_fails_without_make_symmetric_fixed_total_number():
    """Check that connections requiring symmetric cannot be created without make_symmetric for fixed_total_number."""
    nest.ResetKernel()
    set1 = nest.Create("hh_psc_alpha_gap", 2)
    set2 = nest.Create("hh_psc_alpha_gap", 2)
    with pytest.raises(nest.kernel.NESTError, match="BadProperty|requires_symmetric"):
        nest.Connect(
            set1,
            set2,
            conn_spec={"rule": "fixed_total_number", "N": 3, "make_symmetric": False},
            syn_spec={"synapse_model": "gap_junction"},
        )


@pytest.mark.skipif_without_gsl
def test_requires_symmetric_fails_all_to_all_different_sources_targets():
    """Check that all-to-all connection can only be created with sources == targets."""
    nest.ResetKernel()
    set1 = nest.Create("hh_psc_alpha_gap", 2)
    set2 = nest.Create("hh_psc_alpha_gap", 2)
    with pytest.raises(nest.kernel.NESTError, match="BadProperty|requires_symmetric"):
        nest.Connect(
            set1,
            set2,
            conn_spec={"rule": "all_to_all", "make_symmetric": False},
            syn_spec={"synapse_model": "gap_junction"},
        )


@pytest.mark.skipif_without_gsl
def test_requires_symmetric_fails_all_to_all_array_weights():
    """Check that all-to-all connection fails with array weights."""
    nest.ResetKernel()
    set1 = nest.Create("hh_psc_alpha_gap", 2)
    with pytest.raises(
        nest.kernel.NESTError, match="BadProperty|requires_symmetric|wrong type|One-dimensional parameter arrays"
    ):
        nest.Connect(
            set1,
            set1,
            conn_spec={"rule": "all_to_all", "make_symmetric": False},
            syn_spec={"synapse_model": "gap_junction", "weight": [1.0, 2.0, 3.0, 4.0]},
        )


@pytest.mark.skipif_without_gsl
def test_requires_symmetric_fails_all_to_all_random_weights():
    """Check that all-to-all connection fails with random weights."""
    nest.ResetKernel()
    set1 = nest.Create("hh_psc_alpha_gap", 2)
    with pytest.raises(nest.kernel.NESTError, match="BadProperty|requires_symmetric"):
        nest.Connect(
            set1,
            set1,
            conn_spec={"rule": "all_to_all", "make_symmetric": False},
            syn_spec={"synapse_model": "gap_junction", "weight": nest.random.uniform()},
        )


@pytest.mark.skipif_without_gsl
def test_requires_symmetric_passes_all_to_all_scalar():
    """Check that all-to-all connection works with scalar weights when sources == targets."""
    nest.ResetKernel()
    set1 = nest.Create("hh_psc_alpha_gap", 2)
    nest.Connect(
        set1,
        set1,
        conn_spec={"rule": "all_to_all", "make_symmetric": False},
        syn_spec={"synapse_model": "gap_junction"},
    )
    # Test passes if no exception is raised


@pytest.mark.skipif_without_gsl
def test_requires_symmetric_passes_all_to_all_scalar_weight():
    """Check that all-to-all connection works with scalar weight when sources == targets."""
    nest.ResetKernel()
    set1 = nest.Create("hh_psc_alpha_gap", 2)
    nest.Connect(
        set1,
        set1,
        conn_spec={"rule": "all_to_all", "make_symmetric": False},
        syn_spec={"synapse_model": "gap_junction", "weight": 2.0},
    )
    # Test passes if no exception is raised
