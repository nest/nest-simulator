# -*- coding: utf-8 -*-
#
# test_layer_get_set.py
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
Tests for ``get`` and ``set`` functions for spatial ``NodeCollection``.
"""

import nest
import numpy.testing as nptest
import pytest


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


def test_layer_set_on_instance():
    """Test ``set`` on spatial ``NodeCollection`` instance."""

    layer_shape = [3, 3]
    layer = nest.Create(
        "iaf_psc_alpha",
        positions=nest.spatial.grid(
            shape=layer_shape,
            extent=[2.0, 2.0],
            edge_wrap=True,
        ),
    )

    layer.set(V_m=-50.0)
    expected_V_m = [-50.0] * layer_shape[0] * layer_shape[1]
    nptest.assert_array_equal(layer.V_m, expected_V_m)


def test_layer_set_attribute_on_instance():
    """Test ``set`` on spatial ``NodeCollection`` instance."""

    layer_shape = [3, 3]
    layer = nest.Create(
        "iaf_psc_alpha",
        positions=nest.spatial.grid(
            shape=layer_shape,
            extent=[2.0, 2.0],
            edge_wrap=True,
        ),
    )

    layer.V_m = -50.0
    expected_V_m = [-50.0] * layer_shape[0] * layer_shape[1]
    nptest.assert_array_equal(layer.V_m, expected_V_m)


def test_layer_set_nonexistent_param_raises():
    """Test that ``set`` with non-existent parameter raises exception."""

    layer = nest.Create(
        "iaf_psc_alpha",
        positions=nest.spatial.grid(
            shape=[3, 3],
            extent=[2.0, 2.0],
            edge_wrap=True,
        ),
    )

    with pytest.raises(nest.NESTErrors.UnaccessedDictionaryEntry):
        layer.set(center=[1.0, 1.0])

    with pytest.raises(nest.NESTErrors.UnaccessedDictionaryEntry):
        layer.center = [1.0, 1.0]


def test_layer_get_node_param():
    """Test ``get`` on layered ``NodeCollection`` node parameter."""
    layer = nest.Create("iaf_psc_alpha", positions=nest.spatial.grid(shape=[2, 2]))

    nptest.assert_equal(layer.get("V_m"), -70.0)


@pytest.mark.parametrize(
    "spatial_param, expected_value",
    [
        ("center", [0.0, 0.0]),
        ("shape", [3, 3]),
        ("edge_wrap", True),
        ("extent", [2.0, 2.0]),
        ("network_size", 9),
    ],
)
def test_layer_get_spatial_params(spatial_param, expected_value):
    """Test getter on layered ``NodeCollection`` spatial parameters."""

    layer = nest.Create(
        "iaf_psc_alpha",
        positions=nest.spatial.grid(
            shape=[3, 3],
            extent=[2.0, 2.0],
            edge_wrap=True,
        ),
    )
    nptest.assert_equal(layer.spatial[spatial_param], expected_value)


def test_layer_get_all_spatial_params_at_once():
    """Test getting all spatial parameters on layered ``NodeCollection`` at once."""

    layer = nest.Create(
        "iaf_psc_alpha",
        positions=nest.spatial.grid(
            shape=[3, 3],
            extent=[2.0, 2.0],
            edge_wrap=True,
        ),
    )

    all_spatial_params_dict = layer.spatial
    expected_keys = ["center", "edge_wrap", "extent", "network_size", "shape"]

    assert set(all_spatial_params_dict.keys()) == set(expected_keys)


def test_spatial_param_on_single_element_layer():
    """Test spatial parameter on single element layer."""

    layer = nest.Create("iaf_psc_alpha", positions=nest.spatial.grid(shape=[1, 1]))

    assert len(layer) == 1

    center = layer.spatial["center"]
    columns = layer.spatial["shape"][0]
    all_spatial_params = layer.spatial

    nptest.assert_equal(center, [0.0, 0.0])
    assert columns == 1
    nptest.assert_equal(all_spatial_params["center"], [0.0, 0.0])
