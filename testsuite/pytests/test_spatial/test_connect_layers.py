# -*- coding: utf-8 -*-
#
# test_connect_layers.py
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
Tests of Connect with layers.
"""

import unittest

import nest
import numpy as np

try:
    import scipy.stats

    HAVE_SCIPY = True
except ImportError:
    HAVE_SCIPY = False

nest.set_verbosity("M_ERROR")


class ConnectLayersTestCase(unittest.TestCase):
    def setUp(self):
        self.dim = [4, 5]
        self.extent = [10.0, 10.0]
        nest.ResetKernel()
        nest.rng_seed = 123
        self.layer = nest.Create("iaf_psc_alpha", positions=nest.spatial.grid(self.dim, extent=self.extent))

    def _check_connections(self, projection, expected_num_connections):
        """Helper function which asserts that connecting with the specified conn_spec gives
        the expected number of connections."""
        nest.Connect(projection)
        conns = nest.GetConnections()
        self.assertEqual(len(conns), expected_num_connections)

    def _check_connections_statistical(self, projection, p, num_pairs):
        """Helper function which asserts that the number of connections created are based on a bernoulli distribution.
        The connection function is iterated N times, then the distribution of number of created connections are tested
        against a bernoulli distribution using a Kolmogorov-Smirnov test. This is done ks_N times, to get statistical
        values. The mean of the KS tests is then compared to the limits. If either of the values are below the specified
        limits, the test fails."""
        N = 100  # Number of samples per KS test
        ks_N = 5  # Number of KS tests to perform.
        p_val_lim = 0.1  # Limit for the p value of the KS test
        ks_stat_lim = 0.2  # Limit for the KS statistic

        p_vals = np.zeros(ks_N)
        ks_stats = np.zeros(ks_N)

        for ks_i in range(ks_N):
            n_conns = np.zeros(N)
            ref = np.zeros(N)
            for i in range(N):
                nest.Connect(projection)
                nest.BuildNetwork()
                n_conns[i] = nest.num_connections - np.sum(n_conns)
                ref[i] = np.sum(scipy.stats.bernoulli.rvs(p, size=num_pairs))
            ks_stats[ks_i], p_vals[ks_i] = scipy.stats.ks_2samp(n_conns, ref)
            print(f"ks_stat={ks_stats[ks_i]}, p_val={p_vals[ks_i]}")

        mean_p_val = np.mean(p_vals)
        mean_ks_stat = np.mean(ks_stats)
        print(f"mean_ks_stat={mean_ks_stat}, mean_p_val={mean_p_val}")

        self.assertGreater(mean_p_val, p_val_lim)
        self.assertLess(mean_ks_stat, ks_stat_lim)

    def _assert_connect_layers_autapses(self, autapses, expected_num_autapses):
        """Helper function which asserts that connecting with or without allowing autapses gives
        the expected number of autapses."""
        projection = nest.PairwiseBernoulli(self.layer, self.layer, p=1.0, allow_autapses=autapses)
        nest.Connect(projection)
        conns = nest.GetConnections()
        n_autapses = 0
        for s, t in zip(conns.sources(), conns.targets()):
            if s == t:
                n_autapses += 1
        self.assertEqual(n_autapses, expected_num_autapses)

    def _assert_connect_layers_multapses(self, multapses):
        """Helper function which asserts that connecting with or without allowing multapses
        gives the expected number of multapses."""
        projection = nest.FixedIndegree(
            self.layer, self.layer, indegree=10, allow_autapses=False, allow_multapses=multapses
        )
        nest.Connect(projection)
        conns = nest.GetConnections()
        conn_pairs = np.array([list(conns.sources()), list(conns.targets())]).T
        num_nonunique_conns = len(conn_pairs) - len(np.unique(conn_pairs, axis=0))
        if multapses:
            self.assertGreater(num_nonunique_conns, 0)
        else:
            self.assertEqual(num_nonunique_conns, 0)

    def _assert_connect_sliced(self, pre, post):
        """Helper function which asserts that connecting with Connect gives the expected number of connections."""
        # Using distance based probability with zero weight to
        # use ConnectLayers on the SLI level.
        p = 1.0 + 0.0 * nest.spatial.distance
        expected_conns = len(pre) * len(post)

        projection = nest.PairwiseBernoulli(pre, post, p=p)
        nest.Connect(projection)
        conns = nest.GetConnections()
        result = "{} ({}), pre length={}, post length={}".format(len(conns), expected_conns, len(pre), len(post))
        print(result)
        self.assertEqual(len(conns), expected_conns, "pre length={}, post length={}".format(len(pre), len(post)))

    def _reset_and_create_sliced(self, positions):
        """Helper function which resets the kernel and creates a layer and
        a variation of sliced instances of that layer."""
        nest.ResetKernel()
        kwargs = (
            {"positions": positions} if isinstance(positions, nest.spatial.grid) else {"n": 20, "positions": positions}
        )
        layer = nest.Create("iaf_psc_alpha", **kwargs)
        return {"layer": layer, "single": layer[10], "range": layer[8:12], "step": layer[::2]}

    def test_connect_layers_indegree(self):
        """Connecting layers with fixed_indegree."""
        projection = nest.FixedIndegree(self.layer, self.layer, indegree=2, p=1.0)
        self._check_connections(projection, 40)

    def test_connect_layers_outdegree(self):
        """Connecting layers with fixed_outdegree."""
        projection = nest.FixedOutdegree(self.layer, self.layer, outdegree=2, p=1.0)
        self._check_connections(projection, 40)

    def test_connect_layers_bernoulli(self):
        """Connecting layers with pairwise_bernoulli."""
        projection = nest.PairwiseBernoulli(self.layer, self.layer, p=1.0, use_on_source=False)
        self._check_connections(projection, 400)

    def test_connect_layers_bernoulli_source(self):
        """Connecting layers with pairwise_bernoulli."""
        projection = nest.PairwiseBernoulli(self.layer, self.layer, p=1.0, use_on_source=True)
        self._check_connections(projection, 400)

    def test_connect_layers_indegree_mask(self):
        """Connecting layers with fixed_indegree and mask."""
        projection = nest.FixedIndegree(
            self.layer,
            self.layer,
            indegree=1,
            mask={"rectangular": {"lower_left": [-5.0, -5.0], "upper_right": [0.1, 0.1]}},
        )
        self._check_connections(projection, 20)

    def test_connect_layers_indegree_kernel(self):
        """Connecting layers with fixed_indegree and kernel."""
        projection = nest.FixedIndegree(self.layer, self.layer, indegree=1, p=0.5)
        self._check_connections(projection, 20)

    def test_connect_layers_indegree_kernel_mask(self):
        """Connecting layers with fixed_indegree, kernel and mask."""
        projection = nest.FixedIndegree(
            self.layer,
            self.layer,
            indegree=1,
            p=0.5,
            mask={"rectangular": {"lower_left": [-5.0, -5.0], "upper_right": [0.1, 0.1]}},
        )
        self._check_connections(projection, 20)

    def test_connect_layers_indegree_parameter(self):
        """Connecting layers with fixed_indegree and parameter as indegree."""
        # Parameter will give numbers close to 5. With std=0.1 they should all be rounded to an indegree of exactly 5.
        mean = 5.0
        param = nest.random.normal(mean=mean, std=0.1)
        conn_spec = {"rule": "fixed_indegree", "indegree": param, "p": 1.0, "allow_multapses": True}
        self._check_connections(conn_spec, int(mean) * 20)

    def test_connect_layers_outdegree_parameter(self):
        """Connecting layers with fixed_indegree and parameter as indegree."""
        # Parameter will give numbers close to 5. With std=0.1 they should all be rounded to an outdegree of exactly 5.
        mean = 5.0
        param = nest.random.normal(mean=mean, std=0.1)
        conn_spec = {"rule": "fixed_outdegree", "outdegree": param, "p": 1.0, "allow_multapses": True}
        self._check_connections(conn_spec, int(mean) * 20)

    def test_connect_layers_outdegree_mask(self):
        """Connecting layers with fixed_outdegree and mask"""
        projection = nest.FixedOutdegree(
            self.layer,
            self.layer,
            outdegree=1,
            mask={"rectangular": {"lower_left": [-5.0, -5.0], "upper_right": [0.1, 0.1]}},
        )
        self._check_connections(projection, 20)

    def test_connect_layers_outdegree_kernel(self):
        """Connecting layers with fixed_outdegree and kernel"""
        projection = nest.FixedOutdegree(self.layer, self.layer, outdegree=1, p=0.5)
        self._check_connections(projection, 20)

    def test_connect_layers_outdegree_kernel_mask(self):
        """Connecting layers with fixed_outdegree, kernel and mask"""
        projection = nest.FixedOutdegree(
            self.layer,
            self.layer,
            outdegree=1,
            p=0.5,
            mask={"rectangular": {"lower_left": [-5.0, -5.0], "upper_right": [0.1, 0.1]}},
        )
        self._check_connections(projection, 20)

    def test_connect_layers_bernoulli_mask(self):
        """Connecting layers with pairwise_bernoulli and mask"""
        projection = nest.PairwiseBernoulli(
            self.layer, self.layer, p=1.0, mask={"rectangular": {"lower_left": [-5.0, -5.0], "upper_right": [0.1, 0.1]}}
        )
        self._check_connections(projection, 108)

    @unittest.skipIf(not HAVE_SCIPY, "SciPy package is not available")
    def test_connect_layers_bernoulli_kernel_mask(self):
        """Connecting layers with pairwise_bernoulli, kernel and mask"""
        p = 0.5
        projection = nest.PairwiseBernoulli(
            self.layer, self.layer, p=p, mask={"rectangular": {"lower_left": [-5.0, -5.0], "upper_right": [0.1, 0.1]}}
        )
        self._check_connections_statistical(projection, p, 108)

    @unittest.skipIf(not HAVE_SCIPY, "SciPy package is not available")
    def test_connect_layers_bernoulli_kernel_mask_source(self):
        """
        Connecting layers with pairwise_bernoulli, kernel and mask on source
        """
        p = 0.5
        projection = nest.PairwiseBernoulli(
            self.layer,
            self.layer,
            p=p,
            use_on_source=True,
            mask={"rectangular": {"lower_left": [-5.0, -5.0], "upper_right": [0.1, 0.1]}},
        )
        self._check_connections_statistical(projection, p, 108)

    def test_connect_nonlayers_mask(self):
        """Throw when connecting non-layer NodeCollections with mask."""
        neurons = nest.Create("iaf_psc_alpha", 20)
        projection = nest.PairwiseBernoulli(
            neurons, neurons, p=1.0, mask={"rectangular": {"lower_left": [-5.0, -5.0], "upper_right": [0.1, 0.1]}}
        )
        with self.assertRaises(TypeError):
            nest.Connect(projection)
            nest.BuildNetwork()

    def test_connect_nonlayers_kernel(self):
        """Throw when connecting non-layer NodeCollections with kernel."""
        neurons = nest.Create("iaf_psc_alpha", 20)
        projection = nest.FixedOutdegree(neurons, neurons, p=1.0, outdegree=1)
        with self.assertRaises(TypeError):
            nest.Connect(projection)
            nest.BuildNetwork()

    def test_connect_kernel_mask_wrong_rule(self):
        """Throw when connecting with mask or kernel and wrong rule."""
        projection_kernel = nest.AllToAll(self.layer, self.layer, p=0.5)
        projection_mask = nest.AllToAll(
            self.layer, self.layer, mask={"rectangular": {"lower_left": [-5.0, -5.0], "upper_right": [0.1, 0.1]}}
        )
        for projection in [projection_kernel, projection_mask]:
            with self.assertRaises(nest.kernel.NESTError):
                nest.Connect(projection)
                nest.BuildNetwork()

    def test_connect_oversized_mask(self):
        """Connecting with specified oversized mask possible."""
        free_layer = nest.Create(
            "iaf_psc_alpha", positions=nest.spatial.free([[0.0, 0.0]], edge_wrap=True, extent=[1.0, 1.0])
        )
        projection = nest.PairwiseBernoulli(free_layer, free_layer, p=1.0, mask={"circular": {"radius": 2.0}})

        with self.assertRaises(nest.kernel.NESTError):
            nest.Connect(projection)
            nest.BuildNetwork()
        self.assertEqual(nest.num_connections, 0)

        nest.reset_projection_collection()
        projection.conn_spec["allow_oversized_mask"] = True
        nest.Connect(projection)
        nest.BuildNetwork()
        self.assertEqual(nest.num_connections, 1)

    def test_connect_layers_weights(self):
        """Connecting layers with specified weights"""
        projection = nest.PairwiseBernoulli(
            self.layer, self.layer, p=1.0, syn_spec=nest.synapsemodels.static(weight=nest.random.uniform(min=0.5))
        )
        nest.Connect(projection)
        conns = nest.GetConnections()
        conn_weights = np.array(conns.get("weight"))
        self.assertTrue(len(np.unique(conn_weights)) > 1)
        self.assertTrue((conn_weights >= 0.5).all())
        self.assertTrue((conn_weights <= 1.0).all())

    def test_connect_layers_delays(self):
        """Connecting layers with specified delays"""
        projection = nest.PairwiseBernoulli(
            self.layer, self.layer, p=1.0, syn_spec=nest.synapsemodels.static(delay=nest.random.uniform(min=0.5))
        )
        nest.Connect(projection)
        conns = nest.GetConnections()
        conn_delays = np.array(conns.get("delay"))
        self.assertTrue(len(np.unique(conn_delays)) > 1)
        self.assertTrue((conn_delays >= 0.5).all())
        self.assertTrue((conn_delays <= 1.0).all())

    def test_connect_layers_autapses_possible(self):
        """Connecting layers with autapses possible"""
        self._assert_connect_layers_autapses(True, 20)

    def test_connect_layers_autapses_impossible(self):
        """Connecting layers with autapses impossible"""
        self._assert_connect_layers_autapses(False, 0)

    def test_connect_layers_multapses_possible(self):
        """Connecting layers with multapses possible"""
        self._assert_connect_layers_multapses(True)

    def test_connect_layers_multapses_impossible(self):
        """Connecting layers with multapses impossible"""
        self._assert_connect_layers_multapses(False)

    def test_connect_sliced_grid_layer(self):
        """Connecting with sliced grid layer"""
        positions = nest.spatial.grid([4, 5], extent=[10.0, 10.0])
        for sliced in ["single", "range", "step"]:
            layers = self._reset_and_create_sliced(positions)
            layer = layers["layer"]
            sliced_pre = layers[sliced]
            self._assert_connect_sliced(sliced_pre, layer)
        for sliced in ["single", "range", "step"]:
            layers = self._reset_and_create_sliced(positions)
            layer = layers["layer"]
            sliced_post = layers[sliced]
            self._assert_connect_sliced(layer, sliced_post)

    def test_connect_sliced_free_layer(self):
        """Connecting with sliced free layer"""
        positions = nest.spatial.free(nest.random.uniform(), extent=[10.0, 10.0])
        for sliced in ["single", "range", "step"]:
            layers = self._reset_and_create_sliced(positions)
            layer = layers["layer"]
            sliced_pre = layers[sliced]
            self._assert_connect_sliced(sliced_pre, layer)
        for sliced in ["single", "range", "step"]:
            layers = self._reset_and_create_sliced(positions)
            layer = layers["layer"]
            sliced_post = layers[sliced]
            self._assert_connect_sliced(layer, sliced_post)

    def test_connect_synapse_label(self):
        indegree = 10
        syn_label = 123
        projection = nest.FixedIndegree(
            self.layer,
            self.layer,
            indegree=indegree,
            p=1.0,
            mask={"rectangular": {"lower_left": [-5.0, -5.0], "upper_right": [0.0, 0.0]}},
            syn_spec=nest.synapsemodels.stdp_lbl(synapse_label=syn_label),
        )

        nest.Connect(projection)
        conns = nest.GetConnections()
        self.assertEqual(conns.get("synapse_label"), [syn_label] * len(self.layer) * indegree)

    def test_connect_receptor_type(self):
        receptor_type = 7
        multisyn_layer = nest.Create(
            "iaf_psc_exp_multisynapse",
            params={"tau_syn": [0.1 + i for i in range(receptor_type)]},
            positions=nest.spatial.grid(self.dim, extent=self.extent),
        )
        indegree = 10

        projection = nest.FixedIndegree(
            multisyn_layer,
            multisyn_layer,
            indegree=indegree,
            p=1.0,
            mask={"rectangular": {"lower_left": [-5.0, -5.0], "upper_right": [0.0, 0.0]}},
            syn_spec=nest.synapsemodels.static(receptor_type=receptor_type),
        )

        nest.Connect(projection)
        conns = nest.GetConnections()
        self.assertEqual(conns.get("receptor"), [receptor_type] * len(multisyn_layer) * indegree)

    def test_connect_integer_param(self):
        # weight and delay are intentionally integers for this test.
        weight = 2
        delay = 2
        multisyn_layer = nest.Create("iaf_psc_exp", positions=nest.spatial.grid(self.dim, extent=self.extent))
        indegree = 10
        # Combination of fixed_indegree and connection probability (kernel) to trigger ConnectLayers.
        conn_spec = {"rule": "fixed_indegree", "indegree": indegree, "p": 1.0}
        syn_spec = {"weight": weight, "delay": delay}

        nest.Connect(multisyn_layer, multisyn_layer, conn_spec, syn_spec)
        conns = nest.GetConnections()
        self.assertEqual(conns.weight, [weight] * len(multisyn_layer) * indegree)
        self.assertEqual(conns.delay, [delay] * len(multisyn_layer) * indegree)


def suite():
    suite = unittest.makeSuite(ConnectLayersTestCase, "test")
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
