
import nest
import hl_api

hl_api.nest = nest

def test ():
    """ Runs a battery of unit tests on Topology PyNEST """
    import nest.topology.tests
    import unittest

    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(nest.topology.tests.suite())


from hl_api import *
