# -*- coding: utf-8 -*-
#
# test_mask_anchors_boundaries.py
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
import os

import nest
import numpy as np
import pytest

"""
Test masks with varying anchors, boundary conditions and extents.
"""

src_layer_ref = np.array(
    [
        [1.0, -0.5, 0.5],
        [2.0, -0.5, 0.25],
        [3.0, -0.5, 0.0],
        [4.0, -0.5, -0.25],
        [5.0, -0.5, -0.5],
        [6.0, -0.25, 0.5],
        [7.0, -0.25, 0.25],
        [8.0, -0.25, 0.0],
        [9.0, -0.25, -0.25],
        [10.0, -0.25, -0.5],
        [11.0, 0.0, 0.5],
        [12.0, 0.0, 0.25],
        [13.0, 0.0, 0.0],
        [14.0, 0.0, -0.25],
        [15.0, 0.0, -0.5],
        [16.0, 0.25, 0.5],
        [17.0, 0.25, 0.25],
        [18.0, 0.25, 0.0],
        [19.0, 0.25, -0.25],
        [20.0, 0.25, -0.5],
        [21.0, 0.5, 0.5],
        [22.0, 0.5, 0.25],
        [23.0, 0.5, 0.0],
        [24.0, 0.5, -0.25],
        [25.0, 0.5, -0.5],
    ]
)

target_layer_ref = np.array(
    [
        [26.0, -0.5, 0.5],
        [27, -0.5, 0.25],
        [28, -0.5, 0],
        [29, -0.5, -0.25],
        [30, -0.5, -0.5],
        [31, -0.25, 0.5],
        [32, -0.25, 0.25],
        [33, -0.25, 0],
        [34, -0.25, -0.25],
        [35, -0.25, -0.5],
        [36, 0, 0.5],
        [37, 0, 0.25],
        [38, 0, 0],
        [39, 0, -0.25],
        [40, 0, -0.5],
        [41, 0.25, 0.5],
        [42, 0.25, 0.25],
        [43, 0.25, 0],
        [44, 0.25, -0.25],
        [45, 0.25, -0.5],
        [46, 0.5, 0.5],
        [47, 0.5, 0.25],
        [48, 0.5, 0],
        [49, 0.5, -0.25],
        [50, 0.5, -0.5],
    ]
)

home_path = os.path.abspath(os.path.dirname(__file__))


def network(use_free_mask, mask_params, anchor, wrap, extent):
    nest.ResetKernel()
    nest.use_compressed_spikes = False

    if use_free_mask:
        layer = nest.spatial.free(
            [[x, y] for x in np.linspace(-0.5, 0.5, 5) for y in np.linspace(0.5, -0.5, 5)],
            edge_wrap=wrap,
            extent=extent,
        )
    else:
        layer = nest.spatial.grid([5, 5], edge_wrap=wrap, extent=extent)

    population_type = "iaf_psc_alpha"

    mask_type, params = mask_params
    conns = {"rule": "pairwise_bernoulli", "mask": {mask_type: params, "anchor": anchor}}

    src_layer = nest.Create(population_type, positions=layer)
    target_layer = nest.Create(population_type, positions=layer)

    nest.Connect(src_layer, target_layer, conns)
    return src_layer, target_layer


def compare_layers_and_connections(use_free_mask, tmp_path, mask_params, edge_wrap, anchor, extent):
    src_layer, target_layer = network(use_free_mask, mask_params, anchor, edge_wrap, extent)
    path = str(tmp_path) + "tmp.txt"

    # TODO: replace Dump methods by in-memory equivalent once available

    nest.DumpLayerNodes(src_layer, path)
    stored_src = np.genfromtxt(path)

    nest.DumpLayerNodes(target_layer, path)
    stored_target = np.genfromtxt(path)

    nest.DumpLayerConnections(src_layer, target_layer, "static_synapse", path)
    stored_connections = np.genfromtxt(path)

    file_name = f"wrap_{str(edge_wrap)}_anchor_" + "_".join(str(x) for x in anchor)
    directory = mask_params[0]
    reference_path = f'spatial_test_references/{"free" if use_free_mask else "grid"}/{directory}/{file_name}.txt'

    connections_ref = np.loadtxt(os.path.join(home_path, reference_path))

    assert np.all(stored_src == src_layer_ref)
    assert np.all(stored_target == target_layer_ref)

    # The order in which connections are written to file is implementation dependent. Therefore, we need to
    # sort results and expectations here. We use lexsort to sort entire rows of the arrays. We need to
    # transpose the array to provide it properly as keys to lexsort.
    np.testing.assert_equal(
        stored_connections[np.lexsort(stored_connections.T), :], connections_ref[np.lexsort(connections_ref.T), :]
    )


