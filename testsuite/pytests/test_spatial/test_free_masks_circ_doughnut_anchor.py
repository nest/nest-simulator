# -*- coding: utf-8 -*-
#
# test_free_masks_circ_doughnut_anchor.py
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
import numpy as np
import pytest

from testsuite.pytests.test_spatial.SpatialTestRefs import SpatialTestRefs

"""
Test free masks with varying anchors, boundary conditions and extents.
"""


class TestFreeMasks(SpatialTestRefs):

    def network(self, mask_params, anchor, wrap, extent):
        nest.ResetKernel()
        nest.set(use_compressed_spikes=False, sort_connections_by_source=False)
        positions = nest.spatial.free([[x, y] for x in np.linspace(-0.5, 0.5, 5) for y in np.linspace(0.5, -0.5, 5)],
                                      edge_wrap=wrap, extent=extent)

        population_type = 'iaf_psc_alpha'

        mask_type, params = mask_params
        conns = {'rule': 'pairwise_bernoulli', 'mask': {mask_type: params, 'anchor': anchor}}

        src_layer = nest.Create(population_type, positions=positions)
        target_layer = nest.Create(population_type, positions=positions)

        nest.Connect(src_layer, target_layer, conns)
        return src_layer, target_layer

    def compare_layers_and_connections(self, mask_params, edge_wrap, anchor, extent):
        src_layer, target_layer = self.network(mask_params, anchor, edge_wrap, extent)

        # TODO: replace Dump methods by in-memory equivalent once available
        path = "TEMP_SOLUTION"

        nest.DumpLayerNodes(src_layer, path)
        stored_src = np.genfromtxt(path)

        nest.DumpLayerNodes(target_layer, path)
        stored_target = np.genfromtxt(path)

        nest.DumpLayerConnections(src_layer, target_layer, 'static_synapse', path)
        stored_connections = np.genfromtxt(path)

        file_name = f'wrap_{str(edge_wrap)}_anchor_' + '_'.join(str(x) for x in anchor)
        directory = mask_params[0]
        connections_ref = np.loadtxt(f'spatial_test_references/{directory}/{file_name}.txt')

        assert np.all(stored_src == self.src_layer_ref)
        assert np.all(stored_target == self.target_layer_ref)
        assert np.all(stored_connections == connections_ref)

    @pytest.mark.parametrize('mask_params', [('doughnut', {'outer_radius': 0.25, 'inner_radius': 0.1}),
                                             ('circular', {'radius': 0.25})])
    @pytest.mark.parametrize('anchor', [[0., 0.], [-0.25, 0.]])
    @pytest.mark.parametrize('edge_wrap', [True, False])
    def test_layers_and_connection_match_with_varying_anchor_and_boundary_circ_doughnut(self, mask_params,
                                                                                        anchor, edge_wrap):
        extent = [1.25, 1.25]

        self.compare_layers_and_connections(mask_params, edge_wrap, anchor, extent)

    @pytest.mark.parametrize('anchor', [[0.0, 0.0], [-0.5, -0.25]])
    def test_layers_and_connection_match_varying_anchor_rect(self, anchor):
        mask_params = ('rectangular', {'lower_left': [0.0, 0.0], 'upper_right': [0.6, 0.3]})
        edge_wrap = False
        extent = [1.25, 1.25]

        self.compare_layers_and_connections(mask_params, edge_wrap, anchor, extent)

    def test_layers_and_connections_match_rect_edge_wrap_offset(self):
        edge_wrap = True
        mask_params = ('rectangular', {'lower_left': [-0.001, -0.001], 'upper_right': [0.6, 0.301]})
        anchor = [0.0, 0.0]
        extent = [1.05, 1.05]

        self.compare_layers_and_connections(mask_params, edge_wrap, anchor, extent)

    def test_layers_and_connections_match_rect_edge_wrap(self):
        edge_wrap = True
        mask_params = ('rectangular', {'lower_left': [0.0, 0.0], 'upper_right': [0.6, 0.3]})
        anchor = [-0.5, -0.25]
        extent = [1.25, 1.25]

        self.compare_layers_and_connections(mask_params, edge_wrap, anchor, extent)
