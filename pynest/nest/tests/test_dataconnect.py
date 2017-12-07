# -*- coding: utf-8 -*-
#
# test_dataconnect.py
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
DataConnect
"""

import unittest
import nest


@nest.check_stack
class DataConnectTestCase(unittest.TestCase):
    """Find connections and test if values can be set."""

    def test_DataConnect(self):
        """DataConnect"""

        nest.ResetKernel()

        a = nest.Create("iaf_psc_alpha", 10)
        sources = [x for x in a[:1]]
        target = [1.0 * x for x in a[1:]]
        weight = [2.0 * x for x in target]
        delay = [1.0 * x for x in target]
        connections = [{'target': target, 'weight': weight,
                        'delay': delay} for t in target]
        nest.DataConnect(sources, connections)
        conn1 = nest.GetConnections(sources)
        stat1 = nest.GetStatus(conn1)
        target1 = [d['target'] for d in stat1]
        self.assertEqual(target, target1)

        # Now we test the connectome version of data-connect
        # by erasing and reinstantiating
        # the nestwork from the status data.
        nest.ResetKernel()
        a = nest.Create("iaf_psc_alpha", 10)
        nest.DataConnect(stat1)
        conn2 = nest.GetConnections()

        self.assertEqual(conn1, conn2)


def suite():

    suite = unittest.makeSuite(DataConnectTestCase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
