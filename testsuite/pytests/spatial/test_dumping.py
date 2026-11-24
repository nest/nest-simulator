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
import numpy as np
import pytest


class DumpingTestCase(unittest.TestCase):
    @pytest.fixture(autouse=True)
    def _tmp_path(self, tmp_path):
        """Gives each test a directory of its own, which pytest removes."""
        self.tmp_path = tmp_path

    def test_DumpNodes(self):
        """Test dumping nodes."""
        nest.ResetKernel()
        spatial_nodes = nest.Create(
            "iaf_psc_alpha", positions=nest.spatial.grid(shape=[3, 3], extent=[2.0, 2.0], edge_wrap=True)
        )

        filename = str(self.tmp_path / "nodes.out.lyr")
        nest.DumpLayerNodes(spatial_nodes, filename)

        npa = np.genfromtxt(filename)
        reference = np.array([[n.global_id] + list(nest.GetPosition(n)) for n in spatial_nodes])
        self.assertTrue(np.allclose(npa, reference))

    def test_DumpConns(self):
        """Test dumping connections."""
        cdict = {"rule": "pairwise_bernoulli", "p": 1.0}
        nest.ResetKernel()
        spatial_nodes = nest.Create(
            "iaf_psc_alpha", positions=nest.spatial.grid(shape=[2, 1], extent=[2.0, 2.0], edge_wrap=True)
        )
        nest.Connect(spatial_nodes, spatial_nodes, cdict)

        filename = str(self.tmp_path / "conns.out.cnn")
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

    def test_DumpConns_diff(self):
        """Test dump connections between different layers."""
        cdict = {"rule": "pairwise_bernoulli", "p": 1.0}
        nest.ResetKernel()
        pos = nest.spatial.grid(shape=[1, 1], extent=[2.0, 2.0], edge_wrap=True)
        l1 = nest.Create("iaf_psc_alpha", positions=pos)
        l2 = nest.Create("iaf_psc_alpha", positions=pos)
        nest.Connect(l1, l2, cdict)

        print("Num. connections:", nest.num_connections)

        filename = str(self.tmp_path / "conns.out.cnn")
        nest.DumpLayerConnections(l1, l2, "static_synapse", filename)
        print("filename:", filename)
        npa = np.genfromtxt(filename)
        reference = np.array([1.0, 2.0, 1.0, 1.0, 0.0, 0.0])
        self.assertTrue(np.array_equal(npa, reference))

    def test_DumpConns_syn(self):
        """Test dump connections with specific synapse."""
        cdict = {"rule": "pairwise_bernoulli", "p": 1.0}
        nest.ResetKernel()
        pos = nest.spatial.grid(shape=[1, 1], extent=[2.0, 2.0], edge_wrap=True)
        l1 = nest.Create("iaf_psc_alpha", positions=pos)
        l2 = nest.Create("iaf_psc_alpha", positions=pos)
        l3 = nest.Create("iaf_psc_alpha", positions=pos)
        nest.Connect(l1, l2, cdict)

        syn_model = "stdp_synapse"
        nest.Connect(l2, l3, cdict, {"synapse_model": syn_model})

        print("Num. connections:", nest.num_connections)

        filename = str(self.tmp_path / "conns.out.cnn")
        nest.DumpLayerConnections(l2, l3, syn_model, filename)
        print("filename:", filename)
        npa = np.genfromtxt(filename)
        reference = np.array([2.0, 3.0, 1.0, 1.0, 0.0, 0.0])
        self.assertTrue(np.array_equal(npa, reference))

    def test_DumpConns_sliced(self):
        """Test dumping connections with sliced layer."""
        cdict = {"rule": "pairwise_bernoulli", "p": 1.0}
        nest.ResetKernel()
        spatial_nodes = nest.Create(
            "iaf_psc_alpha", positions=nest.spatial.grid(shape=[10, 1], extent=[2.0, 2.0], edge_wrap=True)
        )
        nest.Connect(spatial_nodes, spatial_nodes, cdict)

        filename = str(self.tmp_path / "conns.out.cnn")
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
