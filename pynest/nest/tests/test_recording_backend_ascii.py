# -*- coding: utf-8 -*-
#
# test_recording_backend_ascii.py
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

import os
import unittest
import nest


class TestRecordingBackendASCII(unittest.TestCase):

    def testAAAOverwriteFiles(self):

        nest.ResetKernel()

        mm_params = {"record_to": "ascii", "record_from": ["V_m"]}
        mm = nest.Create("multimeter", params=mm_params)

        try:
            os.remove(mm.get("filenames")[0])
        except FileNotFoundError:
            pass

        nest.Connect(mm, nest.Create("iaf_psc_alpha"))
        nest.Simulate(100)

        nest.ResetKernel()

        mm = nest.Create("multimeter", params=mm_params)
        nest.Connect(mm, nest.Create("iaf_psc_alpha"))

        with self.assertRaises(nest.kernel.NESTErrors.IOError):
            nest.Simulate(100)

        nest.Cleanup()

        nest.SetKernelStatus({"overwrite_files": True})
        nest.Simulate(100)

    def testDataPrefixDataPathAndFilenameExtension(self):
        """Test if filename is corrrectly built.

        This also tests that data_prefix, data_path, file extension
        end up in filenames.
        """

        nest.ResetKernel()

        data_prefix = "data_prefix"
        data_path = "/tmp/"
        file_extension = "nest"

        kernel_params = {
            "data_prefix": data_prefix,
            "data_path": data_path,
            "overwrite_files": True,
        }
        nest.SetKernelStatus(kernel_params)

        mm_params = {
            "record_to": "ascii",
            "record_from": ["V_m"],
            "file_extension": file_extension,
        }
        mm = nest.Create("multimeter", mm_params)
        fname = mm.get("filenames")[0]

        self.assertTrue(data_path in fname)
        self.assertTrue(data_prefix in fname)
        self.assertTrue(fname.endswith(file_extension))

    def testLabel(self):
        """Test that label replaces the model name in the file name if set."""

        nest.ResetKernel()

        label = "label"

        kernel_params = {
            "overwrite_files": True,
        }
        nest.SetKernelStatus(kernel_params)

        mm_params = {
            "record_to": "ascii",
            "record_from": ["V_m"],
            "label": label,
        }
        mm = nest.Create("multimeter", mm_params)
        fname = mm.get("filenames")[0]

        self.assertTrue(label in fname)
        self.assertTrue(mm.get("model") not in fname)

    def testFileContent(self):
        """Test if the file contains correct headers and expected content"""

        nest.ResetKernel()
        nest.SetKernelStatus({"overwrite_files": True})

        mm = nest.Create("multimeter", params={"record_to": "ascii"})
        mm.set({"interval": 0.1, "record_from": ["V_m"]})
        nest.Connect(mm, nest.Create("iaf_psc_alpha"))

        nest.Simulate(15)

        fname = mm.get("filenames")[0]
        with open(fname) as f:
            lines = f.readlines()

            self.assertEqual(len(lines), mm.get("n_events")+3)

            nest.ll_api.sr("statusdict/version ::")
            version = nest.ll_api.spp()
            self.assertEqual(lines[0], "# NEST version: {}\n".format(version))

            header_2 = "# RecordingBackendASCII version:"
            self.assertTrue(lines[1].startswith(header_2))

            self.assertEqual(lines[2], "# sender\ttime_ms\tV_m\n")

            self.assertEqual(lines[3], "2\t0.100\t-70.000\n")

    def testEventCounter(self):
        """Test that n_events counts the number of events correctly."""

        nest.ResetKernel()
        nest.SetKernelStatus({"overwrite_files": True})

        mm = nest.Create("multimeter", params={"record_to": "ascii"})
        mm.set({"interval": 0.1, "record_from": ["V_m"]})
        nest.Connect(mm, nest.Create("iaf_psc_alpha"))

        nest.Simulate(15)
        self.assertEqual(mm.get("n_events"), 140)

        nest.Simulate(1)
        self.assertEqual(mm.get("n_events"), 150)

        # Now with multithreading

        nest.ResetKernel()
        kernel_params = {"overwrite_files": True, "local_num_threads": 2}
        nest.SetKernelStatus(kernel_params)

        mm = nest.Create("multimeter", params={"record_to": "ascii"})
        mm.set({"interval": 0.1, "record_from": ["V_m"]})
        nest.Connect(mm, nest.Create("iaf_psc_alpha", 2))

        nest.Simulate(15)
        self.assertEqual(mm.get("n_events"), 280)

        nest.Simulate(1)
        self.assertEqual(mm.get("n_events"), 300)

    def testResetEventCounter(self):
        """"""

        nest.ResetKernel()
        nest.SetKernelStatus({"overwrite_files": True})

        mm = nest.Create("multimeter", params={"record_to": "ascii"})
        mm.set({"interval": 0.1, "record_from": ["V_m"]})
        nest.Connect(mm, nest.Create("iaf_psc_alpha"))

        nest.Simulate(15)

        # Check that an error is raised when setting n_events to a number != 0
        with self.assertRaises(nest.kernel.NESTErrors.BadProperty):
            mm.n_events = 10

        # Check that the event counter was indeed not changed
        self.assertEqual(mm.get("n_events"), 140)

        # Check that the events dict is cleared when setting n_events to 0
        mm.n_events = 0
        self.assertEqual(mm.get("n_events"), 0)

    def testTimeInSteps(self):
        """Check if time_in_steps works properly."""

        nest.ResetKernel()
        nest.SetKernelStatus({"overwrite_files": True})

        mm = nest.Create("multimeter", params={"record_to": "ascii"})

        # Check that time_in_steps is set False by default
        self.assertFalse(mm.get("time_in_steps"))

        mm.set({"record_from": ["V_m"], "time_in_steps": True})
        nest.Connect(mm, nest.Create("iaf_psc_alpha"))

        nest.Simulate(15)

        fname = mm.get("filenames")[0]
        with open(fname) as f:
            lines = f.readlines()
            h3_expected = "# sender\ttime_step\ttime_offset\tV_m\n"
            self.assertEqual(lines[2], h3_expected)


def suite():
    suite = unittest.TestLoader()
    suite = suite.loadTestsFromTestCase(TestRecordingBackendASCII)
    return suite


if __name__ == '__main__':
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
