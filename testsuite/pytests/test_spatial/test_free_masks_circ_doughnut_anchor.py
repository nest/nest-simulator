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
  Doughnut mask, outer radius 0.25, inner radius 0.1
 
  Setup:
      - 5x5 -> 5x5, extent 1.25x1.25
      - nodes "freely placed" at regular grid locations
  expectation:
    each node is connected to
      - the two nodes to the right and left of the source location in the target layer
      - the two nodes above and below that location
      - should give identical results to equivalent reg_mask_donut_anchor test case

         Sources             Targets
      2  7 12 17 22    	  28 33 38 43 48
      3  8 13 18 23		  29 34	39 44 49
      4	 9 14 19 24		  30 35	40 45 50
      5	10 15 20 25		  31 36	41 46 51
      6	11 16 21 26		  32 37	42 47 52
 
     Exemplary connections
       2 ->    33    17 -> 38    48    22 -> 43
 	    29 	              44                49
 
            31                46                51
       6 ->    37    21 -> 42    52    26 -> 47
"""


class TestFreeMasks(SpatialTestRefs):

    def network(self, mask_params, anchor, wrap):
        nest.ResetKernel()
        nest.set(use_compressed_spikes=False, sort_connections_by_source=False)
        positions = nest.spatial.free([[x, y] for x in np.linspace(-0.5, 0.5, 5) for y in np.linspace(0.5, -0.5, 5)],
                                      edge_wrap=wrap, extent=[1.25, 1.25])

        population_type = 'iaf_psc_alpha'

        mask_type, params = mask_params
        params['anchor'] = anchor
        conns = {'rule': 'pairwise_bernoulli', 'mask': {mask_type: params}}

        src_layer = nest.Create(population_type, positions=positions)
        target_layer = nest.Create(population_type, positions=positions)

        nest.Connect(src_layer, target_layer, conns)
        return src_layer, target_layer

    @pytest.mark.parametrize('mask_params', [('doughnut', {'outer_radius': 0.25, 'inner_radius': 0.1}),
                                             ('circular', {'radius': 0.25})])
    @pytest.mark.parametrize('anchor', [[0., 0.], [-0.25, 0.]])
    @pytest.mark.parametrize('edge_wrap', [True, False])
    def test_layers_and_connection_match(self, tmp_path, mask_params, anchor, edge_wrap):
        src_layer, target_layer = self.network(mask_params, anchor, edge_wrap)

        # demand for a method that returns the same format as
        # DumpLayerNodes/Connections but does not require writing to file?
        path = str(tmp_path) + "/layer.txt"

        nest.DumpLayerNodes(src_layer, path)
        stored_src = np.genfromtxt(path)

        nest.DumpLayerNodes(target_layer, path)
        stored_target = np.genfromtxt(path)

        nest.DumpLayerConnections(src_layer, target_layer, 'static_synapse', path)
        stored_connections = np.genfromtxt(path)

        file_name = f'wrap_{str(edge_wrap)}_anchor_' + '_'.join(str(x) for x in anchor)
        connections_ref = np.loadtxt(f'spatial_test_references/{mask_params[0]}/{file_name}.txt')

        assert np.all(stored_src == self.src_layer_ref)
        assert np.all(stored_target == self.target_layer_ref)
        assert np.all(stored_connections == connections_ref)
