# -*- coding: utf-8 -*-
#
# test_create.py
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
GIDCollection tests
"""

import nest
import unittest


@nest.check_stack
class TestGIDCollection(unittest.TestCase):
    """GIDCollection tests"""

    def test_GIDCollection_to_list(self):

        nest.ResetKernel()

        n = nest.Create('iaf_neuron',10)
        n_list = [x for x in n]
        list_compare = [1,2,3,4,5,6,7,8,9,10]
        self.assertEqual(n_list,list_compare)


    def test_equal(self):

        nest.ResetKernel()

        n = nest.Create('iaf_neuron', 10)
        list_compare = (1,2,3,4,5,6,7,8,9,10)
        self.assertEqual(n,list_compare)

    def test_indexing(self):

        nest.ResetKernel()

        n = nest.Create('iaf_neuron', 5)
        self.assertEqual(n[2],3)

    def test_slicing(self):

        nest.ResetKernel()

        n = nest.Create('iaf_neuron', 10)
        self.assertEqual(n[:5],(1,2,3,4,5))



def suite():
    suite = unittest.makeSuite(TestGIDCollection, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())

if __name__ == "__main__":
    run()
