# -*- coding: utf-8 -*-
#
# test_networks.py
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
Network tests
"""

import unittest
import nest


@nest.hl_api.check_stack
class NetworkTestCase(unittest.TestCase):
    """Network tests"""

    def test_BeginEndSubnet(self):
        """Begin/End Subnet"""

        nest.ResetKernel()

        nest.hl_api.BeginSubnet()
        sn = nest.hl_api.EndSubnet()

        nest.hl_api.BeginSubnet(label='testlabel')
        sn = nest.hl_api.EndSubnet()

        self.assertEqual(nest.GetStatus(sn, 'label')[0], 'testlabel')

    def test_CurrentSubnet(self):
        """Current Subnet"""

        nest.ResetKernel()
        self.assertEqual(nest.hl_api.CurrentSubnet(), (0, ))

        nest.hl_api.BeginSubnet()
        self.assertEqual(nest.hl_api.CurrentSubnet(), (1, ))

    def test_GetLeaves(self):
        """GetLeaves"""

        nest.ResetKernel()
        model = 'iaf_psc_alpha'
        l = nest.hl_api.LayoutNetwork(model, (2, 3))
        allLeaves = (3, 4, 5, 7, 8, 9)

        # test all
        self.assertEqual(nest.GetLeaves(l), (allLeaves, ))

        # test all with empty dict
        self.assertEqual(nest.GetLeaves(l, properties={}), (allLeaves, ))

        # test iteration over subnets
        self.assertEqual(nest.GetLeaves(l + l), (allLeaves, allLeaves))

        # children of l are not leaves, should yield empty
        self.assertEqual(nest.GetLeaves(
            l, properties={'parent': l[0]}), (tuple(), ))

        # local id of middle nodes
        self.assertEqual(nest.GetLeaves(
            l, properties={'local_id': 2}), ((4, 8), ))

        # selection by model type
        self.assertEqual(nest.GetLeaves(
            l, properties={'model': model}), (allLeaves, ))

    def test_GetNodes(self):
        """GetNodes"""

        nest.ResetKernel()
        model = 'iaf_psc_alpha'
        l = nest.hl_api.LayoutNetwork(model, (2, 3))
        allNodes = tuple(range(2, 10))
        allSubnets = (2, 6)
        allLeaves = tuple(n for n in allNodes if n not in allSubnets)

        # test all
        self.assertEqual(nest.GetNodes(l), (allNodes, ))

        # test all with empty dict
        self.assertEqual(nest.GetNodes(l, properties={}), (allNodes, ))

        # test iteration over subnets
        self.assertEqual(nest.GetNodes(l + l), (allNodes, allNodes))

        # children of l are nodes
        self.assertEqual(nest.GetNodes(
            l, properties={'parent': l[0]}), (allSubnets, ))

        # local id of second intermediate subnet and middle nodes
        self.assertEqual(nest.GetNodes(
            l, properties={'local_id': 2}), ((4, 6, 8), ))

        # selection by model type
        self.assertEqual(nest.GetNodes(
            l, properties={'model': 'subnet'}), (allSubnets, ))
        self.assertEqual(nest.GetNodes(
            l, properties={'model': model}), (allLeaves, ))

    def test_GetChildren(self):
        """GetChildren"""

        nest.ResetKernel()
        model = 'iaf_psc_alpha'
        l = nest.hl_api.LayoutNetwork(model, (2, 3))
        topKids = (2, 6)
        kids2 = (3, 4, 5)
        kids6 = (7, 8, 9)

        # test top level
        self.assertEqual(nest.hl_api.GetChildren(l), (topKids, ))

        # test underlying level
        self.assertEqual(nest.hl_api.GetChildren((2, 6)), (kids2, kids6))

        # test with empty dict
        self.assertEqual(nest.hl_api.GetChildren(
            l, properties={}), (topKids, ))

        # local id of middle nodes
        self.assertEqual(nest.hl_api.GetChildren(
            (2, 6), properties={'local_id': 2}), ((4, ), (8, )))

        # selection by model type
        self.assertEqual(nest.hl_api.GetChildren(
            l, properties={'model': 'subnet'}), (topKids, ))
        self.assertEqual(nest.hl_api.GetChildren(
            (2, ), properties={'model': 'subnet'}), (tuple(), ))
        self.assertEqual(nest.hl_api.GetChildren(
            (2, ), properties={'model': model}), (kids2, ))

    def test_GetNetwork(self):
        """GetNetwork"""

        nest.ResetKernel()
        nest.hl_api.BeginSubnet(label='subnet1')
        nest.hl_api.BeginSubnet(label='subnet2')

        n = nest.Create('iaf_psc_alpha', 100)
        sn2 = nest.hl_api.EndSubnet()
        sn1 = nest.hl_api.EndSubnet()

        self.assertEqual(nest.hl_api.CurrentSubnet(), (0, ))
        self.assertEqual(nest.hl_api.GetNetwork(sn1, 1)[1], sn2[0])
        self.assertEqual(len(nest.hl_api.GetNetwork(sn1, 2)[1]), len(
            nest.hl_api.GetNetwork(sn2, 1)))


def suite():

    suite = unittest.makeSuite(NetworkTestCase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
