#! /usr/bin/env python
#
# decorators.py
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
import sys

class _skipIf(object):
    """
    @unittest.skipIf() decorator that works for Python 2.6-
    """

    def __init__(self, condition, reason, obj_type = 'unittest'):

        self.condition = condition
        self.reason = reason
        self.obj_type = obj_type

    def __call__(self, obj):

        if sys.version_info >= (2, 7, 0):

            return unittest.skipIf(self.condition, self.reason)(obj)

        else:

            if not self.condition:
                return obj
            else:
                if self.obj_type == 'testcase':
                    class dummy(unittest.TestCase): pass
                else:
                    def dummy(_): pass

                return dummy
