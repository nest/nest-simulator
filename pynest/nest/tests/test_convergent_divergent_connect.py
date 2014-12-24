# -*- coding: utf-8 -*-
#
# test_convergent_divergent_connect.py
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
import sys


@nest.check_stack
class ConvergentDivergentConnectTestCase(unittest.TestCase):
    """Tests of Convergent/DivergentConnect"""


    def test_ConvergentConnect(self):
        """ConvergentConnect pre to post"""

        nest.ResetKernel()
        pre  = nest.Create("iaf_neuron", 3) 
        post = nest.Create("iaf_neuron", 1)
        nest.ConvergentConnect(pre, post)
        expected_targets = tuple(post[0] for _ in range(len(pre)))
        connections = nest.FindConnections(pre)
        targets = nest.GetStatus(connections, "target")
        self.assertEqual(expected_targets, targets)

        
    def test_ConvergentConnectWD(self):
        """ConvergentConnect pre to post with weight and delay"""

        nest.ResetKernel()
        pre  = nest.Create("iaf_neuron", 3) 
        post = nest.Create("iaf_neuron", 1)
        nest.ConvergentConnect(pre, post, weight=(2.0,2.0,2.0), delay=(1.0,2.0,3.0))
        connections = nest.FindConnections(pre)
        weights = nest.GetStatus(connections, "weight")
        delays = nest.GetStatus(connections, "delay")
        self.assertEqual(weights, (2.0,2.0,2.0))
        self.assertEqual(delays , (1.0,2.0,3.0))

        
    def test_DivergentConnect(self):
        """DivergentConnect pre to post"""

        nest.ResetKernel()
        pre  = nest.Create("iaf_neuron", 1) 
        post = nest.Create("iaf_neuron", 3)
        nest.DivergentConnect(pre, post)
        connections = nest.FindConnections(pre)
        targets = nest.GetStatus(connections, "target")
        self.assertEqual(targets, post)


    def test_DivergentConnectWD(self):
        """DivergentConnect pre to post with weight and delay"""

        nest.ResetKernel()
        pre  = nest.Create("iaf_neuron", 1) 
        post = nest.Create("iaf_neuron", 3)
        nest.DivergentConnect(pre, post, weight=(2.0,2.0,2.0), delay=(1.0,2.0,3.0))
        connections = nest.FindConnections(pre)
        weights = nest.GetStatus(connections, "weight")
        delays = nest.GetStatus(connections, "delay")
        self.assertEqual(weights, (2.0,2.0,2.0))
        self.assertEqual(delays , (1.0,2.0,3.0))


    def test_ConvergentDivergentConnectOptions(self):
        """Convergent/DivergentConnect with non-standard options and
        ensure that the option settings are properly restored before
        returning."""

        nest.ResetKernel()

        copts = nest.sli_func('GetOptions', '/RandomConvergentConnect', litconv=True)
        dopts = nest.sli_func('GetOptions', '/RandomDivergentConnect', litconv=True)

        ncopts = dict((k, not v) for k, v in copts.items() if k != 'DefaultOptions')
        ndopts = dict((k, not v) for k, v in dopts.items() if k != 'DefaultOptions')

        n = nest.Create('iaf_neuron', 3)

        nest.RandomConvergentConnect(n, n, 1, options=ncopts)
        nest.RandomDivergentConnect(n, n, 1, options=ndopts)

        opts = nest.sli_func('GetOptions', '/RandomConvergentConnect', litconv=True)
        self.assertEqual(copts, opts)

        opts = nest.sli_func('GetOptions', '/RandomDivergentConnect', litconv=True)
        self.assertEqual(dopts, opts)


def suite():

    suite = unittest.makeSuite(ConvergentDivergentConnectTestCase,'test')
    return suite

def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
