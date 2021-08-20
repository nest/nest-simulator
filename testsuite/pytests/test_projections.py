# -*- coding: utf-8 -*-
#
# test_projections.py
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

import unittest
import nest

nest.set_verbosity('M_ERROR')


class TestProjections(unittest.TestCase):
    threaded_num_threads = 2
    spatial_grid_dim = [4, 5]
    spatial_extent = [2., 2.]
    rec_mask = {'rectangular': {
        'lower_left': [-5., -5.],
        'upper_right': [5., 5.]}}

    def setUp(self):
        nest.ResetKernel()

    def connect_and_assert(self, projection, expected_num_conns):
        """ """
        nest.Connect(projection)
        nest.BuildNetwork()

        conns = nest.GetConnections()
        self.assertEqual(len(conns), expected_num_conns)

    def test_connect_one_to_one_projection(self):
        """Connect with one_to_one projection subclass"""
        n = nest.Create('iaf_psc_alpha')
        projection = nest.OneToOne(source=n, target=n)
        self.connect_and_assert(projection, 1)
        conns = nest.GetConnections()
        self.assertEqual(conns.source, 1)
        self.assertEqual(conns.target, 1)

    def test_connect_all_to_all_projection(self):
        """Connect with all to all projection subclass"""
        n = nest.Create('iaf_psc_alpha', 5)
        projection = nest.AllToAll(source=n, target=n)
        self.connect_and_assert(projection, len(n)*len(n))

    def test_connect_fixed_indegree_projection(self):
        """Connect with fixed_indegree projection subclass"""
        n = nest.Create('iaf_psc_alpha')
        projection = nest.FixedIndegree(source=n, target=n, indegree=5)
        self.connect_and_assert(projection, 5)

    def test_connect_fixed_outdegree_projection(self):
        """Connect with fixed_outdegree projection subclass"""
        n = nest.Create('iaf_psc_alpha')
        projection = nest.FixedOutdegree(source=n, target=n, outdegree=5)
        self.connect_and_assert(projection, 5)

    def test_connect_fixed_total_number_projection(self):
        """Connect with fixed_total_number projection subclass"""
        n = nest.Create('iaf_psc_alpha')
        projection = nest.FixedTotalNumber(source=n, target=n, N=5)
        self.connect_and_assert(projection, 5)

    def test_connect_bernoulli_projection(self):
        """Connect with pairwise bernoulli projection subclass"""
        n = nest.Create('iaf_psc_alpha', 100)
        p = 1.0
        projection = nest.PairwiseBernoulli(source=n, target=n, p=p)
        self.connect_and_assert(projection, p*len(n)*len(n))

    def test_connect_batch_projections(self):
        """Connect with multiple batched projections"""
        N = 10
        IN_A = 2
        IN_B = 5
        n = nest.Create('iaf_psc_alpha', N)

        nest.Connect(nest.FixedIndegree(source=n, target=n, indegree=IN_A))
        nest.Connect(nest.FixedIndegree(source=n, target=n, indegree=IN_B))
        nest.BuildNetwork()

        conns = nest.GetConnections()
        self.assertEqual(len(conns), N*(IN_A + IN_B))

    def test_connect_batch_projection_list(self):
        """Connect with multiple batched projections as a list"""
        N = 10
        IN_A = 2
        IN_B = 5
        n = nest.Create('iaf_psc_alpha', N)

        nest.Connect([nest.FixedIndegree(source=n, target=n, indegree=IN_A),
                      nest.FixedIndegree(source=n, target=n, indegree=IN_B)])
        nest.BuildNetwork()

        conns = nest.GetConnections()
        self.assertEqual(len(conns), N*(IN_A + IN_B))

    def test_connect_with_synapse_object(self):
        """Connect projection with synapse object"""
        weight = 0.5
        delay = 0.7

        for synapse, args, syn_ref in [(nest.synapsemodels.static, {}, 'static_synapse'),
                                       (nest.synapsemodels.ht, {'P': 0.8}, 'ht_synapse'),
                                       (nest.synapsemodels.stdp, {'lambda': 0.02}, 'stdp_synapse')]:
            nest.ResetKernel()
            n = nest.Create('iaf_psc_alpha')

            syn = synapse(weight=weight, delay=delay, **args)
            projection = nest.OneToOne(source=n, target=n, syn_spec=syn)

            nest.Connect(projection)
            nest.BuildNetwork()

            conns = nest.GetConnections()
            self.assertAlmostEqual(conns.weight, weight)
            self.assertAlmostEqual(conns.delay, delay)
            for key, item in args.items():
                self.assertAlmostEqual(conns.get(key), item)
            self.assertEqual(conns.synapse_model, syn_ref)

    def test_connect_with_collocated_synapses(self):
        """Connect projection with collocated synapses"""
        n = nest.Create('iaf_psc_alpha')

        weight_a = -2.
        weight_b = 3.

        syn_spec = nest.CollocatedSynapses(nest.synapsemodels.static(weight=weight_a),
                                           nest.synapsemodels.static(weight=weight_b))
        projection = nest.OneToOne(source=n, target=n, syn_spec=syn_spec)
        nest.Connect(projection)
        nest.BuildNetwork()

        conns = nest.GetConnections()
        self.assertEqual(len(conns), len(syn_spec))
        self.assertEqual([weight_a, weight_b], conns.weight)

    def test_connect_projection_spatial_indegree(self):
        """Spatial connect indegree with projections"""
        indegree = 1
        layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid(
            self.spatial_grid_dim, extent=self.spatial_extent))
        projection = nest.FixedIndegree(source=layer, target=layer, indegree=indegree, mask=self.rec_mask)
        self.connect_and_assert(projection, len(layer)*indegree)

    def test_connect_projection_spatial_outdegree(self):
        """Spatial connect outdegree with projections"""
        outdegree = 1
        layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid(
            self.spatial_grid_dim, extent=self.spatial_extent))
        projection = nest.FixedOutdegree(
            source=layer, target=layer, outdegree=outdegree, mask=self.rec_mask)
        self.connect_and_assert(projection, len(layer)*outdegree)

    def test_connect_projection_spatial_bernoulli_target(self):
        """Spatial connect bernoulli on target with projections"""
        p = 1.0
        layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid(
            self.spatial_grid_dim, extent=self.spatial_extent))
        projection = nest.PairwiseBernoulli(source=layer, target=layer, p=p, mask=self.rec_mask)
        self.connect_and_assert(projection, p*len(layer)*len(layer))

    def test_connect_projection_spatial_bernoulli_source(self):
        """Spatial connect bernoulli on source with projections"""
        p = 1.0
        layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid(
            self.spatial_grid_dim, extent=self.spatial_extent))
        projection = nest.PairwiseBernoulli(
            source=layer, target=layer, p=p, use_on_source=True, mask=self.rec_mask)
        self.connect_and_assert(projection, p*len(layer)*len(layer))

    def test_connect_projection_spatial_collocated(self):
        """Spatial connect with projections and collocated synapses"""
        indegree = 1
        weight_a = -2.
        weight_b = 3.

        layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid(
            self.spatial_grid_dim, extent=self.spatial_extent))

        syn_spec = nest.CollocatedSynapses(nest.synapsemodels.static(weight=weight_a),
                                           nest.synapsemodels.static(weight=weight_b))
        projection = nest.FixedIndegree(source=layer, target=layer, indegree=indegree, mask=self.rec_mask,
                                        syn_spec=syn_spec)
        nest.Connect(projection)
        nest.BuildNetwork()

        weight_ref = sorted([weight_a, weight_b]*len(layer))
        conns = nest.GetConnections()
        self.assertEqual(len(conns), len(layer)*len(syn_spec)*indegree)
        self.assertEqual(sorted(conns.weight), weight_ref)

    def test_connect_one_to_one_projection_threaded(self):
        """Connect with one_to_one projection subclass, threaded"""
        nest.SetKernelStatus({'local_num_threads': self.threaded_num_threads})
        n = nest.Create('iaf_psc_alpha')
        projection = nest.OneToOne(source=n, target=n)
        self.connect_and_assert(projection, 1)
        conns = nest.GetConnections()
        self.assertEqual(conns.source, 1)
        self.assertEqual(conns.target, 1)

    def test_connect_all_to_all_projection_threaded(self):
        """Connect with all to all projection subclass, threaded"""
        nest.SetKernelStatus({'local_num_threads': self.threaded_num_threads})
        n = nest.Create('iaf_psc_alpha', 5)
        projection = nest.AllToAll(source=n, target=n)
        self.connect_and_assert(projection, len(n)*len(n))

    def test_connect_fixed_indegree_projection_threaded(self):
        """Connect with fixed_indegree projection subclass, threaded"""
        nest.SetKernelStatus({'local_num_threads': self.threaded_num_threads})
        n = nest.Create('iaf_psc_alpha')
        projection = nest.FixedIndegree(source=n, target=n, indegree=5)
        self.connect_and_assert(projection, 5)

    def test_connect_fixed_outdegree_projection_threaded(self):
        """Connect with fixed_outdegree projection subclass, threaded"""
        nest.SetKernelStatus({'local_num_threads': self.threaded_num_threads})
        n = nest.Create('iaf_psc_alpha')
        projection = nest.FixedOutdegree(source=n, target=n, outdegree=5)
        self.connect_and_assert(projection, 5)

    def test_connect_fixed_total_number_projection_threaded(self):
        """Connect with fixed_total_number projection subclass, threaded"""
        nest.SetKernelStatus({'local_num_threads': self.threaded_num_threads})
        n = nest.Create('iaf_psc_alpha')
        projection = nest.FixedTotalNumber(source=n, target=n, N=5)
        self.connect_and_assert(projection, 5)

    def test_connect_bernoulli_projection_threaded(self):
        """Connect with pairwise bernoulli projection subclass, threaded"""
        nest.SetKernelStatus({'local_num_threads': self.threaded_num_threads})
        n = nest.Create('iaf_psc_alpha', 100)
        p = 1.0
        projection = nest.PairwiseBernoulli(source=n, target=n, p=p)
        self.connect_and_assert(projection, p*len(n)*len(n))

    def test_connect_projection_spatial_indegree_threads(self):
        """Spatial connect indegree with projections, threads"""
        nest.SetKernelStatus({'local_num_threads': self.threaded_num_threads})
        indegree = 1
        layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid(
            self.spatial_grid_dim, extent=self.spatial_extent))
        projection = nest.FixedIndegree(source=layer, target=layer, indegree=indegree, mask=self.rec_mask)
        self.connect_and_assert(projection, len(layer)*indegree)

    def test_connect_projection_spatial_outdegree_threads(self):
        """Spatial connect outdegree with projections, threads"""
        nest.SetKernelStatus({'local_num_threads': self.threaded_num_threads})
        outdegree = 1
        layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid(
            self.spatial_grid_dim, extent=self.spatial_extent))
        projection = nest.FixedOutdegree(
            source=layer, target=layer, outdegree=outdegree, mask=self.rec_mask)
        self.connect_and_assert(projection, len(layer)*outdegree)

    def test_connect_projection_spatial_bernoulli_target_threads(self):
        """Spatial connect bernoulli on target with projections, threads"""
        nest.SetKernelStatus({'local_num_threads': self.threaded_num_threads})
        p = 1.0
        layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid(
            self.spatial_grid_dim, extent=self.spatial_extent))
        projection = nest.PairwiseBernoulli(source=layer, target=layer, p=p, mask=self.rec_mask)
        self.connect_and_assert(projection, p*len(layer)*len(layer))

    def test_connect_projection_spatial_bernoulli_source_threads(self):
        """Spatial connect bernoulli on source with projections, threads"""
        nest.SetKernelStatus({'local_num_threads': self.threaded_num_threads})
        p = 1.0
        layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid(
            self.spatial_grid_dim, extent=self.spatial_extent))
        projection = nest.PairwiseBernoulli(
            source=layer, target=layer, p=p, use_on_source=True, mask=self.rec_mask)
        self.connect_and_assert(projection, p*len(layer)*len(layer))


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestProjections)
    return suite


if __name__ == '__main__':
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
