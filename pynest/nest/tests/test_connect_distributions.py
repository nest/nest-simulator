# -*- coding: utf-8 -*-
#
# test_connect_distributions.py
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
from . import test_connect_helpers as hf

HAVE_GSL = nest.ll_api.sli_func("statusdict/have_gsl ::")


class TestDists(unittest.TestCase):

    rule = 'all_to_all'
    conn_dict = {'rule': rule}
    label = 'weight'
    # defauly synapse dictionary
    model = 'static_synapse'
    syn_dict = {'synapse_model': model}
    # sizes of populations
    Ndist1 = 40
    Ndist2 = 40
    # parameter for test of distributed parameter
    pval = 0.05  # minimum p-value to pass kolmogorov smirnov test

    def setUp(self):
        nest.ResetKernel()
        nest.ll_api.sr("statusdict/threading :: (no) eq not")
        if not nest.ll_api.spp():
            # no multi-threading
            nest.SetKernelStatus({'grng_seed': 120,
                                  'rng_seeds': [576]})
        else:
            # multi-threading
            nest.SetKernelStatus({'local_num_threads': 2,
                                  'grng_seed': 120,
                                  'rng_seeds': [576, 886]})
        pass

    def setUpNetwork(self, conn_params=None, syn_dict=None):
        conn_params['allow_autapses'] = False
        conn_params['allow_multapses'] = False
        self.pop1 = nest.Create('iaf_psc_alpha', self.Ndist1)
        self.pop2 = nest.Create('iaf_psc_alpha', self.Ndist2)
        nest.Connect(self.pop1, self.pop2, conn_params, syn_dict)

    def testNormalDist(self):
        syn_params = self.syn_dict.copy()
        distribution = 'normal'
        mean = 4.5
        std = 2.0
        syn_params[self.label] = {
            'distribution': distribution, 'mu': mean, 'sigma': std}
        self.setUpNetwork(self.conn_dict, syn_params)
        is_dist = hf.check_ks(self.pop1, self.pop2,
                              self.label, self.pval, syn_params[self.label])
        self.assertTrue(is_dist)

    def testNormalClippedDist(self):
        syn_params = self.syn_dict.copy()
        distribution = 'normal_clipped'
        mean = 1.0
        std = 2.0
        xmin = 0.5
        xmax = 10.2
        syn_params[self.label] = {
            'distribution': distribution,
            'mu': mean, 'sigma': std, 'low': xmin, 'high': xmax
        }
        self.setUpNetwork(self.conn_dict, syn_params)
        is_dist = hf.check_ks(self.pop1, self.pop2,
                              self.label, self.pval, syn_params[self.label])
        self.assertTrue(is_dist)

    def testBinomialDist(self):
        syn_params = self.syn_dict.copy()
        distribution = 'binomial'
        n = 50
        p = 0.4
        syn_params[self.label] = {'distribution': distribution, 'n': n, 'p': p}
        self.setUpNetwork(self.conn_dict, syn_params)
        is_dist = hf.check_ks(self.pop1, self.pop2,
                              self.label, self.pval, syn_params[self.label])
        self.assertTrue(is_dist)

    def testBinomialClippedDist(self):
        syn_params = self.syn_dict.copy()
        distribution = 'binomial_clipped'
        n = 50
        p = 0.4
        xmin = 10
        xmax = 30
        syn_params[self.label] = {
            'distribution': distribution, 'n': n, 'p': p,
            'low': xmin, 'high': xmax
        }
        self.setUpNetwork(self.conn_dict, syn_params)
        is_dist = hf.check_ks(self.pop1, self.pop2,
                              self.label, self.pval, syn_params[self.label])
        self.assertTrue(is_dist)

    @unittest.skipIf(not HAVE_GSL, 'GSL is not available')
    def testGslBinomialDist(self):
        syn_params = self.syn_dict.copy()
        distribution = 'gsl_binomial'
        n = 70
        p = 0.55
        syn_params[self.label] = {'distribution': distribution, 'n': n, 'p': p}
        self.setUpNetwork(self.conn_dict, syn_params)
        syn_params[self.label]['distribution'] = 'binomial'
        is_dist = hf.check_ks(self.pop1, self.pop2,
                              self.label, self.pval, syn_params[self.label])
        self.assertTrue(is_dist)

    def testExponentialDist(self):
        syn_params = self.syn_dict.copy()
        distribution = 'exponential'
        l = 1.7
        syn_params[self.label] = {'distribution': distribution, 'lambda': l}
        self.setUpNetwork(self.conn_dict, syn_params)
        is_dist = hf.check_ks(self.pop1, self.pop2,
                              self.label, self.pval, syn_params[self.label])
        self.assertTrue(is_dist)

    def testExponentialClippedDist(self):
        syn_params = self.syn_dict.copy()
        distribution = 'exponential_clipped'
        l = 1.7
        xmin = 0.2
        xmax = 2.0
        syn_params[self.label] = {
            'distribution': distribution, 'lambda': l, 'low': xmin,
            'high': xmax
        }
        self.setUpNetwork(self.conn_dict, syn_params)
        is_dist = hf.check_ks(self.pop1, self.pop2,
                              self.label, self.pval, syn_params[self.label])
        self.assertTrue(is_dist)

    def testGammaDist(self):
        syn_params = self.syn_dict.copy()
        distribution = 'gamma'
        order = 0.5
        scale = 0.75
        syn_params[self.label] = {
            'distribution': distribution, 'order': order, 'scale': scale}
        self.setUpNetwork(self.conn_dict, syn_params)
        is_dist = hf.check_ks(self.pop1, self.pop2,
                              self.label, self.pval, syn_params[self.label])
        self.assertTrue(is_dist)

    def testGammaClippedDist(self):
        syn_params = self.syn_dict.copy()
        distribution = 'gamma_clipped'
        order = 0.5
        scale = 0.75
        xmin = 0.3
        xmax = 1.5
        syn_params[self.label] = {
            'distribution': distribution,
            'order': order, 'scale': scale,
            'low': xmin, 'high': xmax
        }
        self.setUpNetwork(self.conn_dict, syn_params)
        is_dist = hf.check_ks(self.pop1, self.pop2,
                              self.label, self.pval, syn_params[self.label])
        self.assertTrue(is_dist)

    def testLognormalDist(self):
        syn_params = self.syn_dict.copy()
        distribution = 'lognormal'
        mean = 0.5
        std = 1.2
        syn_params[self.label] = {
            'distribution': distribution, 'mu': mean, 'sigma': std}
        self.setUpNetwork(self.conn_dict, syn_params)
        is_dist = hf.check_ks(self.pop1, self.pop2,
                              self.label, self.pval, syn_params[self.label])
        self.assertTrue(is_dist)

    def testLognormalClippedDist(self):
        syn_params = self.syn_dict.copy()
        distribution = 'lognormal_clipped'
        mean = 0.5
        std = 1.2
        xmin = 1.5
        xmax = 10.3
        syn_params[self.label] = {
            'distribution': distribution,
            'mu': mean, 'sigma': std, 'low': xmin, 'high': xmax}
        self.setUpNetwork(self.conn_dict, syn_params)
        is_dist = hf.check_ks(self.pop1, self.pop2,
                              self.label, self.pval, syn_params[self.label])
        self.assertTrue(is_dist)

    def testPoissonDist(self):
        # TODO: check for small lambda
        syn_params = self.syn_dict.copy()
        distribution = 'poisson'
        l = 5.1
        syn_params[self.label] = {'distribution': distribution, 'lambda': l}
        self.setUpNetwork(self.conn_dict, syn_params)
        is_dist = hf.check_ks(self.pop1, self.pop2,
                              self.label, self.pval, syn_params[self.label])
        self.assertTrue(is_dist)

    def testPoissonClippedDist(self):
        # TODO: check for small lambda
        syn_params = self.syn_dict.copy()
        distribution = 'poisson_clipped'
        l = 5.1
        xmin = 2
        xmax = 8
        syn_params[self.label] = {
            'distribution': distribution, 'lambda': l,
            'low': xmin, 'high': xmax}
        self.setUpNetwork(self.conn_dict, syn_params)
        is_dist = hf.check_ks(self.pop1, self.pop2,
                              self.label, self.pval, syn_params[self.label])
        self.assertTrue(is_dist)

    def testUniformDist(self):
        syn_params = self.syn_dict.copy()
        distribution = 'uniform'
        a = 2.3
        b = 4.5
        syn_params[self.label] = {
            'distribution': distribution, 'low': a, 'high': b}
        self.setUpNetwork(self.conn_dict, syn_params)
        is_dist = hf.check_ks(self.pop1, self.pop2,
                              self.label, self.pval, syn_params[self.label])
        self.assertTrue(is_dist)

    def testUniformIntDist(self):
        syn_params = self.syn_dict.copy()
        distribution = 'uniform_int'
        low = 3
        high = 100
        syn_params[self.label] = {
            'distribution': distribution, 'low': low, 'high': high}
        self.setUpNetwork(self.conn_dict, syn_params)
        is_dist = hf.check_ks(self.pop1, self.pop2,
                              self.label, self.pval, syn_params[self.label])
        self.assertTrue(is_dist)


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestDists)
    return suite


if __name__ == '__main__':
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
