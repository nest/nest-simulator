# -*- coding: utf-8 -*-
#
# compatibility.py
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
import numpy
import inspect


def _skipIf(condition, _):
    """
    @unittest.skipIf() decorator stub for Python 2.6-
    """

    def decorator(test_item):
        if inspect.isclass(test_item) and \
           issubclass(test_item, unittest.TestCase):
            test_item = type("DummyTestCase", (unittest.TestCase, ), {})
        elif inspect.isfunction(test_item):
            def ret_none(obj):
                return None
            test_item = ret_none
        else:
            raise ValueError("unable to decorate {0}".format(test_item))
        return test_item

    if condition:
        return decorator
    else:
        return lambda obj: obj


# Python 2.6-
if not hasattr(unittest, 'skipIf'):
    unittest.skipIf = _skipIf

# Python 3.2+
if hasattr(unittest.TestCase, 'assertRaisesRegex'):
    pass
# Python 2.7 - 3.1
elif hasattr(unittest.TestCase, 'assertRaisesRegexp'):
    unittest.TestCase.assertRaisesRegex = unittest.TestCase.assertRaisesRegexp
# Python 2.6-
else:
    unittest.TestCase.assertRaisesRegex = lambda *args, **kwargs: None

# Python 2.6-
if not hasattr(unittest.TestCase, 'assertIsInstance'):
    unittest.TestCase.assertIsInstance = lambda self, obj, cls, **kwargs: \
        unittest.TestCase.assertTrue(self, isinstance(obj, cls), **kwargs)

# Python 2.6-
if not hasattr(unittest.TestCase, 'assertGreater'):
    unittest.TestCase.assertGreater = lambda self, a, b, **kwargs: \
        unittest.TestCase.assertTrue(self, a > b, **kwargs)

# NumPy 1.4.0-
if not hasattr(numpy, 'fill_diagonal'):
    def fill_diagonal(a, b):
        diagonal = [numpy.arange(min(a.shape))] * a.ndim
        a[diagonal] = b
    numpy.fill_diagonal = fill_diagonal
