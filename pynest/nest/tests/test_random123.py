# -*- coding: utf-8 -*-
#
# test_random123.py
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
Tests of random123 generators, using the Kolmogorov-Smirnov test. Adapted from random parameter tests.

If Random123 generators are not supported, i.e. if the test of the generators
during CMake configuration fails, all tests are skipped. If the system does
not support 64-bit generators, tests of Philox_64 and Threefry_64 are skipped.
"""

import unittest
import nest


try:
    import scipy.stats
    HAVE_SCIPY = True
except ImportError:
    HAVE_SCIPY = False

nest.set_verbosity('M_WARNING')


class BaseTestCases:
    """Wrapper for the base test class. Wrapping is done so that the base class is not run."""
    @unittest.skipIf(not HAVE_SCIPY, 'SciPy package is not available')
    class Random123TestCase(unittest.TestCase):
        def ks_assert(self, param, cdf, cdf_args):
            """
            Test that the given Parameter distribution fits with the expected
            cumulative distribution using K-S.
            """
            p_val_lim = 0.05
            param_values = [param.GetValue() for _ in range(2000)]
            d, p_val = scipy.stats.kstest(param_values, cdf, args=cdf_args)
            self.assertGreater(p_val, p_val_lim)

        def setUp(self):
            nest.ResetKernel()
            nest.SetKernelStatus({'rng_type': self.rng_type})

        def shortDescription(self):
            """
            Generate a description for the test based on the RNG type.
            This is done to be able to tell the difference between tests
            of the different RNG types.
            """
            doc = '{} ({})'.format(self._testMethodDoc, self.rng_type)
            return doc.split("\n")[0].strip()

        def test_uniform(self):
            """Test uniform distribution Parameter"""
            w_min = -1.3
            w_max = 2.7

            cdf = 'uniform'
            cdf_args = (w_min, w_max - w_min)  # Uniform limits in SciPy are given as (loc, loc + scale)
            param = nest.random.uniform(min=w_min, max=w_max)

            self.ks_assert(param, cdf, cdf_args)

        def test_uniform_int(self):
            """Test uniform_int distribution Parameter"""
            w_min = 0
            w_max = 100

            cdf = 'randint'
            cdf_args = (w_min, w_max)
            # Parameter distribution is in the range [0, max)
            param = nest.random.uniform_int(w_max)

            self.ks_assert(param, cdf, cdf_args)

        def test_normal(self):
            """Test normal distribution Parameter"""
            mean = 2.3
            std = 1.7

            cdf = 'norm'
            cdf_args = (mean, std)
            param = nest.random.normal(mean=mean, std=std)

            self.ks_assert(param, cdf, cdf_args)

        def test_node_param_parameter(self):
            """Test uniformly distributed V_m"""
            w_min = -60.
            w_max = -50.
            p_val_lim = 0.05

            cdf = 'uniform'
            cdf_args = (w_min, w_max - w_min)  # Uniform limits in SciPy are given as (loc, loc + scale)
            param = nest.random.uniform(min=w_min, max=w_max)

            nodes = nest.Create('iaf_psc_alpha', 10)
            nodes.V_m = param
            vm = nodes.V_m

            d, p_val = scipy.stats.kstest(vm, cdf, args=cdf_args)

            self.assertGreater(p_val, p_val_lim)


def not_supported_msg(what):
    return f'{what} is not supported on the current system'


@unittest.skipIf('Philox_32' not in nest.GetKernelStatus('rng_types'), not_supported_msg('Random123'))
class Philox32TestCase(BaseTestCases.Random123TestCase):
    rng_type = 'Philox_32'


@unittest.skipIf('Philox_64' not in nest.GetKernelStatus('rng_types'), not_supported_msg('Philox_64'))
class Philox64TestCase(BaseTestCases.Random123TestCase):
    rng_type = 'Philox_64'


@unittest.skipIf('Threefry_32' not in nest.GetKernelStatus('rng_types'), not_supported_msg('Random123'))
class Threefry32TestCase(BaseTestCases.Random123TestCase):
    rng_type = 'Threefry_32'


@unittest.skipIf('Threefry_64' not in nest.GetKernelStatus('rng_types'), not_supported_msg('Threefry_64'))
class Threefry64TestCase(BaseTestCases.Random123TestCase):
    rng_type = 'Threefry_64'


def suite():
    test_classes = [Philox64TestCase, Philox32TestCase, Threefry32TestCase, Threefry64TestCase]
    suite_list = [unittest.makeSuite(test_class) for test_class in test_classes]
    return unittest.TestSuite(suite_list)


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
