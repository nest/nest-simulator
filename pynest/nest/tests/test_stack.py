# -*- coding: utf-8 -*-
#
# test_stack.py
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
Stack tests
"""

import unittest
import nest

from . import compatibility

from array import array

try:
    import numpy
    HAVE_NUMPY = True
except ImportError:
    HAVE_NUMPY = False


@nest.ll_api.check_stack
class StackTestCase(unittest.TestCase):
    """Stack tests"""

    def test_stack_checker(self):
        def empty_stack():
            nest.ll_api.sli_run('clear')

        def leave_on_stack():
            nest.ll_api.sli_push(1)

        check_empty_stack = nest.ll_api.stack_checker(empty_stack)
        check_leave_on_stack = nest.ll_api.stack_checker(leave_on_stack)

        debug = nest.ll_api.get_debug()
        # We have to set debug to True to check the stack
        nest.ll_api.set_debug(True)

        # This should pass without errors
        check_empty_stack()

        try:
            self.assertRaises(nest.kernel.NESTError, check_leave_on_stack)
        except:  # Ensure that debug is reset if we get an error.
            nest.ll_api.set_debug(debug)
            raise
        nest.ll_api.set_debug(debug)

    def test_Count(self):
        """Object count"""

        nest.ResetKernel()
        nest.ll_api.sr('clear')

        for i in range(100):
            nest.ll_api.sps(i)

        nest.ll_api.sr('count')
        self.assertEqual(nest.ll_api.spp(), 100)

        for i in range(100):
            self.assertEqual(nest.ll_api.spp(), (99 - i))

        nest.ll_api.sr('count')
        self.assertEqual(nest.ll_api.spp(), 0)

    def test_PushPop(self):
        """Object push and pop"""

        nest.ResetKernel()

        objects = (
            (True, ) * 2,
            (False, ) * 2,

            (1, ) * 2,
            (-100, ) * 2,

            (3.14, ) * 2,
            (-1.7588e11, ) * 2,

            ('string', ) * 2,

            # Literals should be converted to SLI literals
            (nest.kernel.SLILiteral('test'), ) * 2,

            # Arrays are converted to tuples on the way out
            ((1, 2, 3, 4, 5), ) * 2,
            ([1, 2, 3, 4, 5], (1, 2, 3, 4, 5)),

            # Dictionary round trip conversion should be consistent
            ({
                'key': 123,
                'sub_dict': {
                    nest.kernel.SLILiteral('foo'): 'bar'
                }
            }, ) * 2,
        )

        for obj_in, obj_out in objects:
            nest.ll_api.sps(obj_in)
            self.assertEqual(obj_out, nest.ll_api.spp())

    @unittest.skipIf(not HAVE_NUMPY, 'NumPy package is not available')
    def test_PushPop_NumPy(self):

        nest.ResetKernel()

        # Test support for slices and strides
        arr = numpy.array(((1, 2, 3, 4, 5), (6, 7, 8, 9, 0)))

        nest.ll_api.sps(arr[1, :])
        self.assertTrue(
            (nest.ll_api.spp() == numpy.array((6, 7, 8, 9, 0))).all())

        nest.ll_api.sps(arr[:, 1])
        self.assertTrue((nest.ll_api.spp() == numpy.array((2, 7))).all())

        # Test conversion using buffer interface
        nest.ll_api.sps(array('l', [1, 2, 3]))
        self.assertTrue((nest.ll_api.spp() == numpy.array((1, 2, 3))).all())

        nest.ll_api.sps(array('d', [1., 2., 3.]))
        self.assertTrue((nest.ll_api.spp() == numpy.array((1., 2., 3.))).all())

        # Test conversion without using buffer interface
        if hasattr(numpy, 'int16'):
            i16 = numpy.array((1, 2, 3), dtype=numpy.int16)
            nest.ll_api.sps(i16)
            self.assertTrue((nest.ll_api.spp() == i16).all())

        # Test support for scalars and zero-dimensional arrays
        a1 = numpy.array((1, 2, 3))[1]
        a2 = numpy.array((1., 2., 3.))[1]
        a3 = numpy.array(2)
        a4 = numpy.array(2.)

        for x in (a1, a3):
            nest.ll_api.sps(x)
            self.assertEqual(nest.ll_api.spp(), 2)

        for x in (a2, a4):
            nest.ll_api.sps(x)
            self.assertEqual(nest.ll_api.spp(), 2.)

    @unittest.skipIf(
        HAVE_NUMPY, 'Makes no sense when NumPy package is available')
    def test_PushPop_no_NumPy(self):

        nest.ResetKernel()

        a1 = array('i', [1, 2, 3])
        a2 = array('l', [1, 2, 3])
        a3 = array('f', [1.0, 2.0, 3.0])
        a4 = array('d', [1.0, 2.0, 3.0])

        for x in (a1, a2, a3, a4):
            nest.ll_api.sps(x)
            self.assertEqual(x, nest.ll_api.spp())


def suite():

    suite = unittest.makeSuite(StackTestCase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())

if __name__ == "__main__":
    run()
