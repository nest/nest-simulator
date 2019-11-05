# -*- coding: utf-8 -*-
#
# test_random_parameter.py
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
Tests of random Parameter distributions, using the Kolmogorov-Smirnov test.
"""

import unittest
import nest
import numpy as np

try:
    import scipy.stats
    HAVE_SCIPY = True
except ImportError:
    HAVE_SCIPY = False


@unittest.skipIf(not HAVE_SCIPY, 'SciPy package is not available')
class RandomParameterTestCase(unittest.TestCase):
    def ks_assert(self, param, cdf, cdf_args):
        """
        Test that the given Parameter distribution fits with the expected
        cumulative distribution using K-S.
        """
        p_val_lim = 0.05
        param_values = [param.GetValue() for _ in range(100)]
        d, p_val = scipy.stats.kstest(param_values, cdf, args=cdf_args)
        self.assertGreater(p_val, p_val_lim)

    def test_uniform(self):
        """Test uniform distribution Parameter"""
        w_min = -1.3
        w_max = 2.7

        cdf = 'uniform'
        cdf_args = (w_min, w_max - w_min)  # Uniform limits in SciPy are given as (loc, loc + scale)
        param = nest.random.uniform(min=w_min, max=w_max)

        self.ks_assert(param, cdf, cdf_args)

    def test_normal(self):
        """Test normal distribution Parameter"""
        mean = 2.3
        std = 1.7

        cdf = 'norm'
        cdf_args = (mean, std)
        param = nest.random.normal(mean=mean, std=std)

        self.ks_assert(param, cdf, cdf_args)

    def test_lognormal(self):
        """Test lognormal distribution Parameter"""
        loc = 0.  # Only for SciPy distribution
        std = 0.9
        mean = 1.7
        scale = np.exp(mean)

        cdf = 'lognorm'
        cdf_args = (std, loc, scale)
        param = nest.random.lognormal(mean=mean, std=std)

        self.ks_assert(param, cdf, cdf_args)

    def test_exponential(self):
        """Test exponential distribution Parameter"""
        loc = 0.0  # Only for SciPy distribution
        beta = 1.0

        cdf = 'expon'
        cdf_args = (loc, beta)
        param = nest.random.exponential(beta=beta)

        self.ks_assert(param, cdf, cdf_args)


def suite():
    suite = unittest.makeSuite(RandomParameterTestCase, 'test')
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
