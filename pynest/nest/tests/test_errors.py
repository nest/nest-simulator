#! /usr/bin/env python
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
import sys

class ErrorTestCase(unittest.TestCase):
    """Tests if errors are handled correctly"""

    def test_Raise(self):
        """Error raising"""

        nest.ResetKernel()
        try:
            raise nest.NESTError('test')
            self.fail('an error should have risen!')  # should not be reached
        except nest.NESTError:
            info = sys.exc_info()[1]
            if not "test" in info.__str__():
                self.fail('could not pass error message to NEST!')             
        # another error has been thrown, this is wrong
        except: 
          self.fail('wrong error has been thrown')


    def test_StackUnderFlow(self):
        """Stack underflow"""

        nest.ResetKernel()
        try:
            nest.sr('clear ;')
            self.fail('an error should have risen!') # should not be reached
        except nest.NESTError:
            info = sys.exc_info()[1]
            if not "StackUnderflow" in info.__str__():
                self.fail('wrong error message')              
        # another error has been thrown, this is wrong
        except: 
          self.fail('wrong error has been thrown')  


    def test_DivisionByZero(self):
        """Division by zero"""

        nest.ResetKernel()
        try:
            nest.sr('1 0 div')
            self.fail('an error should have risen!')  # should not be reached
        except nest.NESTError:
            info = sys.exc_info()[1]
            if not "DivisionByZero" in info.__str__():
                self.fail('wrong error message')              
        # another error has been thrown, this is wrong
        except: 
          self.fail('wrong error has been thrown')  


    def test_UnknownNode(self):
        """Unknown node"""

        nest.ResetKernel()
        try:
            nest.Connect([99],[99])
            self.fail('an error should have risen!')  # should not be reached
        except nest.NESTError:
            info = sys.exc_info()[1]
            if not "UnknownNode" in info.__str__():
                self.fail('wrong error message')              
        # another error has been thrown, this is wrong
        except: 
          self.fail('wrong error has been thrown')

          
    def test_UnknownModel(self):
        """Unknown model name"""

        nest.ResetKernel()
        try:
            nest.Create(-1)
            self.fail('an error should have risen!')  # should not be reached
        except nest.NESTError:
            info = sys.exc_info()[1]
            if not "UnknownModelName" in info.__str__():
                self.fail('wrong error message')              
        # another error has been thrown, this is wrong
        except: 
          self.fail('wrong error has been thrown')  


def suite():

    suite = unittest.makeSuite(ErrorTestCase,'test')
    return suite


if __name__ == "__main__":

    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
