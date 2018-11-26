# -*- coding: utf-8 -*-
#
# test_errors.py
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
Tests for error handling
"""

import unittest
import nest


@nest.hl_api.check_stack
class ErrorTestCase(unittest.TestCase):
    """Tests if errors are handled correctly"""

    def test_Raise(self):
        """Error raising"""

        def raise_custom_exception(exc, msg):
            raise exc(msg)

        message = "test"
        exception = nest.ll_api.NESTError

        self.assertRaisesRegex(
            exception, message, raise_custom_exception, exception, message)

    def test_StackUnderFlow(self):
        """Stack underflow"""

        nest.ResetKernel()

        self.assertRaisesRegex(
            nest.ll_api.NESTError, "StackUnderflow", nest.ll_api.sr, 'clear ;')

    def test_DivisionByZero(self):
        """Division by zero"""

        nest.ResetKernel()

        self.assertRaisesRegex(
            nest.ll_api.NESTError, "DivisionByZero", nest.ll_api.sr, '1 0 div')

    def test_UnknownNode(self):
        """Unknown node"""

        nest.ResetKernel()

        self.assertRaisesRegex(
            nest.ll_api.NESTError, "UnknownNode", nest.Connect, (99, ), (99, ))

    def test_UnknownModel(self):
        """Unknown model name"""

        nest.ResetKernel()

        self.assertRaisesRegex(
            nest.ll_api.NESTError, "UnknownModelName", nest.Create, -1)


def suite():
    suite = unittest.makeSuite(ErrorTestCase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())

if __name__ == "__main__":
    run()
