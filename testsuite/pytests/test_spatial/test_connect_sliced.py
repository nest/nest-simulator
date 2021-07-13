# -*- coding: utf-8 -*-
#
# test_connect_sliced.py
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
Tests of Connect with sliced spatial populations.
"""

import unittest
import nest
import numpy as np
import numpy.testing as np_testing

nest.set_verbosity('M_ERROR')


class ConnectSlicedSpatialTestCase(unittest.TestCase):
    N = 21
    middle_node = N//2
    free_pos = nest.spatial.free([[x, 0.] for x in np.linspace(0, 1.0, N)])
    grid_pos = nest.spatial.grid([N, 1], extent=[1.05, 0.1], center=[0.5, 0.])
    parameter = nest.logic.conditional(nest.spatial.distance < 0.19, 1.0, 0.0)
    mask = {'rectangular': {'lower_left': [0., 0.], 'upper_right': [1., 1.]}}

    reference = np.zeros(N, dtype=np.int64)
    reference[7:14] = 1

    reference_masked = np.copy(reference)
    reference_masked[:10] = 0

    def setUp(self):
        nest.ResetKernel()
        self.free_nodes = nest.Create('iaf_psc_alpha', positions=self.free_pos)
        self.grid_nodes = nest.Create('iaf_psc_alpha', positions=self.grid_pos)

    def _assert_histogram(self, sources_or_targets, ref):
        """Create a histogram of input data and assert it against reference values"""
        hist = np.histogram(sources_or_targets, bins=self.N, range=(1, self.N+1))
        counts, _ = hist
        np_testing.assert_array_equal(counts, ref)

    def test_connect_sliced_spatial_on_target(self):
        """Connect sliced spatial source population"""
        for nodes in [self.free_nodes, self.grid_nodes]:
            nest.Connect(nodes[self.middle_node], nodes,
                         conn_spec={'rule': 'pairwise_bernoulli',
                                    'p': self.parameter})
            self._assert_histogram(nest.GetConnections().target, self.reference)

    def test_masked_connect_sliced_spatial_on_target(self):
        """Masked connect sliced spatial source population"""
        for nodes in [self.free_nodes, self.grid_nodes]:
            nest.Connect(nodes[self.middle_node], nodes,
                         conn_spec={'rule': 'pairwise_bernoulli',
                                    'p': self.parameter,
                                    'mask': self.mask})
            self._assert_histogram(nest.GetConnections().target, self.reference_masked)

    def test_connect_sliced_spatial_on_source(self):
        """Connect sliced spatial target population"""
        for nodes in [self.free_nodes, self.grid_nodes]:
            nest.Connect(nodes, nodes[self.middle_node],
                         conn_spec={'rule': 'pairwise_bernoulli',
                                    'p': self.parameter,
                                    'use_on_source': True})
            self._assert_histogram(nest.GetConnections().source, self.reference)

    def test_masked_connect_sliced_spatial_on_source(self):
        """Masked connect sliced spatial target population"""
        for nodes in [self.free_nodes, self.grid_nodes]:
            nest.Connect(nodes, nodes[self.middle_node],
                         conn_spec={'rule': 'pairwise_bernoulli',
                                    'p': self.parameter,
                                    'use_on_source': True,
                                    'mask': self.mask})
            self._assert_histogram(nest.GetConnections().source, self.reference_masked)

    def test_sliced_spatial_inheritance(self):
        """Sliced spatial inherits attributes"""
        wrapped_pos = nest.spatial.free(nest.random.uniform(-0.5, 0.5), num_dimensions=2, edge_wrap=True)
        extent_pos = nest.spatial.free(nest.random.uniform(-0.5, 0.5), extent=[5., 2.5])
        wrapped_nodes = nest.Create('iaf_psc_alpha', positions=wrapped_pos)
        extent_nodes = nest.Create('iaf_psc_alpha', positions=extent_pos)
        for nodes in [self.free_nodes, self.grid_nodes, wrapped_nodes, extent_nodes]:
            for nodes_sliced in nodes:
                for attr in ['edge_wrap', 'extent']:
                    spatial_attr = nodes.spatial[attr]
                    sliced_spatial_attr = nodes_sliced.spatial[attr]
                    self.assertEqual(spatial_attr, sliced_spatial_attr, 'with attr="{}"'.format(attr))

    def test_connect_sliced_spatial_range(self):
        """Connect spatial population sliced with range"""
        start = 3
        end = 10
        ref = np.copy(self.reference)
        ref[:start] = 0
        ref[end:] = 0
        for nodes in [self.free_nodes, self.grid_nodes]:
            nest.Connect(nodes[self.middle_node], nodes[start:end],
                         conn_spec={'rule': 'pairwise_bernoulli',
                                    'p': self.parameter})
            self._assert_histogram(nest.GetConnections().target, ref)

    def test_connect_sliced_spatial_step(self):
        """Connect spatial population sliced with step"""
        step = 2
        ref = np.copy(self.reference)
        ref[1::step] = 0
        for nodes in [self.free_nodes, self.grid_nodes]:
            nest.Connect(nodes[self.middle_node], nodes[::step],
                         conn_spec={'rule': 'pairwise_bernoulli',
                                    'p': self.parameter})
            self._assert_histogram(nest.GetConnections().target, ref)


def suite():
    suite = unittest.makeSuite(ConnectSlicedSpatialTestCase, 'test')
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
