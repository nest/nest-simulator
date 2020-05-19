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
    positions = nest.spatial.free([[x, 0.] for x in np.linspace(0, 1.0, N)])
    parameter = nest.logic.conditional(nest.spatial.distance < 0.19, 1.0, 0.0)
    mask = {'rectangular': {'lower_left': [0., 0.], 'upper_right': [1., 1.]}}
    reference = np.array([0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0])
    reference_masked = np.array([0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0])

    def setUp(self):
        nest.ResetKernel()
        self.spatial_nodes = nest.Create('iaf_psc_alpha', positions=self.positions)

    def _assert_histogram(self, sources_or_targets, masked=False):
        hist = np.histogram(sources_or_targets, bins=self.N, range=(1, self.N+1))
        counts, nids = hist
        ref = self.reference_masked if masked else self.reference
        np_testing.assert_array_equal(counts, ref)

    def test_connect_sliced_spatial_on_target(self):
        """Connect sliced spatial source population"""
        # pairwise_bernoulli_on_target_<2>
        nest.Connect(self.spatial_nodes[self.middle_node], self.spatial_nodes,
                     conn_spec={'rule': 'pairwise_bernoulli',
                                'p': self.parameter})
        self._assert_histogram(nest.GetConnections().target)

    def test_masked_connect_sliced_spatial_on_target(self):
        """Masked connect sliced spatial source population"""
        # pairwise_bernoulli_on_target_<2>
        nest.Connect(self.spatial_nodes[self.middle_node], self.spatial_nodes,
                     conn_spec={'rule': 'pairwise_bernoulli',
                                'p': self.parameter,
                                'mask': self.mask})
        self._assert_histogram(nest.GetConnections().target, masked=True)

    def test_connect_sliced_spatial_on_source(self):
        """Connect sliced spatial target population"""
        # pairwise_bernoulli_on_source_<2>
        nest.Connect(self.spatial_nodes, self.spatial_nodes[self.middle_node],
                     conn_spec={'rule': 'pairwise_bernoulli',
                                'p': self.parameter,
                                'use_on_source': True})
        self._assert_histogram(nest.GetConnections().source)

    def test_masked_connect_sliced_spatial_on_source(self):
        """Masked connect sliced spatial target population"""
        # pairwise_bernoulli_on_source_<2>
        nest.Connect(self.spatial_nodes, self.spatial_nodes[self.middle_node],
                     conn_spec={'rule': 'pairwise_bernoulli',
                                'p': self.parameter,
                                'use_on_source': True,
                                'mask': self.mask})
        self._assert_histogram(nest.GetConnections().source, masked=True)


def suite():
    suite = unittest.makeSuite(ConnectSlicedSpatialTestCase, 'test')
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
