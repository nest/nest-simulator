# -*- coding: utf-8 -*-
#
# test_getconnections.py
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
GetConnections
"""

import unittest
import nest


@nest.ll_api.check_stack
class GetConnectionsTestCase(unittest.TestCase):
    """Find connections and test if values can be set."""

    def test_GetConnections(self):
        """GetConnections"""

        nest.ResetKernel()

        a = nest.Create("iaf_psc_alpha", 3)
        nest.Connect(a, a)
        c1 = nest.GetConnections(a)
        c2 = nest.GetConnections(a, synapse_model="static_synapse")
        self.assertEqual(c1, c2)

        weights = (2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0)
        d1 = tuple({"weight": w} for w in weights)

        c3 = nest.GetConnections(a, a)
        nest.SetStatus(c3, d1)
        s1 = nest.GetStatus(c3, "weight")
        self.assertEqual(s1, weights)

        c4 = nest.GetConnections()
        self.assertEqual(c1, c4)

        weights = (11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0, 19.0)
        d1 = tuple({"weight": w} for w in weights)

        c5 = nest.GetConnections(a, a)
        c5.set(d1)
        s2 = c5.get('weight')
        self.assertEqual(s2, list(weights))

        c6 = nest.GetConnections()
        self.assertEqual(c1, c6)


def suite():

    suite = unittest.makeSuite(GetConnectionsTestCase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
