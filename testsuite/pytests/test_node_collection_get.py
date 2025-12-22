# -*- coding: utf-8 -*-
#
# test_node_collection_get.py
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
Test ``NodeCollection`` getter.
"""

import json

import nest
import numpy as np
import numpy.testing as nptest
import pandas as pd
import pandas.testing as pdtest
import pytest


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


@pytest.mark.parametrize(
    "neuron_param, expected_value",
    [
        ["C_m", 250.0],
        ["E_L", -70.0],
        ["V_m", -70.0],
        ["t_ref", 2.0],
    ],
)
def test_node_collection_get_neuron_params(neuron_param, expected_value):
    """Test ``get`` on neuron parameters."""

    nodes = nest.Create("iaf_psc_alpha", 10)
    nptest.assert_equal(nodes.get(neuron_param), expected_value)


def test_node_collection_get_node_ids():
    """Test ``get`` on node IDs."""

    nodes = nest.Create("iaf_psc_alpha", 10)
    nptest.assert_array_equal(nodes.get("global_id"), list(range(1, 11)))


def test_node_collection_get_multiple_params():
    """Test ``get`` on multiple parameters."""

    num_nodes = 10
    nodes = nest.Create("iaf_psc_alpha", num_nodes)

    g = nodes.get(["local", "thread", "vp"])

    g_reference = {
        "local": np.full((num_nodes), True),
        "thread": np.zeros(num_nodes, dtype=int),
        "vp": np.zeros(num_nodes, dtype=int),
    }

    nptest.assert_equal(g["local"], True)
    nptest.assert_equal(g["thread"], 0)
    nptest.assert_equal(g["vp"], 0)
    nptest.assert_equal(g, g_reference)


@pytest.mark.parametrize(
    "neuron_param, expected_value",
    [
        ["C_m", 250.0],
        ["E_L", -70.0],
        ["V_m", -70.0],
        ["t_ref", 2.0],
    ],
)
def test_node_collection_get_attribute(neuron_param, expected_value):
    """Test the ``__getattr__`` method."""

    nodes = nest.Create("iaf_psc_alpha", 10)
    nptest.assert_equal(getattr(nodes, neuron_param), expected_value)


def test_node_collection_get_nonexistent_attribute_raises():
    """Ensure that getting a non-existent attribute raises exception."""

    nodes = nest.Create("iaf_psc_alpha", 10)

    with pytest.raises(KeyError):
        nodes.nonexistent_attribute


def test_node_collection_get_empty_attribute():
    """Ensure that getting an empty attribute does not raise exception."""

    nodes = nest.Create("iaf_psc_alpha", 10)

    assert nodes.spatial is None


def test_node_collection_get_spatial_attributes():
    """Ensure that getting an empty attribute does not raise exception."""

    spatial_node = nest.Create("iaf_psc_alpha", positions=nest.spatial.grid([2, 2]))

    spatial_reference = {
        "network_size": 4,
        "center": [0.0, 0.0],
        "edge_wrap": False,
        "extent": [1.0, 1.0],
        "shape": [2, 2],
    }

    nptest.assert_equal(spatial_node.spatial, spatial_reference)


def test_sliced_node_collection_get():
    """Test ``get`` on sliced ``NodeCollection``."""

    nodes = nest.Create("iaf_psc_alpha", 10)

    V_m = nodes[2:5].get("V_m")
    g = nodes[5:7].get(["t_ref", "tau_m"])
    C_m = nodes[2:9:2].get("C_m")

    nptest.assert_array_equal(V_m, [-70.0, -70.0, -70.0])
    nptest.assert_array_equal(g["t_ref"], [2.0, 2.0])
    nptest.assert_array_equal(C_m, [250.0, 250.0, 250.0, 250.0])


def test_composite_node_collection_get():
    """Test ``get`` on composite ``NodeCollection``."""

    n1 = nest.Create("iaf_psc_alpha", 2)
    n2 = nest.Create("iaf_psc_delta", 2)
    n3 = nest.Create("iaf_psc_exp")
    n4 = nest.Create("iaf_psc_alpha", 3)

    n1.set(V_m=[-77.0, -88.0])
    n3.set({"V_m": -55.0})

    n1.set(C_m=[251.0, 252.0])
    n2.set(C_m=[253.0, 254.0])
    n3.set({"C_m": 255.0})
    n4.set(C_m=[256.0, 257.0, 258.0])

    n5 = n1 + n2 + n3 + n4

    status_dict = n5.get()

    # Check that we get values in correct order
    vm_ref = [-77.0, -88.0, -70.0, -70.0, -55, -70.0, -70.0, -70.0]
    nptest.assert_array_equal(status_dict["V_m"], vm_ref)

    # Check that we get None where not applicable
    # tau_syn_ex is part of iaf_psc_alpha
    tau_ref = [2.0, 2.0, None, None, 2.0, 2.0, 2.0, 2.0]
    nptest.assert_array_equal(status_dict["tau_syn_ex"], tau_ref)

    # refractory_input is part of iaf_psc_delta
    refrac_ref = [None, None, False, False, None, None, None, None]
    nptest.assert_array_equal(status_dict["refractory_input"], refrac_ref)

    # Check that calling get with string works on composite NCs, both on
    # parameters all the models have, and on individual parameters.
    Cm_ref = [x * 1.0 for x in range(251, 259)]
    Cm_actual = n5.get("C_m")
    nptest.assert_array_equal(Cm_actual, Cm_ref)

    refrac_actual = n5.get("refractory_input")
    nptest.assert_array_equal(refrac_actual, refrac_ref)


def test_different_sized_node_collections_get():
    """
    Test ``get`` with different input for different sizes of ``NodeCollection``s.
    """

    single_sr = nest.Create("spike_recorder", 1)
    multi_sr = nest.Create("spike_recorder", 10)
    empty_array_float = np.array([], dtype=float)
    empty_array_int = np.array([], dtype=int)

    # Single node, literal parameter
    assert single_sr.get("start") == 0.0

    # Single node, array parameter
    assert single_sr.get(["start", "time_in_steps"]) == {"start": 0.0, "time_in_steps": False}

    # Single node, hierarchical with literal parameter
    nptest.assert_array_equal(single_sr.get("events", "times"), empty_array_float)

    # Multiple nodes, hierarchical with literal parameter
    for spike_times in multi_sr.get("events", "times"):
        nptest.assert_array_equal(spike_times, empty_array_float)

    # Single node, hierarchical with array parameter
    single_events_dict = single_sr.get("events", ["senders", "times"])
    assert len(single_events_dict) == 2
    assert "senders" in single_events_dict
    assert "times" in single_events_dict
    nptest.assert_array_equal(single_events_dict["senders"], empty_array_int)
    nptest.assert_array_equal(single_events_dict["times"], empty_array_float)

    # Multiple nodes, hierarchical with array parameter
    multi_events_dict = single_sr.get("events", ["senders", "times"])
    assert len(multi_events_dict) == 2
    assert "senders" in multi_events_dict
    assert "times" in multi_events_dict
    for sender in multi_events_dict["senders"]:
        nptest.assert_array_equal(sender, empty_array_int)
    for spike_times in multi_events_dict["times"]:
        nptest.assert_array_equal(spike_times, empty_array_float)

    # Single node, no parameter (gets all values)
    single_full_dict = single_sr.get()
    assert single_full_dict["start"] == 0.0

    # Multiple nodes, no parameter (gets all values)
    multi_full_dict = multi_sr.get()
    nptest.assert_equal(multi_full_dict["start"], 0.0)

    # Ensure that single and multiple gets have the same number of params
    single_num_params = len(single_full_dict.keys())
    multi_num_params = len(multi_full_dict.keys())
    assert multi_num_params == single_num_params


def test_node_collection_get_pandas_output():
    """Test ``NodeCollection`` ``get`` with ``output=pandas``."""

    single_sr = nest.Create("spike_recorder", 1)
    multi_sr = nest.Create("spike_recorder", 10)
    empty_array_float = np.array([], dtype=float)

    # Single node, literal parameter
    pdtest.assert_frame_equal(
        single_sr.get("start", output="pandas"), pd.DataFrame({"start": [0.0]}, index=tuple(single_sr.tolist()))
    )

    # Multiple nodes, literal parameter
    pdtest.assert_frame_equal(
        multi_sr.get("start", output="pandas"),
        pd.DataFrame({"start": [0.0 for i in range(len(multi_sr))]}, index=tuple(multi_sr.tolist())),
    )

    # Single node, array parameter
    pdtest.assert_frame_equal(
        single_sr.get(["start", "n_events"], output="pandas"),
        pd.DataFrame({"start": [0.0], "n_events": [0]}, index=tuple(single_sr.tolist())),
    )

    # Multiple nodes, array parameter
    ref_dict = {"start": [0.0 for i in range(len(multi_sr))], "n_events": [0]}
    pdtest.assert_frame_equal(
        multi_sr.get(["start", "n_events"], output="pandas"),
        pd.DataFrame(ref_dict, index=tuple(multi_sr.tolist())),
    )

    # Single node, hierarchical with literal parameter
    pdtest.assert_frame_equal(
        single_sr.get("events", "times", output="pandas"),
        pd.DataFrame({"times": [[]]}, index=tuple(single_sr.tolist())),
    )

    # Multiple nodes, hierarchical with literal parameter
    ref_dict = {"times": [empty_array_float for i in range(len(multi_sr))]}
    pdtest.assert_frame_equal(
        multi_sr.get("events", "times", output="pandas"), pd.DataFrame(ref_dict, index=tuple(multi_sr.tolist()))
    )

    # Single node, hierarchical with array parameter
    ref_df = pd.DataFrame({"times": [[]], "senders": [[]]}, index=tuple(single_sr.tolist()))
    ref_df = ref_df.reindex(sorted(ref_df.columns), axis=1)
    pdtest.assert_frame_equal(single_sr.get("events", ["senders", "times"], output="pandas"), ref_df)

    # Multiple nodes, hierarchical with array parameter
    ref_dict = {"times": [[] for i in range(len(multi_sr))], "senders": [[] for i in range(len(multi_sr))]}
    ref_df = pd.DataFrame(ref_dict, index=tuple(multi_sr.tolist()))
    ref_df = ref_df.reindex(sorted(ref_df.columns), axis=1)
    sr_df = multi_sr.get("events", ["senders", "times"], output="pandas")
    sr_df = sr_df.reindex(sorted(sr_df.columns), axis=1)
    pdtest.assert_frame_equal(sr_df, ref_df)

    # Single node, no parameter (gets all values)
    values = single_sr.get(output="pandas")
    num_values_single_sr = values.shape[1]
    nptest.assert_equal(values["start"][single_sr.tolist()[0]], 0.0)

    # Multiple nodes, no parameter (gets all values)
    values = multi_sr.get(output="pandas")

    assert values.shape == (len(multi_sr), num_values_single_sr)
    pdtest.assert_series_equal(
        values["start"], pd.Series({key: 0.0 for key in tuple(multi_sr.tolist())}, dtype=float, name="start")
    )

    # With data in events
    nodes = nest.Create("iaf_psc_alpha", 10)
    pg = nest.Create("poisson_generator", params={"rate": 70000.0})
    nest.Connect(pg, nodes)
    nest.Connect(nodes, single_sr)
    nest.Connect(nodes, multi_sr, "one_to_one")
    nest.Simulate(50)

    ref_values = single_sr.get("events", ["senders", "times"])
    ref_df = pd.DataFrame({key: [ref_values[key]] for key in ["senders", "times"]}, index=tuple(single_sr.tolist()))
    sd_df = single_sr.get("events", ["senders", "times"], output="pandas")
    pdtest.assert_frame_equal(sd_df, ref_df)

    ref_values = multi_sr.get("events", ["senders", "times"])
    ref_df = pd.DataFrame(ref_values, index=tuple(multi_sr.tolist()))
    sd_df = multi_sr.get("events", ["senders", "times"], output="pandas")
    pdtest.assert_frame_equal(sd_df, ref_df)


def test_node_collection_get_json_output():
    """Test ``NodeCollection`` ``get`` with ``output=json``."""

    single_sr = nest.Create("spike_recorder", 1)
    multi_sr = nest.Create("spike_recorder", 10)

    # Single node, literal parameter
    assert json.loads(single_sr.get("start", output="json")) == 0.0

    # Multiple nodes, literal parameter
    nptest.assert_equal(json.loads(multi_sr.get("start", output="json")), len(multi_sr) * [0.0])

    # Single node, array parameter
    ref_dict = {"start": 0.0, "n_events": 0}
    assert json.loads(single_sr.get(["start", "n_events"], output="json")) == ref_dict

    # Multiple nodes, array parameter
    ref_dict = {"start": len(multi_sr) * [0.0], "n_events": len(multi_sr) * [0]}
    assert json.loads(multi_sr.get(["start", "n_events"], output="json")) == ref_dict

    # Single node, hierarchical with literal parameter
    assert json.loads(single_sr.get("events", "times", output="json")) == []

    # Multiple nodes, hierarchical with literal parameter
    ref_list = len(multi_sr) * [[]]
    assert json.loads(multi_sr.get("events", "times", output="json")) == ref_list

    # Single node, hierarchical with array parameter
    ref_dict = {"senders": [], "times": []}
    assert json.loads(single_sr.get("events", ["senders", "times"], output="json")) == ref_dict

    # Multiple nodes, hierarchical with array parameter
    ref_dict = {"times": len(multi_sr) * [[]], "senders": len(multi_sr) * [[]]}
    assert json.loads(multi_sr.get("events", ["senders", "times"], output="json")) == ref_dict

    # Single node, no parameter (gets all values)
    values = json.loads(single_sr.get(output="json"))
    num_values_single_sr = len(values)
    assert values["start"] == 0.0

    # Multiple nodes, no parameter (gets all values)
    values = json.loads(multi_sr.get(output="json"))
    assert len(values) == num_values_single_sr
    nptest.assert_array_equal(values["start"], len(multi_sr) * [0.0])

    # With data in events
    nodes = nest.Create("iaf_psc_alpha", 10)
    pg = nest.Create("poisson_generator", params={"rate": 70000.0})
    nest.Connect(pg, nodes)
    nest.Connect(nodes, single_sr)
    nest.Connect(nodes, multi_sr, "one_to_one")
    nest.Simulate(50)

    sd_ref = single_sr.get("events", ["senders", "times"])
    sd_json = single_sr.get("events", ["senders", "times"], output="json")
    sd_dict = json.loads(sd_json)
    assert len(sd_dict.keys()) == 2
    assert sorted(sd_dict.keys()) == sorted(sd_ref.keys())
    for key in ["senders", "times"]:
        nptest.assert_array_equal(sd_dict[key], sd_ref[key])

    multi_sr_ref = multi_sr.get("events", ["senders", "times"])
    multi_sr_json = multi_sr.get("events", ["senders", "times"], output="json")
    multi_sr_dict = json.loads(multi_sr_json)
    assert len(multi_sr_dict.keys()) == 2
    assert sorted(multi_sr_dict.keys()) == sorted(multi_sr_ref.keys())

    for key in ["senders", "times"]:
        multi_sr_ref_element = [list(element) for element in multi_sr_ref[key]]
        assert multi_sr_dict[key] == multi_sr_ref_element
