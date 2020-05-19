# -*- coding: utf-8 -*-
#
# test_weights_as_lists.py
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
Creation tests
"""

import unittest
import warnings
import nest


@nest.ll_api.check_stack
class WeightsAsListTestCase(unittest.TestCase):
    """Creation tests"""

    def test_FixedTotalNumberWeight(self):
        """..."""

        nest.ResetKernel()
        
        ref_weights = [1.2, -3.5, 0.4, -0.2]

        A = nest.Create("iaf_psc_alpha", 3)  
        B = nest.Create("iaf_psc_delta", 4)  
        conn_dict = {'rule': 'fixed_total_number', 'N': 4}  
        syn_dict = {'weight': ref_weights}  
        nest.Connect(A, B, conn_dict, syn_dict)
        
        conns = nest.GetConnections()
        weights = conns.weight
        
        self.assertEqual(weights, ref_weights)


def suite():
    suite = unittest.makeSuite(WeightsAsListTestCase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
