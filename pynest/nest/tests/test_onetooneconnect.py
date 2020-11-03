# -*- coding: utf-8 -*-
#
# test_onetooneconnect.py
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
UnitTests for the PyNEST connect API.
"""

import unittest
import nest


@nest.ll_api.check_stack
class OneToOneConnectTestCase(unittest.TestCase):
    """Tests of Connect with OneToOne pattern"""

    def setUp(self):
        nest.ResetKernel()

    def test_ConnectPrePost(self):
        """Connect pre to post"""

        pre = nest.Create("iaf_psc_alpha", 2)
        post = nest.Create("iaf_psc_alpha", 2)
        nest.Connect(pre, post, "one_to_one")
        connections = nest.GetConnections(pre)
        targets = connections.get("target")
        self.assertEqual(list(targets), post.tolist())

    def test_ConnectPrePostParams(self):
        """Connect pre to post with a params dict"""

        pre = nest.Create("iaf_psc_alpha", 2)
        post = nest.Create("iaf_psc_alpha", 2)
        nest.Connect(pre, post, "one_to_one", syn_spec={"weight": 2.0})
        connections = nest.GetConnections(pre)
        weights = connections.get("weight")
        self.assertEqual(weights, [2.0, 2.0])

        nest.ResetKernel()
        pre = nest.Create("iaf_psc_alpha", 2)
        post = nest.Create("iaf_psc_alpha", 2)
        nest.Connect(pre, post, conn_spec={"rule": "one_to_one"},
                     syn_spec={"weight": [2.0, 3.0]})
        connections = nest.GetConnections(pre)
        weights = connections.get("weight")
        self.assertEqual(weights, [2.0, 3.0])

    def test_ConnectPrePostWD(self):
        """Connect pre to post with a weight and delay"""

        pre = nest.Create("iaf_psc_alpha", 2)
        post = nest.Create("iaf_psc_alpha", 2)
        nest.Connect(pre, post, conn_spec={"rule": "one_to_one"},
                     syn_spec={"weight": 2.0, "delay": 2.0})
        connections = nest.GetConnections(pre)
        weights = connections.get("weight")
        delays = connections.get("delay")
        self.assertEqual(weights, [2.0, 2.0])
        self.assertEqual(delays, [2.0, 2.0])

        nest.ResetKernel()
        pre = nest.Create("iaf_psc_alpha", 2)
        post = nest.Create("iaf_psc_alpha", 2)
        nest.Connect(pre, post, conn_spec={"rule": "one_to_one"},
                     syn_spec={"weight": [2.0, 3.0], "delay": [2.0, 3.0]})
        connections = nest.GetConnections(pre)
        weights = connections.get("weight")
        delays = connections.get("delay")
        self.assertEqual(weights, [2.0, 3.0])
        self.assertEqual(delays, [2.0, 3.0])

    def test_IllegalConnection(self):
        """Wrong Connections"""

        n = nest.Create('iaf_psc_alpha')
        vm = nest.Create('voltmeter')
        sr = nest.Create('spike_recorder')

        self.assertRaisesRegex(nest.kernel.NESTError, "IllegalConnection", nest.Connect, n, vm)
        self.assertRaisesRegex(nest.kernel.NESTError, "IllegalConnection", nest.Connect, sr, n)


def suite():

    suite = unittest.makeSuite(OneToOneConnectTestCase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
