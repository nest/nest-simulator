# -*- coding: utf-8 -*-
#
# test_helper_functions.py
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


class TestHelperFunctions(unittest.TestCase):
    def test_get_verbosity(self):
        verbosity = nest.get_verbosity()
        self.assertTrue(isinstance(verbosity, nest.verbosity))

    def test_set_verbosity(self):
        for level in nest.verbosity:
            nest.set_verbosity(level)
            verbosity = nest.get_verbosity()
            self.assertEqual(verbosity, level)


def suite():
    suite = unittest.makeSuite(TestHelperFunctions, "test")
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
