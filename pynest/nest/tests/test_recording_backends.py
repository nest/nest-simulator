# -*- coding: utf-8 -*-
#
# test_recording_backends.py
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

HAVE_MPI = nest.ll_api.sli_func("statusdict/have_mpi ::")
HAVE_SIONLIB = nest.ll_api.sli_func("statusdict/have_sionlib ::")


class TestRecordingBackends(unittest.TestCase):


    def testAAAResetKernel(self):
        """Test proper reset of backend defaults by ResetKernel.

        As ResetKernel is used by many of the tests in this file to
        ensure a consistent initial state, this test has to be run
        before all others. This is ensured by prefix "AAA" in front of
        the name. See https://docs.python.org/3/library/unittest.html
        for details on test execution order.

        """

        mm_defaults = nest.GetDefaults("multimeter")
        rb_properties = ["record_to", "time_in_steps"]
        rb_defaults_initial = [mm_defaults[k] for k in rb_properties]

        self.assertEqual(rb_defaults_initial, ["memory", False])

        mm_defaults = {"record_to": "ascii", "time_in_steps": True}
        nest.SetDefaults("multimeter", mm_defaults)

        mm_defaults = nest.GetDefaults("multimeter")
        rb_properties = ["record_to", "time_in_steps"]
        rb_defaults = [mm_defaults[k] for k in rb_properties]

        self.assertEqual(rb_defaults, ["ascii", True])

        nest.ResetKernel()

        mm_defaults = nest.GetDefaults("multimeter")
        rb_properties = ["record_to", "time_in_steps"]

        self.assertEqual(rb_defaults_initial, rb_defaults)


    def testDefaultBackendsAvailable(self):
        """Test availability of default backends.

        """

        nest.ResetKernel()

        backends = nest.GetKernelStatus("recording_backends")
        expected_backends = ("ascii", "memory", "screen")

        self.assertTrue(all([b in backends for b in expected_backends]))

        if HAVE_SIONLIB:
            self.assertTrue("sionlib" in backends)


    def testGlobalRecordingBackendProperties(self):
        """
        If compiled with SIONlib, set and get global backend properties on the
        corresponding backend, as that is the only backend which has them.
        """

        nest.ResetKernel()

        if HAVE_SIONLIB:
            # sl_params = {"sionlib": {"sion_chunksize": 512 }}
            # nest.SetKernelStatus({"recording_backends": sl_params})
            pass


    def testSetDefaultRecordingBackend(self):
        """
        Test if setting another default recording backend for a recording
        device as works and does not influence the default backend of all
        other recording devices.
        """

        nest.ResetKernel()

        nest.SetDefaults("multimeter", {"record_to": "ascii"})
        rb_defaults_mm = nest.GetDefaults("multimeter")["record_to"]
        rb_defaults_sd = nest.GetDefaults("spike_detector")["record_to"]

        self.assertEqual(rb_defaults_mm, "ascii")
        self.assertEqual(rb_defaults_sd, "memory")


    def testSetDefaultsRecordingBackendProperties(self):
        """
        Test if setting recording backend default properties works.
        """

        nest.ResetKernel()

        sd_defaults = {"record_to": "ascii", "file_extension": "nest"}
        nest.SetDefaults("spike_detector", sd_defaults)
        sd_defaults = nest.GetDefaults("spike_detector")

        self.assertEqual(sd_defaults["record_to"], "ascii")
        self.assertEqual(sd_defaults["file_extension"], "nest")


    def testRecordingBackendDefaultsToInstances(self):
        """
        Make sure that backend defaults end up in instances and don't
        influence other instances even though they use the same backend
        """

        nest.ResetKernel()

        mm_defaults = {"record_to": "ascii", "file_extension": "multimeter"}
        nest.SetDefaults("multimeter", mm_defaults)

        mm_status = nest.Create("multimeter").get()
        self.assertEqual(mm_status["record_to"], "ascii")
        self.assertEqual(mm_status["file_extension"], "multimeter")

        nest.SetDefaults("spike_detector", {"record_to": "ascii"})
        sd_status = nest.Create("spike_detector").get()
        self.assertEqual(sd_status["record_to"], "ascii")
        self.assertEqual(sd_status["file_extension"], "dat")


    def testRecordingBackendMemory(self):
        """
        - Check if the event dict is there from the start
        - Check if the n_events is set correctly
        - Check if the events dict is cleared when setting n_events to 0
          and that an error is thrown if it is set to another number
        - Check if time_in_steps works properly
        - Check it ResetKernel deletes data from memory backend
        """
        pass


    def testRecordingBackendASCII(self):
        """
        - Check if data_prefix and data_path end up in the filenames list
        - Check if setting the file extension works
        - Check if time_in_steps works properly
        """
        pass


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestRecordingBackends)
    return suite

if __name__ == '__main__':
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
