# -*- coding: utf-8 -*-
#
# test_connect_array_fixed_indegree.py
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
Tests of connection with rule fixed_indegree
and parameter arrays in syn_spec
"""

import unittest
import nest
import numpy


@nest.ll_api.check_stack
class ConnectArrayFixedIndegreeTestCase(unittest.TestCase):
    """Tests of connections with fixed indegree and parameter arrays"""

    def test_Connect_Array_Fixed_Indegree(self):
        """Tests of connections with fixed indegree and parameter arrays"""

        N = 20  # number of neurons in each subnet
        K = 5   # number of connections per neuron

        ############################################
        # test with connection rule fixed_indegree
        ############################################
        nest.ResetKernel()

        net1 = nest.Create('iaf_psc_alpha', N)  # creates source subnet
        net2 = nest.Create('iaf_psc_alpha', N)  # creates target subnet

        Warr = [[y*K+x for x in range(K)] for y in range(N)]  # weight array
        Darr = [[y*K+x + 1 for x in range(K)] for y in range(N)]  # delay array

        # synapses and connection dictionaries
        syn_dict = {'synapse_model': 'static_synapse',
                    'weight': Warr, 'delay': Darr}
        conn_dict = {'rule': 'fixed_indegree', 'indegree': K}

        # connects source to target subnet
        nest.Connect(net1, net2, conn_spec=conn_dict, syn_spec=syn_dict)

        for i in range(N):  # loop on all neurons of target subnet

            # gets all connections to the target neuron
            conns = nest.GetConnections(target=net2[i:i+1])
            weight = conns.get('weight')
            delay = conns.get('delay')

            Warr1 = []  # creates empty weight array

            # loop on synapses that connect to target neuron
            for j in range(len(conns)):
                w = weight[j]  # gets synaptic weight
                d = delay[j]   # gets synaptic delay

                self.assertTrue(d - w == 1)  # checks that delay = weight + 1

                Warr1.append(w)  # appends w to Warr1

            self.assertTrue(len(Warr1) == K)  # checks the size of Warr1
            Warr1.sort()                      # sorts the elements of Warr1

            # get row of original weight array, sort it
            # and compare it with Warr1
            Warr2 = sorted(Warr[i])
            for k in range(K):
                self.assertTrue(Warr1[k]-Warr2[k] == 0.0)


def suite():

    suite = unittest.makeSuite(ConnectArrayFixedIndegreeTestCase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())

if __name__ == "__main__":
    run()
