# -*- coding: utf-8 -*-
#
# test_dataconnect.py
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
GIDCollections
"""

import unittest
import nest


@nest.check_stack
class GIDCollectionTestCase(unittest.TestCase):
    """Test GIDCollection."""
    
    def setUp(self):
        nest.ResetKernel()

    def test_GIDCollection_addition(self):
        """Addition of GIDCollections"""

        a = nest.Create("iaf_neuron", 2)
        b = nest.Create("iaf_neuron", 2)
        c = a + b
        
        self.assertEqual(c, (1, 2, 3, 4))
    
    def test_GIDCollection_membership(self):
        """Membership in GIDCollections"""
        
        a = nest.Create("iaf_neuron", 10)
        
        self.assertTrue(5 in a)
        self.assertTrue(10 in a)
        self.assertFalse(12 in a)
        
        

def suite():
    suite = unittest.TestSuite()
    suite.addTest(GIDCollectionTestCase('test_GIDCollection_addition'))
    suite.addTest(GIDCollectionTestCase('test_GIDCollection_membership'))
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
