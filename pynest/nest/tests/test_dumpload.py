# -*- coding: utf-8 -*-
#
# test_dumpload.py
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
Dump and Load tests
"""

import unittest
import nest


class DumpLoadTestCase(unittest.TestCase):

    def setUp(self) -> None:
        nest.set_verbosity("M_ERROR")
        nest.ResetKernel()

    def test_dump(self):
        nest.ResetKernel()
        a = nest.Create("aeif_psc_alpha", 5)
        b = nest.Create("aeif_psc_alpha", 5)
        nest.Connect(a, b)
        thedump = nest.Dump()
        onlysynapses = nest.Dump(selections=["synapses"])
        onlynodes = nest.Dump(selections=["nodes"])
        self.assertIsNotNone(thedump)
        self.assertEqual(len(onlysynapses), 1)
        self.assertEqual(len(onlynodes), 1)
        c = nest.Create("aeif_psc_alpha", 5)
        dump2 = nest.Dump()
        self.assertNotEqual(thedump, dump2)

    def test_dump_emtpy(self):
        nest.ResetKernel()
        wholedump = nest.Dump()
        self.assertEqual(wholedump, {})
        nodes = nest.Dump(selections=["nodes"])
        self.assertEqual(nodes, {})

    def test_load_repeated(self):
        # checks for growing network size
        nest.ResetKernel()
        a = nest.Create("aeif_psc_alpha", 5)
        b = nest.Create("aeif_psc_alpha", 5)
        nest.Connect(a, b)
        thedump = nest.Dump()
        numload0 = nest.GetKernelStatus("network_size")
        # no reset load twice
        result = nest.Load(thedump)
        numload1 = nest.GetKernelStatus("network_size")
        self.assertGreater(numload1, numload0)
        result = nest.Load(thedump)
        numload2 = nest.GetKernelStatus("network_size")
        self.assertGreater(numload2, numload1)

    def test_load_empty(self):
        nest.ResetKernel()
        numbefore = nest.GetKernelStatus("network_size")
        syns = nest.Load({})
        numafter = nest.GetKernelStatus("network_size")
        self.assertEqual(numbefore, numafter)

    def test_integration(self):
        nest.ResetKernel()
        a = nest.Create("aeif_psc_alpha", 5)
        b = nest.Create("aeif_psc_alpha", 5)
        nest.Connect(a, b)
        thedump = nest.Dump()
        netsizebefore = nest.GetKernelStatus("network_size")
        connsize_before = nest.GetKernelStatus("num_connections")
        nest.ResetKernel()
        result = nest.Load(thedump)
        netsize_after = nest.GetKernelStatus("network_size")
        connsize_after = nest.GetKernelStatus("num_connections")
        self.assertEqual(netsizebefore, netsize_after)
        self.assertEqual(connsize_before, connsize_after)


# --------------------------------------------------------------------------- #
#  Run the comparisons
# --------------------------------------------------------------------------- #

def suite():
    return unittest.makeSuite(DumpLoadTestCase, "test")


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == '__main__':
    run()
