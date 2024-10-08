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

import os
import unittest

import nest
import numpy as np


class DumpingTestCase(unittest.TestCase):
    def nest_tmpdir(self):
        """Returns temp dir path from environment, current dir otherwise."""
        if "NEST_DATA_PATH" in os.environ:
            return os.environ["NEST_DATA_PATH"]
        else:
            return "."

    def test_DumpNodes(self):
        """Test dumping nodes."""
        nest.ResetKernel()
        spatial_nodes = nest.Create(
            "iaf_psc_alpha", positions=nest.spatial.grid(shape=[3, 3], extent=[2.0, 2.0], edge_wrap=True)
        )

        filename = os.path.join(self.nest_tmpdir(), "test_DumpNodes.out.lyr")
        nest.DumpLayerNodes(spatial_nodes, filename)

        npa = np.genfromtxt(filename)
        reference = np.array([[n.get("global_id")] + list(nest.GetPosition(n)) for n in spatial_nodes])
        self.assertTrue(np.allclose(npa, reference))
        os.remove(filename)

    def test_DumpConns(self):
        """Test dumping connections."""
        nest.ResetKernel()
        spatial_nodes = nest.Create(
            "iaf_psc_alpha", positions=nest.spatial.grid(shape=[2, 1], extent=[2.0, 2.0], edge_wrap=True)
        )
        nest.Connect(nest.PairwiseBernoulli(spatial_nodes, spatial_nodes, p=1.0))
        nest.BuildNetwork()

        filename = os.path.join(self.nest_tmpdir(), "test_DumpConns.out.cnn")
        nest.DumpLayerConnections(spatial_nodes, spatial_nodes, "static_synapse", filename)
        npa = np.genfromtxt(filename)
        reference = np.array(
            [
                [1.0, 1.0, 1.0, 1.0, 0.0, 0.0],
                [1.0, 2.0, 1.0, 1.0, -1.0, 0.0],
                [2.0, 1.0, 1.0, 1.0, -1.0, 0.0],
                [2.0, 2.0, 1.0, 1.0, 0.0, 0.0],
            ]
        )
        self.assertTrue(np.array_equal(npa, reference))
        os.remove(filename)

    def test_DumpConns_diff(self):
        """Test dump connections between different layers."""
        nest.ResetKernel()
        pos = nest.spatial.grid(shape=[1, 1], extent=[2.0, 2.0], edge_wrap=True)
        l1 = nest.Create("iaf_psc_alpha", positions=pos)
        l2 = nest.Create("iaf_psc_alpha", positions=pos)
        nest.Connect(nest.PairwiseBernoulli(l1, l2, p=1.0))
        nest.BuildNetwork()

        print("Num. connections:", nest.num_connections)

        filename = os.path.join(self.nest_tmpdir(), "test_DumpConns.out.cnn")
        nest.DumpLayerConnections(l1, l2, "static_synapse", filename)
        print("filename:", filename)
        npa = np.genfromtxt(filename)
        reference = np.array([1.0, 2.0, 1.0, 1.0, 0.0, 0.0])
        self.assertTrue(np.array_equal(npa, reference))
        os.remove(filename)

    def test_DumpConns_syn(self):
        """Test dump connections with specific synapse."""
        nest.ResetKernel()
        pos = nest.spatial.grid(shape=[1, 1], extent=[2.0, 2.0], edge_wrap=True)
        l1 = nest.Create("iaf_psc_alpha", positions=pos)
        l2 = nest.Create("iaf_psc_alpha", positions=pos)
        l3 = nest.Create("iaf_psc_alpha", positions=pos)
        nest.Connect(nest.PairwiseBernoulli(l2, l3, p=1.0))

        syn_model = "stdp_synapse"
        nest.Connect(nest.PairwiseBernoulli(l2, l3, p=1.0, syn_spec=nest.synapsemodels.stdp()))
        nest.BuildNetwork()

        print("Num. connections:", nest.num_connections)

        filename = os.path.join(self.nest_tmpdir(), "test_DumpConns.out.cnn")
        nest.DumpLayerConnections(l2, l3, syn_model, filename)
        print("filename:", filename)
        npa = np.genfromtxt(filename)
        reference = np.array([2.0, 3.0, 1.0, 1.0, 0.0, 0.0])
        self.assertTrue(np.array_equal(npa, reference))
        os.remove(filename)

    def test_DumpConns_sliced(self):
        """Test dumping connections with sliced layer."""
        nest.ResetKernel()
        spatial_nodes = nest.Create(
            "iaf_psc_alpha", positions=nest.spatial.grid(shape=[10, 1], extent=[2.0, 2.0], edge_wrap=True)
        )
        nest.Connect(nest.PairwiseBernoulli(spatial_nodes, spatial_nodes, p=1.0))
        nest.BuildNetwork()

        filename = os.path.join(self.nest_tmpdir(), "test_DumpConns.out.cnn")
        nest.DumpLayerConnections(spatial_nodes[0], spatial_nodes, "static_synapse", filename)
        npa = np.genfromtxt(filename)
        reference = np.array(
            [
                [1.0, 1.0, 1.0, 1.0, 0.0, 0.0],
                [1.0, 2.0, 1.0, 1.0, 0.2, 0.0],
                [1.0, 3.0, 1.0, 1.0, 0.4, 0.0],
                [1.0, 4.0, 1.0, 1.0, 0.6, 0.0],
                [1.0, 5.0, 1.0, 1.0, 0.8, 0.0],
                [1.0, 6.0, 1.0, 1.0, -1.0, 0.0],
                [1.0, 7.0, 1.0, 1.0, -0.8, 0.0],
                [1.0, 8.0, 1.0, 1.0, -0.6, 0.0],
                [1.0, 9.0, 1.0, 1.0, -0.4, 0.0],
                [1.0, 10.0, 1.0, 1.0, -0.2, 0.0],
            ]
        )
        # Connections for a single source may be shuffled, so we simply
        # check if the dumped connections are the same as in the reference.
        self.assertEqual(len(npa), len(reference))
        for conn in npa:
            self.assertTrue(conn in reference, "Connection not in reference: {}".format(conn))
        os.remove(filename)


def suite():
    suite = unittest.makeSuite(DumpingTestCase, "test")
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
