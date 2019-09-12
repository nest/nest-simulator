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
Tests for random topology parameter distributions. This is an implementation
of the Kolmogorov-Smirnov test [1] to check that the distribution of weights
fits the expected distribution to level alpha=0.05 when using random
parameters. Also serves as a regression test for ticket #687.

[1] http://www.itl.nist.gov/div898/handbook/eda/section3/eda35g.htm
"""

import unittest
import nest

from math import sqrt

try:
    import numpy

    HAVE_NUMPY = True
except ImportError:
    HAVE_NUMPY = False
try:
    from math import erf

    HAVE_ERF = True
except ImportError:
    HAVE_ERF = False


@unittest.skipIf(not HAVE_NUMPY, 'NumPy package is not available')
class RandomParameterTestCase(unittest.TestCase):
    def kolmogorov_smirnov(self, weight_param, expected_cdf_func):
        """
        Create connections with given distribution of weights and test that it
        fits the given expected cumulative distribution using K-S.
        """

        # n = rows * cols * Nconn
        rows = 10
        cols = 10
        Nconn = 100

        nest.ResetKernel()

        # Create layer and connect with given weight distribution
        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.grid(rows=rows,
                                                        columns=cols))
        nest.Connect(layer, layer,
                     {'rule': 'fixed_indegree', 'indegree': Nconn},
                     {'weight': weight_param})

        # Get connection weights and sort
        connectome = nest.GetConnections()
        weights = numpy.array(connectome.get('weight'))
        weights.sort()
        n = len(weights)

        # The observed (empirical) cdf is simply i/n for weights[i]
        observed_cdf = numpy.arange(n + 1, dtype=float) / n
        expected_cdf = expected_cdf_func(weights)

        D = max(numpy.abs(expected_cdf - observed_cdf[:-1]).max(),
                numpy.abs(expected_cdf - observed_cdf[1:]).max())

        # Code to find Kalpha corresponding to level alpha:
        # alpha = 0.05
        # import scipy.optimize,scipy.stats
        # Kalpha = scipy.optimize.fmin(lambda x:
        #                              abs(alpha-scipy.stats.ksprob(x)), 1)[0]
        Kalpha = 1.3581054687500012

        self.assertTrue(sqrt(n) * D < Kalpha)

    def test_uniform(self):
        """Test uniform distribution of weights."""

        w_min = -1.3
        w_max = 2.7

        weight_param = nest.random.uniform(min=w_min, max=w_max)

        def uniform_cdf_func(w):
            return ((w > w_min) * (w < w_max) *
                    ((w - w_min) / (w_max - w_min)) + (w >= w_max) * 1.0)

        self.kolmogorov_smirnov(weight_param, uniform_cdf_func)

    @unittest.skipIf(not HAVE_ERF, 'Python function math.erf is not available')
    def test_normal(self):
        """Test normal distribution of weights."""

        mean = 2.3
        sigma = 1.7

        weight_param = nest.random.normal(loc=mean, scale=sigma)

        numpy_erf = numpy.vectorize(erf)

        def normal_cdf_func(w):
            return 0.5 * (1.0 + numpy_erf((w - mean) / sqrt(2) / sigma))

        self.kolmogorov_smirnov(weight_param, normal_cdf_func)

    @unittest.skipIf(not HAVE_ERF, 'Python function math.erf is not available')
    def test_lognormal(self):
        """Test lognormal distribution of weights."""

        mu = 1.7
        sigma = 0.9

        weight_param = nest.random.lognormal(mean=mu, sigma=sigma)

        numpy_erf = numpy.vectorize(erf)

        def lognormal_cdf_func(w):
            return 0.5 * (1.0 +
                          numpy_erf((numpy.log(w) - mu) / sqrt(2) / sigma))

        self.kolmogorov_smirnov(weight_param, lognormal_cdf_func)


def suite():
    suite = unittest.makeSuite(RandomParameterTestCase, 'test')
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
