# -*- coding: utf-8 -*-
#
# test_regression_issue-2125.py
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

import unittest

import nest

HAVE_THREADS = nest.build_info["have_threads"]


@unittest.skipIf(not HAVE_THREADS, "NEST was compiled without multi-threading")
class ThreadedDisconnectTestCase(unittest.TestCase):
    def test_threaded_disconnect(self):
        """Test that threaded disconnect does not produce segmentation fault"""
        nest.ResetKernel()
        nest.verbosity = nest.VerbosityLevel.ERROR
        nest.local_num_threads = 2

        neurons = nest.Create("iaf_psc_alpha", 3)

        nest.Connect(neurons[0], neurons[2])

        conns = nest.GetConnections()
        self.assertEqual(len(conns), 1)

        # Make sure we are able to call Disconnect when we have number of threads more than one
        nest.Disconnect(neurons[0], neurons[2])

        conns = nest.GetConnections()
        self.assertEqual(len(conns), 0)


def suite():
    suite = unittest.makeSuite(ThreadedDisconnectTestCase, "test")
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
