#! /usr/bin/env python

""" Runs a battery of unit tests on PyNEST """

import sys, os
os.environ['DELAY_PYNEST_INIT'] = '1'

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

myprint("\n")
myprint("Phase 6: Running PyNEST tests.\n")
myprint("------------------------------\n")

import nest
nest.init(["--verbosity=WARNING", "--quiet"])

import nest.topology

import nest.tests
import nest.topology.tests

import unittest

# The function setUp() is called before each test run,
# it is used here to print the status.
unittest.TestCase.setUp = _setup

# The testsuite is initialized w/ stdout. That goes into the logfile.
runner = unittest.TextTestRunner(verbosity=2, stream=sys.stdout)
result = runner.run(nest.tests.suite())
result_topo = runner.run(nest.topology.tests.suite())

# We return the numbers of tests via a temporaty file
file_handle = file("pynest_test_numbers.log", "w")
file_handle.write(str(result.testsRun + result_topo.testsRun) + " ")
file_handle.write(str(len(result.failures)+len(result.errors) + len(result_topo.failures)+len(result_topo.errors)) + " ")
file_handle.write(str(result.testsRun + result_topo.testsRun - len(result.failures) - len(result.errors) - len(result_topo.failures) - len(result_topo.errors)) + "\n")
file_handle.close()
