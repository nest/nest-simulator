# -*- coding: utf-8 -*-
#
# test_getconnections_multiple_syn_models.py
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
Test ``GetConnections`` in the context of more than one synapse model.

The test constructs a network where the connections are modeled by two
synapse models. A subset of the neurons have one of the synapse models,
and some also have both.
"""

import numpy.testing as nptest
import pandas as pd
import pytest

import nest


@pytest.fixture(scope="module")
def network():
    """
    Fixture for building network.

    Builds the following network:
        - Create 100 neurons
            - 1, 3, ..., 99 are sources
            - 2, 4, ..., 100 are targets
        - Connect 1, 3, ..., 69 -> 2, 4, ..., 70 with `static_synapse`
        - Connect 31, 33, ..., 99   -> 32, 34, ..., 100 with `stdp_synapse`
    """

    nest.Create("iaf_psc_alpha", 100)
    static_sources = nest.NodeCollection(list(range(1, 70, 2)))
    static_targets = nest.NodeCollection(list(range(2, 71, 2)))
    stdp_sources = nest.NodeCollection(list(range(31, 100, 2)))
    stdp_targets = nest.NodeCollection(list(range(32, 101, 2)))

    nest.Connect(static_sources, static_targets, "one_to_one", "static_synapse")
    nest.Connect(stdp_sources, stdp_targets, "one_to_one", "stdp_synapse")

    return {
        "static_synapse": {"sources": static_sources, "targets": static_targets},
        "stdp_synapse": {"sources": stdp_sources, "targets": stdp_targets},
    }


@pytest.mark.parametrize("syn_model", ["static_synapse", "stdp_synapse"])
def test_retrieve_correct_sources_and_targets(network, syn_model):
    """
    Verify that the expected sources and targets are retrieved.
    """

    conns = nest.GetConnections(synapse_model=syn_model)
    actual_sources = conns.get("source")
    actual_targets = conns.get("target")

    expected_sources = network[f"{syn_model}"]["sources"].tolist()
    expected_targets = network[f"{syn_model}"]["targets"].tolist()

    nptest.assert_array_equal(actual_sources, expected_sources)
    nptest.assert_array_equal(actual_targets, expected_targets)


def test_retrieve_all_connections(network):
    """
    Test retrieval of all connections.

    Note that ``static_synapse`` connections will come first.
    """

    conns = nest.GetConnections()

    actual_sources = conns.get("source")
    actual_targets = conns.get("target")

    expected_static_sources = network["static_synapse"]["sources"].tolist()
    expected_stdp_sources = network["stdp_synapse"]["sources"].tolist()
    expected_all_sources = expected_static_sources + expected_stdp_sources

    expected_static_targets = network["static_synapse"]["targets"].tolist()
    expected_stdp_targets = network["stdp_synapse"]["targets"].tolist()
    expected_all_targets = expected_static_targets + expected_stdp_targets

    nptest.assert_array_equal(actual_sources, expected_all_sources)
    nptest.assert_array_equal(actual_targets, expected_all_targets)


@pytest.mark.parametrize()
def test_retrieve_connections_for_some_sources(network):
    """
    Test

    Cannot add NodeCollection to a sliced composite
    """

    # Take first 3 static sources and targets
    src_static = network["static_synapse"]["sources"][:3].tolist()
    tgt_static = network["static_synapse"]["targets"][:3].tolist()

    # Take final 3 stpd sources and targes to avoid those with static+stdp
    src_stdp = network["stdp_synapse"]["sources"][-3:].tolist()
    tgt_stpd = network["stdp_synapse"]["targets"][-3:].tolist()

    src_all = src_static + src_stdp
    # tgt_all = tgt_static + tgt_stpd
    print(src_all)

    conns = nest.GetConnections(source=nest.NodeCollection(src_all))

    actual_sources = conns.get("source")
    print(actual_sources)

    nptest.assert_array_equal(actual_sources, src_all)

    # nptest.assert_array_equal(actual_targets, expected_all_targets)

    """
    /ssrc_static static_sources 3 Take def
    /stgt_static static_targets 3 Take def
    /ssrc_stdp stdp_sources -3 Take def     % take final three to avoid
    /stgt_stdp stdp_targets -3 Take def     % those with static+stdp
    /ssrc_all ssrc_static ssrc_stdp join def
    /stgt_all stgt_static stgt_stdp join def
    /conns << /source ssrc_all cvnodecollection >> GetConnections GetStatus def
    /csrc conns { /source get } Map def
    /ctgt conns { /target get } Map def

    csrc ssrc_all eq
    ctgt stgt_all eq and
    """


def test_retrieve_correct_proportion_of_synapse_model(network):
    """
    Verify that the expected distribution of synapse models is retrieved.

    We expect:
        - 15 connections with `static_synapse` only.
        - 20 connections with `static_synapse` and `stdp_synapse`.
        - 15 connections with `stdp_synapse` only.
    """

    expected_num_static_only = 15
    expected_num_stdp_only = 15
    expected_num_static_stdp = 20

    syn_collection = nest.GetConnections()
    df = pd.DataFrame.from_dict(syn_collection.get())

    # Remove entries with duplicate sources (i.e., no static + stdp connections)
    df_no_dup = df.drop_duplicates(subset=["source"], keep=False)
    actual_num_static_only = len(df_no_dup.loc[df_no_dup["synapse_model"] == "static_synapse"].index)
    actual_num_stdp_only = len(df_no_dup.loc[df_no_dup["synapse_model"] == "stdp_synapse"].index)

    # Obtain entries with duplicate sources only (i.e., only static + stdp connections)
    df_only_dup = df[df.duplicated(subset=["source"], keep=False)]
    # Must divide by 2 since sources are listed twice (once per synapse model)
    actual_num_static_stdp = len(df_only_dup.index) / 2

    assert actual_num_static_only == expected_num_static_only
    assert actual_num_stdp_only == expected_num_stdp_only
    assert actual_num_static_stdp == expected_num_static_stdp


'''
@pytest.mark.parametrize("syn_model", ["static", "stdp"])
def test_retrieve_correct_sources_and_targets(network, syn_model):
    """
    Verify that the expected sources and targets are retrieved.
    """

    expected_sources = network[f"{syn_model}_sources"].tolist()
    expected_targets = network[f"{syn_model}_targets"].tolist()

    syn_collection = nest.GetConnections()
    df = pd.DataFrame.from_dict(syn_collection.get())

    condition = df["synapse_model"] == f"{syn_model}_synapse"
    actual_sources = df["source"].loc[condition].to_numpy()
    actual_targets = df["target"].loc[condition].to_numpy()

    nptest.assert_array_equal(actual_sources, expected_sources)
    nptest.assert_array_equal(actual_targets, expected_targets)
'''
