# -*- coding: utf-8 -*-
#
# test_free_reg_masks.py
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
Tests free regular masks with varying anchors, for overlap and periodic boundaries.
"""


class TestFreeMasks(SpatialTestRefs):

    def network(self, extent, anchor, wrap, mask_params=None):
        if mask_params is None:
            mask_params = {'lower_left': [0.0, 0.0], 'upper_right': [0.6, 0.3]}

        nest.ResetKernel()
        nest.set(use_compressed_spikes=False, sort_connections_by_source=False)
        positions = nest.spatial.free([[x, y] for x in np.linspace(-0.5, 0.5, 5) for y in np.linspace(0.5, -0.5, 5)],
                                      edge_wrap=wrap, extent=extent)

        population_type = 'iaf_psc_alpha'

        conns = {'rule': 'pairwise_bernoulli', 'mask': {'rectangular': mask_params, 'anchor': anchor}}

        src_layer = nest.Create(population_type, positions=positions)
        target_layer = nest.Create(population_type, positions=positions)

        nest.Connect(src_layer, target_layer, conns)
        return src_layer, target_layer

    @pytest.mark.parametrize('anchor', [None, [-0.5, -0.25]])
    def test_layers_and_connection_match_varying_anchor(self, tmp_path, anchor):
        edge_wrap = False
        src_layer, target_layer = self.network([1.25, 1.25], anchor, edge_wrap)

        # TODO: replace Dump methods by in-memory equivalent once available
        path = str(tmp_path) + "/layer.txt"

        nest.DumpLayerNodes(src_layer, path)
        stored_src = np.genfromtxt(path)

        nest.DumpLayerNodes(target_layer, path)
        stored_target = np.genfromtxt(path)

        nest.DumpLayerConnections(src_layer, target_layer, 'static_synapse', path)
        stored_connections = np.genfromtxt(path)

        file_name = f'wrap_{str(edge_wrap)}_anchor_' + '_'.join(str(x) for x in anchor)
        connections_ref = np.loadtxt(f'spatial_test_references/rectangular/{file_name}.txt')

        assert np.all(stored_src == self.src_layer_ref)
        assert np.all(stored_target == self.target_layer_ref)
        assert np.all(stored_connections == connections_ref)
