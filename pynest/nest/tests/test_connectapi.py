#! /usr/bin/env python
#
# test_connectapi.py
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

class Bench(object):

    def __init__(self,g,l):
        self.g = g
        self.l = l

    def __call__(self,cmd,repeat=5):
        from time import time
        t = 0
        for i in range(repeat):
            t1 = time()
            exec cmd in self.g, self.l
            t2 = time()
            t += t2-t1

        print 'Executed "%s".  Elapsed = %f s' % (cmd,t/repeat)
        

class ConnectAPITestCase(unittest.TestCase):
    """Tests of the Connect API"""

    def test_ConnectPrePost(self):
        """Connect pre to post"""

        # Connect([pre], [post])
        nest.ResetKernel()        
        pre = nest.Create("iaf_neuron", 2)
        post = nest.Create("iaf_neuron", 2)
        nest.Connect(pre, post)
        connections = nest.FindConnections(pre)
        targets = nest.GetStatus(connections, "target")
        self.assertEqual(targets, post)


    def test_ConnectPrePostParams(self):
        """Connect pre to post with a params dict"""

        # Connect([pre], [post], params)
        nest.ResetKernel()        
        pre = nest.Create("iaf_neuron", 2)
        post = nest.Create("iaf_neuron", 2)
        nest.Connect(pre, post, {"weight" : 2.0})
        connections = nest.FindConnections(pre)
        weights = nest.GetStatus(connections, "weight")
        self.assertEqual(weights, [2.0, 2.0])

        # Connect([pre], [post], [params])
        nest.ResetKernel()        
        pre = nest.Create("iaf_neuron", 2)
        post = nest.Create("iaf_neuron", 2)
        nest.Connect(pre, post, [{"weight" : 2.0}])
        connections = nest.FindConnections(pre)
        weights = nest.GetStatus(connections, "weight")
        self.assertEqual(weights, [2.0, 2.0])

        # Connect([pre], [post], [params, params])
        nest.ResetKernel()        
        pre = nest.Create("iaf_neuron", 2)
        post = nest.Create("iaf_neuron", 2)
        nest.Connect(pre, post, [{"weight" : 2.0}, {"weight" : 3.0}])
        connections = nest.FindConnections(pre)
        weights = nest.GetStatus(connections, "weight")
        self.assertEqual(weights, [2.0, 3.0])


    def test_ConnectPrePostWD(self):
        """Connect pre to post with a weight and delay"""

        # Connect([pre], [post], w, d)
        nest.ResetKernel()        
        pre = nest.Create("iaf_neuron", 2)
        post = nest.Create("iaf_neuron", 2)
        nest.Connect(pre, post, 2.0, 2.0)
        connections = nest.FindConnections(pre)
        weights = nest.GetStatus(connections, "weight")
        self.assertEqual(weights, [2.0, 2.0])

        # Connect([pre], [post], [w], [d])
        nest.ResetKernel()
        pre = nest.Create("iaf_neuron", 2)
        post = nest.Create("iaf_neuron", 2)
        nest.Connect(pre, post, [2.0], [2.0])
        connections = nest.FindConnections(pre)
        weights = nest.GetStatus(connections, "weight")
        delays = nest.GetStatus(connections, "delay")
        self.assertEqual(weights, [2.0, 2.0])
        self.assertEqual(delays, [2.0, 2.0])

        # Connect([pre], [post], [w, w], [d, d])
        nest.ResetKernel()
        pre = nest.Create("iaf_neuron", 2)
        post = nest.Create("iaf_neuron", 2)
        nest.Connect(pre, post, [2.0, 3.0], [2.0, 3.0])
        connections = nest.FindConnections(pre)
        weights = nest.GetStatus(connections, "weight")
        delays = nest.GetStatus(connections, "delay")
        self.assertEqual(weights, [2.0, 3.0])
        self.assertEqual(delays, [2.0, 3.0])

        
    def test_ConvergentConnect(self):
        """ConvergentConnect pre to post"""

        nest.ResetKernel()
        pre  = nest.Create("iaf_neuron", 3) 
        post = nest.Create("iaf_neuron", 1)
        nest.ConvergentConnect(pre, post)
        expected_targets = [post[0] for x in range(len(pre))]
        connections = nest.FindConnections(pre)
        targets = nest.GetStatus(connections, "target")
        self.assertEqual(expected_targets, targets)

        
    def test_ConvergentConnectWD(self):
        """ConvergentConnect pre to post with weight and delay"""

        nest.ResetKernel()
        pre  = nest.Create("iaf_neuron", 3) 
        post = nest.Create("iaf_neuron", 1)
        nest.ConvergentConnect(pre, post, weight=[2.0,2.0,2.0], delay=[1.0,2.0,3.0])
        connections = nest.FindConnections(pre)
        weights = nest.GetStatus(connections, "weight")
        delays = nest.GetStatus(connections, "delay")
        self.assertEqual(weights, [2.0,2.0,2.0])
        self.assertEqual(delays , [1.0,2.0,3.0])

        
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
        nest.DivergentConnect(pre, post, weight=[2.0,2.0,2.0], delay=[1.0,2.0,3.0])
        connections = nest.FindConnections(pre)
        weights = nest.GetStatus(connections, "weight")
        delays = nest.GetStatus(connections, "delay")
        self.assertEqual(weights, [2.0,2.0,2.0])
        self.assertEqual(delays , [1.0,2.0,3.0])


    def test_WrongConnection(self):
        """Wrong Connections"""

        nest.ResetKernel()
        n  = nest.Create('iaf_neuron')
        vm = nest.Create('voltmeter')
        sd = nest.Create('spike_detector')

        try:
            nest.Connect(n,vm)
            self.fail() # should not be reached
        except nest.NESTError:
            info = sys.exc_info()[1]
            if not "IllegalConnection" in info.__str__():
                self.fail()              
        # another error has been thrown, this is wrong
        except: 
          self.fail()  
            

    def test_UnexcpectedEvent(self):
        """Unexpected Event"""

        nest.ResetKernel()
        n  = nest.Create('iaf_neuron')
        vm = nest.Create('voltmeter')
        sd = nest.Create('spike_detector')

        try:
            nest.Connect(sd,n)
            self.fail() # should not be reached
        except nest.NESTError:
            info = sys.exc_info()[1]
            if not "UnexpectedEvent" in info.__str__():
                self.fail()              
        # another error has been thrown, this is wrong
        except: 
          self.fail()


def suite():

    suite = unittest.makeSuite(ConnectAPITestCase,'test')
    return suite


if __name__ == "__main__":

    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
