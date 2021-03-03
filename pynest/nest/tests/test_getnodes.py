# -*- coding: utf-8 -*-
#
# test_getnodes.py
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
Test GetNodes
"""

import unittest
import nest


@nest.ll_api.check_stack
class GetNodesTestCase(unittest.TestCase):
    """Test GetNodes function"""

    def setUp(self):
        nest.ResetKernel()

        a = nest.Create('iaf_psc_alpha', 3)
        b = nest.Create('iaf_psc_delta', 2, {'V_m': -77.})
        c = nest.Create('iaf_psc_alpha', 4, {'V_m': [-77., -66., -77., -66.],
                                             'tau_m': [10., 11., 12., 13.]})
        d = nest.Create('iaf_psc_exp', 4)

    def test_GetNodes(self):
        """test GetNodes"""
        all_nodes_ref = nest.NodeCollection(list(range(1, nest.GetKernelStatus('network_size') + 1)))
        all_nodes = nest.GetNodes()

        self.assertEqual(all_nodes_ref, all_nodes)

    def test_GetNodes_with_params(self):
        """test GetNodes with params"""
        nodes_Vm = nest.GetNodes({'V_m': -77.})
        nodes_Vm_ref = nest.NodeCollection([4, 5, 6, 8])

        self.assertEqual(nodes_Vm_ref, nodes_Vm)

        nodes_Vm_tau = nest.GetNodes({'V_m': -77., 'tau_m': 12.})
        nodes_Vm_tau_ref = nest.NodeCollection([8])

        self.assertEqual(nodes_Vm_tau_ref, nodes_Vm_tau)

        nodes_exp = nest.GetNodes({'model': 'iaf_psc_exp'})
        nodes_exp_ref = nest.NodeCollection([10, 11, 12, 13])

        self.assertEqual(nodes_exp_ref, nodes_exp)


def suite():

    suite = unittest.makeSuite(GetNodesTestCase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