@pytest.mark.parametrize(
    "mask_params", [("doughnut", {"outer_radius": 0.25, "inner_radius": 0.1}), ("circular", {"radius": 0.25})]
)
@pytest.mark.parametrize("anchor", [[0.0, 0.0], [-0.25, 0.0]])
@pytest.mark.parametrize("edge_wrap", [True, False])
def test_free_circ_doughnut_anchor_and_boundary(tmp_path, mask_params, anchor, edge_wrap):
    extent = [1.25, 1.25]
    use_free_mask = True

    compare_layers_and_connections(use_free_mask, tmp_path, mask_params, edge_wrap, anchor, extent)


@pytest.mark.parametrize("use_free_mask", [True, False])
@pytest.mark.parametrize("anchor", [[0.0, 0.0], [-0.5, -0.25]])
def test_free_and_grid_rect_anchor(tmp_path, anchor, use_free_mask):
    mask_params = ("rectangular", {"lower_left": [0.0, 0.0], "upper_right": [0.6, 0.3]})
    edge_wrap = False
    extent = [1.25, 1.25]

    compare_layers_and_connections(use_free_mask, tmp_path, mask_params, edge_wrap, anchor, extent)


def test_free_rect_anchor_with_offset(tmp_path):
    edge_wrap = True
    mask_params = ("rectangular", {"lower_left": [0.0, 0.0], "upper_right": [0.6, 0.3]})
    anchor = [-0.5, -0.25]
    extent = [1.25, 1.25]
    use_free_mask = True

    compare_layers_and_connections(use_free_mask, tmp_path, mask_params, edge_wrap, anchor, extent)


def test_free_rect_offset_with_edge_wrap(tmp_path):
    edge_wrap = True
    mask_params = ("rectangular", {"lower_left": [-0.001, -0.001], "upper_right": [0.6, 0.301]})
    anchor = [0.0, 0.0]
    extent = [1.05, 1.05]
    use_free_mask = True

    compare_layers_and_connections(use_free_mask, tmp_path, mask_params, edge_wrap, anchor, extent)


@pytest.mark.parametrize(
    "mask_params", [("doughnut", {"outer_radius": 0.25, "inner_radius": 0.1}), ("circular", {"radius": 0.25})]
)
@pytest.mark.parametrize("anchor", [[0.0, 0.0], [-0.25, 0.0]])
@pytest.mark.parametrize("edge_wrap", [True, False])
def test_reg_circ_doughnut_anchor_and_boundary(tmp_path, mask_params, anchor, edge_wrap):
    extent = [1.25, 1.25]
    use_free_mask = False

    compare_layers_and_connections(use_free_mask, tmp_path, mask_params, edge_wrap, anchor, extent)


@pytest.mark.parametrize("anchor", [[0, 0], [0, 1], [1, 0], [5, 5]])
@pytest.mark.parametrize("edge_wrap", [True, False])
def test_reg_grid_anchor(tmp_path, anchor, edge_wrap):
    mask_params = ("grid", {"shape": [3, 2]})
    extent = [1.25, 1.25]
    use_free_mask = False

    compare_layers_and_connections(use_free_mask, tmp_path, mask_params, edge_wrap, anchor, extent)


@pytest.mark.parametrize("anchor", [[0, -1], [-1, 0]])
def test_reg_grid_anchor_negative(tmp_path, anchor):
    mask_params = ("grid", {"shape": [3, 2]})
    extent = [1.25, 1.25]
    use_free_mask = False
    edge_wrap = False

    compare_layers_and_connections(use_free_mask, tmp_path, mask_params, edge_wrap, anchor, extent)


def test_reg_grid_anchor_equal(tmp_path):
    edge_wrap = False
    mask_params = ("grid", {"shape": [3, 3]})
    anchor = [1, 1]
    extent = [1.25, 1.25]
    use_free_mask = False

    compare_layers_and_connections(use_free_mask, tmp_path, mask_params, edge_wrap, anchor, extent)


@pytest.mark.parametrize("anchor", [[0.0, 0.0], [-0.5, -0.25]])
def test_reg_rect_anchor(tmp_path, anchor):
    edge_wrap = False
    mask_params = ("rectangular", {"lower_left": [0.0, 0.0], "upper_right": [0.6, 0.3]})
    extent = [1.25, 1.25]
    use_free_mask = False

    compare_layers_and_connections(use_free_mask, tmp_path, mask_params, edge_wrap, anchor, extent)


def test_grid_rect_offset_with_edge_wrap(tmp_path):
    edge_wrap = True
    mask_params = ("rectangular", {"lower_left": [-0.001, -0.001], "upper_right": [0.6, 0.3]})
    anchor = [0.0, 0.0]
    extent = [1.25, 1.25]
    use_free_mask = False

    compare_layers_and_connections(use_free_mask, tmp_path, mask_params, edge_wrap, anchor, extent)
