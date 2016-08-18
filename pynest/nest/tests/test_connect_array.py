# -*- coding: utf-8 -*-
#
# test_connect_array.py
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
Tests of connection with rules fixed_indegree or fixed_outdegree
and parameter arrays in syn_spec
"""

import unittest
import nest

@nest.check_stack
class ConnectArrayTestCase(unittest.TestCase):
    """Tests of connections with parameter arrays"""

    def test_Connect_Array(self):
        """Tests of connections with parameter arrays"""
        nest.ResetKernel()

        N = 10
        K = 3
        neuron1 = nest.Create('iaf_neuron', N)
        neuron2 = nest.Create('iaf_neuron', N)

        Warr = [[y*10+x for x in range(K)] for y in range(N)]
        Darr = Warr/10.0

        syn_dict = {'model': 'static_synapse', 'weight': Warr}
        conn_dict = {'rule': 'fixed_indegree', 'indegree':K}

        nest.Connect(neuron1, neuron2, conn_spec=conn_dict, syn_spec=syn_dict)

        w=[0.0, 0.0, 0.0]
        d=[0.0, 0.0, 0.0]
        for i in range(N):
            conns=nest.GetConnections(target=neuron2[i:i+1])
            for j in range(len(conns)):
                c=conns[j:j+1]
                w[j] = nest.GetStatus(c,'weight')[0]
                d[j] = nest.GetStatus(c,'delay')[0]
                self.assertTrue(d[j]==w[j]/10.0)
                w1=w[j]-10.0*i
                self.assertTrue(w1==0.0 or w1==1.0 or w1==2.0)

            self.assertTrue(w[0]!=w[1] and w[0]!=w[2] and w[1]!=w[2])

        nest.ResetKernel()
        N = 10
        K = 3
        neuron1 = nest.Create('iaf_neuron', N)
        neuron2 = nest.Create('iaf_neuron', N)

        Warr = [[y*10+x for x in range(K)] for y in range(N)]
        Darr = Warr/10.0

        syn_dict = {'model': 'static_synapse', 'weight': Warr, 'delay': Darr}
        conn_dict = {'rule': 'fixed_outdegree', 'outdegree':K}

        nest.Connect(neuron1, neuron2, conn_spec=conn_dict, syn_spec=syn_dict)

        w=[0.0, 0.0, 0.0]
        d=[0.0, 0.0, 0.0]
        for i in range(N):
            conns=nest.GetConnections(source=neuron1[i:i+1])
            for j in range(len(conns)):
                c=conns[j:j+1]
                w[j] = nest.GetStatus(c,'weight')[0]
                d[j] = nest.GetStatus(c,'delay')[0]
                self.assertTrue(d[j]==w[j]/10.0)
                w1=w[j]-10.0*i
                self.assertTrue(w1==0.0 or w1==1.0 or w1==2.0)

            self.assertTrue(w[0]!=w[1] and w[0]!=w[2] and w[1]!=w[2])

def suite():

    suite = unittest.makeSuite(ConnectArrayTestCase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())

if __name__ == "__main__":
    run()
