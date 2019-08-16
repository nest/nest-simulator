#!/usr/bin/env python
#
# -*- coding: utf-8 -*-
#
# do_tests.py
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

""" Runs a battery of unit tests on PyNEST """

import sys, os
os.environ['PYNEST_QUIET'] = '1'

def myprint(msg):
    sys.stderr.write(msg)
    sys.stderr.flush()
    sys.stdout.write(msg)
    sys.stdout.flush()

def _setup(arg):
    if hasattr(arg,'_testMethodName'):
        MethodName = str(arg._testMethodName)
    elif hasattr(arg,'__testMethodName'):
        MethodName = str(arg.__testMethodName)
    else:
        try:
            MethodName = str(arg).split('(')[0].split(' ')[0]
        except:
            MethodName = str(arg)

    sys.stderr.write("  Running test " + MethodName + "...\n")


import nest
nest.set_verbosity('M_WARNING')

import nest.tests

import unittest

# The function setUp() is called before each test run,
# it is used here to print the status.
unittest.TestCase.setUp = _setup

# The testsuite is initialized w/ stdout. That goes into the logfile.
runner = unittest.TextTestRunner(verbosity=2, stream=sys.stdout)
result = runner.run(nest.tests.suite())

total = result.testsRun
failed = len(result.failures) + len(result.errors)
skipped = len(result.skipped)
passed = total - failed - skipped

# We return the numbers of tests via a temporaty file
file_handle = open("pynest_test_numbers.log", "w")
file_handle.write("{} {} {} {}\n".format(total, passed, skipped, failed))
file_handle.close()
