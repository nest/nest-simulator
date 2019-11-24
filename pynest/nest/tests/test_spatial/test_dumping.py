# -*- coding: utf-8 -*-
#
# test_dumping.py
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
Tests for hl_api_spatial dumping functions.
"""

import unittest
import nest
import os
import numpy as np


class DumpingTestCase(unittest.TestCase):
    def nest_tmpdir(self):
        """Returns temp dir path from environment, current dir otherwise."""
        if 'NEST_DATA_PATH' in os.environ:
            return os.environ['NEST_DATA_PATH']
        else:
            return '.'

    def test_DumpNodes(self):
        """Test dumping nodes."""
        nest.ResetKernel()
        l = nest.Create('iaf_psc_alpha',
                        positions=nest.spatial.grid(shape=[3, 3],
                                                    extent=[2., 2.],
                                                    edge_wrap=True))

        filename = os.path.join(self.nest_tmpdir(), 'test_DumpNodes.out.lyr')
        nest.DumpLayerNodes(l, filename)

        npa = np.genfromtxt(filename)
        reference = np.array([[n.get('global_id')] + list(nest.GetPosition(n)) for n in l])
        self.assertTrue(np.allclose(npa, reference))
        os.remove(filename)

    def test_DumpConns(self):
        """Test dumping connections."""
        cdict = {'rule': 'pairwise_bernoulli', 'p': 1.}
        nest.ResetKernel()
        l = nest.Create('iaf_psc_alpha',
                        positions=nest.spatial.grid(shape=[2, 1],
                                                    extent=[2., 2.],
                                                    edge_wrap=True))
        nest.Connect(l, l, cdict)

        filename = os.path.join(self.nest_tmpdir(), 'test_DumpConns.out.cnn')
        nest.DumpLayerConnections(l, l, 'static_synapse', filename)
        npa = np.genfromtxt(filename)
        reference = np.array([[1.,  1.,  1.,  1.,  0.,  0.],
                              [1.,  2.,  1.,  1., -1.,  0.],
                              [2.,  1.,  1.,  1., -1.,  0.],
                              [2.,  2.,  1.,  1.,  0.,  0.]])
        self.assertTrue(np.array_equal(npa, reference))
        os.remove(filename)

    def test_DumpConns_diff(self):
        """Test dump connections between different layers."""
        cdict = {'rule': 'pairwise_bernoulli', 'p': 1.}
        nest.ResetKernel()
        pos = nest.spatial.grid(shape=[1, 1],
                                extent=[2., 2.],
                                edge_wrap=True)
        l1 = nest.Create('iaf_psc_alpha', positions=pos)
        l2 = nest.Create('iaf_psc_alpha', positions=pos)
        nest.Connect(l1, l2, cdict)

        print('Num. connections: ', nest.GetKernelStatus('num_connections'))

        filename = os.path.join(self.nest_tmpdir(), 'test_DumpConns.out.cnn')
        nest.DumpLayerConnections(l1, l2, 'static_synapse', filename)
        print('filename:', filename)
        npa = np.genfromtxt(filename)
        reference = np.array([1.,  2.,  1.,  1.,  0.,  0.])
        self.assertTrue(np.array_equal(npa, reference))
        os.remove(filename)

    def test_DumpConns_syn(self):
        """Test dump connections with specific synapse."""
        cdict = {'rule': 'pairwise_bernoulli', 'p': 1.}
        nest.ResetKernel()
        pos = nest.spatial.grid(shape=[1, 1],
                                extent=[2., 2.],
                                edge_wrap=True)
        l1 = nest.Create('iaf_psc_alpha', positions=pos)
        l2 = nest.Create('iaf_psc_alpha', positions=pos)
        l3 = nest.Create('iaf_psc_alpha', positions=pos)
        nest.Connect(l1, l2, cdict)

        syn_model = 'stdp_synapse'
        nest.Connect(l2, l3, cdict, {'synapse_model': syn_model})

        print('Num. connections: ', nest.GetKernelStatus('num_connections'))

        filename = os.path.join(self.nest_tmpdir(), 'test_DumpConns.out.cnn')
        nest.DumpLayerConnections(l2, l3, syn_model, filename)
        print('filename:', filename)
        npa = np.genfromtxt(filename)
        reference = np.array([2., 3.,  1.,  1.,  0.,  0.])
        self.assertTrue(np.array_equal(npa, reference))
        os.remove(filename)


def suite():
    suite = unittest.makeSuite(DumpingTestCase, 'test')
    return suite


if __name__ == "__main__":

    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
