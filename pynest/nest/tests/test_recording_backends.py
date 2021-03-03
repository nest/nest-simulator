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

HAVE_SIONLIB = nest.ll_api.sli_func("statusdict/have_sionlib ::")


class TestRecordingBackends(unittest.TestCase):

    def testAAAResetKernel(self):
        """Test reset of recording backend defaults.

        As ResetKernel is used by many of the tests in this file to
        ensure a consistent initial state, this test has to be run
        before all others. This is ensured by prefix "AAA" in front of
        the name. See https://docs.python.org/3/library/unittest.html
        for details on test execution order.
        """

        nest.ResetKernel()

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
        rb_defaults = [mm_defaults[k] for k in rb_properties]

        self.assertEqual(rb_defaults_initial, rb_defaults)

    def testDefaultBackendsAvailable(self):
        """Test availability of default recording backends."""

        nest.ResetKernel()

        backends = nest.GetKernelStatus("recording_backends")
        expected_backends = ("ascii", "memory", "screen")

        self.assertTrue(all([b in backends for b in expected_backends]))

        if HAVE_SIONLIB:
            self.assertTrue("sionlib" in backends)

    def testGlobalRecordingBackendProperties(self):
        """Test setting of global backend properties.

        This sets and gets global backend properties and only runs if
        compiled with SIONlib, as the corresponding backend is the
        only backend with global parameters.
        """

        nest.ResetKernel()

        if HAVE_SIONLIB:
            rb_status = nest.GetKernelStatus("recording_backends")
            chunksize = rb_status["sionlib"]["sion_chunksize"]
            sl_params = {"sionlib": {"sion_chunksize": chunksize + 1}}
            nest.SetKernelStatus({"recording_backends": sl_params})

            rb_status = nest.GetKernelStatus("recording_backends")
            sl_chunksize = rb_status["sionlib"]["sion_chunksize"]
            self.assertEqual(sl_chunksize, chunksize + 1)

    def testSetDefaultRecordingBackend(self):
        """Test setting the default recording backend.

        Test if setting another default recording backend for a
        recording device works and does not influence the default
        backend of all other recording devices.
        """

        nest.ResetKernel()

        nest.SetDefaults("multimeter", {"record_to": "ascii"})
        rb_defaults_mm = nest.GetDefaults("multimeter")["record_to"]
        rb_defaults_sr = nest.GetDefaults("spike_recorder")["record_to"]

        self.assertEqual(rb_defaults_mm, "ascii")
        self.assertEqual(rb_defaults_sr, "memory")

    def testSetDefaultsRecordingBackendProperties(self):
        """Test setting recording backend defaults."""

        nest.ResetKernel()

        sr_defaults = {"record_to": "ascii", "file_extension": "nest"}
        nest.SetDefaults("spike_recorder", sr_defaults)
        sr_defaults = nest.GetDefaults("spike_recorder")

        self.assertEqual(sr_defaults["record_to"], "ascii")
        self.assertEqual(sr_defaults["file_extension"], "nest")

    def testRecordingBackendDefaultsToInstances(self):
        """Test that backend defaults end up in instances.

        Also check that that backend defaults set on one model and
        don't influence other instances even though they use the same
        backend
        """

        nest.ResetKernel()

        mm_defaults = {"record_to": "ascii", "file_extension": "multimeter"}
        nest.SetDefaults("multimeter", mm_defaults)

        mm_status = nest.Create("multimeter").get()
        self.assertEqual(mm_status["record_to"], "ascii")
        self.assertEqual(mm_status["file_extension"], "multimeter")

        nest.SetDefaults("spike_recorder", {"record_to": "ascii"})
        sr_status = nest.Create("spike_recorder").get()
        self.assertEqual(sr_status["record_to"], "ascii")
        self.assertEqual(sr_status["file_extension"], "dat")


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestRecordingBackends)
    return suite


if __name__ == '__main__':
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
