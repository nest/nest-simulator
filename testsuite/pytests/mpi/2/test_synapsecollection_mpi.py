# -*- coding: utf-8 -*-
#
# test_synapsecollection_mpi.py
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

import nest
import unittest


class TestSynapsecollectionMpi(unittest.TestCase):

    def setUp(self):
        nest.ResetKernel()

    def testTooFewConnections(self):
        """SynapseCollection with too few connections"""

        pre = nest.Create('iaf_psc_alpha', 5)
        post = nest.Create('iaf_psc_alpha', 1)

        nest.Connect(pre, post)  # only one process will have a connection

        conns = nest.GetConnections()
        print(conns)


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(unittest.TestLoader().loadTestsFromTestCase(TestSynapsecollectionMpi))


if __name__ == '__main__':
    run()
