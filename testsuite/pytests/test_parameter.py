# -*- coding: utf-8 -*-
#
# test_parameter.py
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
Simple Parameter tests
"""

import nest
import unittest


class TestParameter(unittest.TestCase):

    def setUp(self):
        nest.ResetKernel()

    def test_add(self):
        low = 55.
        high = 75.
        val = 33.
        e = nest.random.uniform(low, high) + val

        self.assertGreater(e.GetValue(), low + val)
        self.assertLess(e.GetValue(), high + val)

    def test_radd(self):
        low = 55.
        high = 75.
        val = 33.
        e = val + nest.random.uniform(low, high)

        self.assertGreater(e.GetValue(), low + val)
        self.assertLess(e.GetValue(), high + val)

    def test_sub(self):
        low = 55.
        high = 75.
        val = 33.
        e = nest.random.uniform(low, high) - val

        self.assertGreater(e.GetValue(), low - val)
        self.assertLess(e.GetValue(), high - val)

    def test_rsub(self):
        low = 55.
        high = 75.
        val = 33.
        e = val - nest.random.uniform(low, high)

        self.assertGreater(e.GetValue(), val - high)
        self.assertLess(e.GetValue(), val - low)

    def test_neg(self):
        low = 55.
        high = 75.
        e = -nest.random.uniform(low, high)

        self.assertGreater(e.GetValue(), -high)
        self.assertLess(e.GetValue(), -low)

    def test_rsub_all_neg(self):
        low = -55.
        high = -75.
        val = -33.
        e = val - nest.random.uniform(high, low)

        self.assertGreater(e.GetValue(), val - low)
        self.assertLess(e.GetValue(), val - high)

    def test_mul(self):
        low = 55.
        high = 75.
        val = 3.
        e = nest.random.uniform(low, high) * val

        self.assertGreater(e.GetValue(), low * val)
        self.assertLess(e.GetValue(), high * val)

    def test_rmul(self):
        low = 55.
        high = 75.
        val = 3.
        e = val * nest.random.uniform(low, high)

        self.assertGreater(e.GetValue(), val * low)
        self.assertLess(e.GetValue(), val * high)

    def test_div(self):
        low = 55.
        high = 75.
        val = 3.
        e = nest.random.uniform(low, high) / val

        self.assertGreater(e.GetValue(), low / val)
        self.assertLess(e.GetValue(), high / val)


def suite():
    suite = unittest.makeSuite(TestParameter, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
