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
        self.assertTrue(isinstance(verbosity, int))

    def test_set_verbosity(self):
        levels = [('M_ALL', 0),
                  ('M_DEBUG', 5),
                  ('M_STATUS', 7),
                  ('M_INFO', 10),
                  ('M_DEPRECATED', 18),
                  ('M_WARNING', 20),
                  ('M_ERROR', 30),
                  ('M_FATAL', 40),
                  ('M_QUIET', 100)
                  ]
        for level, code in levels:
            nest.set_verbosity(level)
            verbosity = nest.get_verbosity()
            self.assertEqual(verbosity, code)


def suite():
    suite = unittest.makeSuite(TestHelperFunctions, 'test')
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
