# -*- coding: utf-8 -*-
#
# test_recording_backend_memory.py
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
import numpy as np

HAVE_THREADS = nest.build_info["have_threads"]


@unittest.skipIf(not HAVE_THREADS, "NEST was compiled without multi-threading")
class TestRecordingBackendMemory(unittest.TestCase):
    def testEventsDict(self):
        """Test if the event dict is there from the start."""

        nest.ResetKernel()

        mm = nest.Create("multimeter", params={"record_to": "memory"})
        events = mm.events  # noqa: F841

    def testEventCounter(self):
        """Test that n_events counts the number of events correctly."""

        nest.ResetKernel()

        mm = nest.Create("multimeter", params={"record_to": "memory"})
        mm.set({"interval": 0.1, "record_from": ["V_m"]})
        nest.Connect(mm, nest.Create("iaf_psc_alpha"))

        nest.Simulate(15)
        self.assertEqual(mm.n_events, 140)
        self.assertEqual(len(mm.events["times"]), 140)

        nest.Simulate(1)
        self.assertEqual(mm.n_events, 150)
        self.assertEqual(len(mm.events["times"]), 150)

        # Now with multithreading

        nest.ResetKernel()
        nest.local_num_threads = 2

        mm = nest.Create("multimeter", params={"record_to": "memory"})
        mm.set({"interval": 0.1, "record_from": ["V_m"]})
        nest.Connect(mm, nest.Create("iaf_psc_alpha", 2))

        nest.Simulate(15)
        self.assertEqual(mm.n_events, 280)
        self.assertEqual(len(mm.events["times"]), 280)

        nest.Simulate(1)
        self.assertEqual(mm.n_events, 300)
        self.assertEqual(len(mm.events["times"]), 300)

    def testResetEventCounter(self):
        """"""

        nest.ResetKernel()

        mm = nest.Create("multimeter", params={"record_to": "memory"})
        mm.set({"interval": 0.1, "record_from": ["V_m"]})
        nest.Connect(mm, nest.Create("iaf_psc_alpha"))

        nest.Simulate(15)

        # Check that an error is raised when setting n_events to a number != 0
        with self.assertRaises(nest.NESTError):
            mm.n_events = 10

        # Check that the event counter was indeed not changed and the
        # events dictionary is still intact
        self.assertEqual(mm.n_events, 140)
        self.assertEqual(len(mm.events["times"]), 140)

        # Check that the events dict is cleared when setting n_events to 0
        mm.n_events = 0
        self.assertEqual(len(mm.events["times"]), 0)

    def testTimeInSteps(self):
        """"""

        nest.ResetKernel()

        mm = nest.Create("multimeter", params={"record_to": "memory"})

        # Check that time_in_steps is set False by default
        self.assertFalse(mm.time_in_steps)

        # Check times are in float (i.e. ms) and offsets are not there
        # if time_in_steps == False
        self.assertEqual(mm.events["times"].dtype, "float64")
        self.assertFalse("offsets" in mm.events)

        # Check times are in int (i.e.steps) and offsets are there and of
        # type float if time_in_steps == True
        mm.time_in_steps = True
        self.assertTrue(all(isinstance(e, int) for e in mm.events["times"]))

        # Check that time_in_steps cannot be set after Simulate has
        # been called.
        nest.Simulate(10)
        with self.assertRaises(nest.NESTError):
            mm.time_in_steps = False


def suite():
    suite = unittest.TestLoader()
    suite = suite.loadTestsFromTestCase(TestRecordingBackendMemory)
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
