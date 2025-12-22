# -*- coding: utf-8 -*-
#
# test_node_collection_set.py
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
Test ``NodeCollection`` setter.
"""

import nest
import numpy.testing as nptest
import pytest


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


def test_node_collection_set_single_param():
    """Test ``set`` with a single parameter."""

    nodes = nest.Create("iaf_psc_alpha", 10)
    nodes.set(tau_Ca=500.0)

    nptest.assert_equal(nodes.tau_Ca, 500.0)


def test_node_collection_set_list_of_single_param():
    """Test ``set`` with list of a single parameter."""

    nodes = nest.Create("iaf_psc_alpha", 10)
    Vreset_ref = [-85.0, -82.0, -80.0, -77.0, -75.0, -72.0, -70.0, -67.0, -65.0, -62.0]
    nodes.set(V_reset=Vreset_ref)

    nptest.assert_array_equal(nodes.V_reset, Vreset_ref)


def test_node_collection_set_list_of_single_param_wrong_length_raises():
    """Test that ``set`` with list of wrong length raises exception."""

    nodes = nest.Create("iaf_psc_alpha", 10)

    with pytest.raises(IndexError):
        nodes.set(V_reset=[-85.0, -82.0, -80.0, -77.0, -75.0])


def test_node_collection_set_dict_single_param():
    """Test ``set`` with dictionary containing a single parameter."""

    nodes = nest.Create("iaf_psc_alpha", 10)
    nodes.set({"C_m": 100.0})

    nptest.assert_equal(nodes.C_m, 100.0)


def test_node_collection_set_list_of_dicts():
    """Test ``set`` with dictionary containing a single parameter."""

    nodes = nest.Create("iaf_psc_alpha", 10)
    nodes.set(
        (
            {"V_m": 10.0},
            {"V_m": 20.0},
            {"V_m": 30.0},
            {"V_m": 40.0},
            {"V_m": 50.0},
            {"V_m": 60.0},
            {"V_m": 70.0},
            {"V_m": 80.0},
            {"V_m": 90.0},
            {"V_m": -100.0},
        )
    )
    nptest.assert_array_equal(nodes.V_m, [10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0, 90.0, -100.0])


def test_node_collection_set_dict_multiple_params():
    """Test ``set`` with dictionary containing multiple parameters."""

    nodes = nest.Create("iaf_psc_alpha", 10)
    nodes.set({"t_ref": 44.0, "tau_m": 2.0, "tau_minus": 42.0})

    nptest.assert_equal(nodes.t_ref, 44.0)
    nptest.assert_equal(nodes.tau_m, 2.0)
    nptest.assert_equal(nodes.tau_minus, 42.0)


def test_node_collection_set_dict_with_lists():
    """Test ``set`` with dictionary containing multiple parameter lists."""

    nodes = nest.Create("iaf_psc_alpha", 3)
    Vm_ref = [-11.0, -12.0, -13.0]
    Cm_ref = 177.0
    tau_minus_ref = [22.0, 24.0, 26.0]
    nodes.set({"V_m": Vm_ref, "C_m": Cm_ref, "tau_minus": tau_minus_ref})

    nptest.assert_array_equal(nodes.V_m, Vm_ref)
    nptest.assert_equal(nodes.C_m, Cm_ref)
    nptest.assert_array_equal(nodes.tau_minus, tau_minus_ref)


def test_node_collection_set_dict_with_single_element_lists():
    """Test ``set`` with dictionary containing single element parameter lists."""

    node = nest.Create("iaf_psc_alpha")
    Vm_ref = -13.0
    Cm_ref = 222.0
    node.set({"V_m": [Vm_ref], "C_m": [Cm_ref]})

    assert node.V_m == Vm_ref
    assert node.C_m == Cm_ref


def test_node_collection_set_dict_bool_list():
    """Test ``set`` with dictionary containing single element parameter lists."""

    srecs = nest.Create("spike_recorder", 3)
    time_in_steps_ref = [True, False, True]
    srecs.set({"time_in_steps": time_in_steps_ref})

    nptest.assert_array_equal(srecs.time_in_steps, time_in_steps_ref)


def test_create_with_params_list_that_should_not_be_split():
    """Test ``Create`` with list that should not be split."""

    spikes_ref = [1.0, 2.0, 30.0]
    sg = nest.Create("spike_generator", 2, {"spike_times": spikes_ref})

    spikes_sg1 = sg[0].spike_times
    spikes_sg2 = sg[1].spike_times

    nptest.assert_array_equal(spikes_sg1, spikes_ref)
    nptest.assert_array_equal(spikes_sg2, spikes_ref)


def test_set_on_spike_generator():
    """Test ``set`` with dict with list that should not be split"""

    sgens = nest.Create("spike_generator", 2)
    spikes_ref = [1.0, 2.0, 3.0]
    sgens.set({"spike_times": spikes_ref})

    spikes_sg1 = sgens[0].spike_times
    spikes_sg2 = sgens[1].spike_times

    nptest.assert_array_equal(spikes_sg1, spikes_ref)
    nptest.assert_array_equal(spikes_sg2, spikes_ref)


def test_node_collection_set_nonexistent_param_raises():
    """Test that ``set`` with dictionary containing non-existent parameter raises exception."""

    nodes = nest.Create("iaf_psc_alpha", 10)

    with pytest.raises(nest.NESTErrors.UnaccessedDictionaryEntry):
        nodes.set({"vp": 2})


@pytest.mark.parametrize("empty_nc", [nest.NodeCollection(), nest.NodeCollection([])])
def test_set_on_empty_node_collection(empty_nc):
    """Ensure that ``set`` on an empty ``NodeCollection`` does not raise an error."""

    empty_nc.set()


def test_sliced_node_collection_set():
    """Test ``set`` on sliced ``NodeCollection``."""

    nodes = nest.Create("iaf_psc_alpha", 10)

    nodes[2:5].set(({"V_m": -50.0}, {"V_m": -40.0}, {"V_m": -30.0}))
    nodes[5:7].set({"t_ref": 4.4, "tau_m": 3.0})
    nodes[2:9:2].set(C_m=111.0)

    Vm_ref = [-70.0, -70.0, -50.0, -40.0, -30.0, -70.0, -70.0, -70.0, -70.0, -70.0]
    Cm_ref = [250.0, 250.0, 111.0, 250.0, 111.0, 250.0, 111.0, 250.0, 111.0, 250.0]
    status_dict_ref = {
        "t_ref": [2.0, 2.0, 2.0, 2.0, 2.0, 4.4, 4.4, 2.0, 2.0, 2.0],
        "tau_m": [10.0, 10.0, 10.0, 10.0, 10.0, 3.00, 3.00, 10.0, 10.0, 10.0],
    }

    actual_status_dict = nodes.get(["t_ref", "tau_m"])

    nptest.assert_array_equal(nodes.V_m, Vm_ref)
    nptest.assert_array_equal(nodes.C_m, Cm_ref)
    nptest.assert_equal(actual_status_dict, status_dict_ref)


def test_node_collection_set_attribute():
    """Test the ``__setattr__`` method."""

    nodes = nest.Create("iaf_psc_alpha", 10)
    V_reset_ref = [-85.0, -82.0, -80.0, -77.0, -75.0, -72.0, -70.0, -67.0, -65.0, -62.0]

    nodes.C_m = 100.0
    nodes.V_reset = V_reset_ref

    nptest.assert_equal(nodes.C_m, 100.0)
    nptest.assert_array_equal(nodes.V_reset, V_reset_ref)


def test_node_collection_set_attribute_list_wrong_length_raises():
    """Test that setting attribute to list of wrong length raises exception."""

    nodes = nest.Create("iaf_psc_alpha", 10)

    with pytest.raises(IndexError):
        nodes.V_reset = [-85.0, -82.0, -80.0, -77.0, -75.0]


def test_node_collection_set_nonexistent_attribute_raises():
    """Test that setting non-existent attribute raises exception."""

    nodes = nest.Create("iaf_psc_alpha", 10)

    with pytest.raises(nest.NESTErrors.UnaccessedDictionaryEntry):
        nodes.nonexistent_attribute = 1.0
