# -*- coding: utf-8 -*-
#
# test_use_gid_in_filename.py
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
Test if use_gid_in_filename works properly as a parameter of recording devices
"""

import unittest
import nest
import os


@nest.ll_api.check_stack
class UseGidInFilenameTestCase(unittest.TestCase):
    """Tests of use_gid_in_filename"""

    def test_WithGid(self):
        """With GID"""

        nest.ResetKernel()
        nest.SetKernelStatus({"overwrite_files": True})

        sd = nest.Create("spike_detector", 1, {"to_file": True,
                         "label": "sd", "use_gid_in_filename": True})
        nest.Simulate(100.0)

        filename = nest.GetStatus(sd, "filenames")[0][0].split("/")[-1]
        self.assertEqual(filename, "sd-1-0.gdf")

    def test_NoGid(self):
        """Without GID"""

        nest.ResetKernel()
        nest.SetKernelStatus({"overwrite_files": True})

        sd = nest.Create("spike_detector", 1, {"to_file": True,
                         "label": "sd", "use_gid_in_filename": False})
        nest.Simulate(100.0)

        filename = nest.GetStatus(sd, "filenames")[0][0].split("/")[-1]
        self.assertEqual(filename, "sd-0.gdf")


def suite():
    suite = unittest.makeSuite(UseGidInFilenameTestCase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
