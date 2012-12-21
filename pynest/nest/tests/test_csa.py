"""
CSA tests
"""

import unittest
import nest
import nest.pynestkernel as kernel

from decorators import _skipIf

try:
    import csa
    haveCSA = True
except ImportError:
    haveCSA = False

try:
    import numpy
    haveNumpy = True
except ImportError:
    haveNumpy = False

@_skipIf(not haveCSA, 'Python CSA library not installed', 'testcase')
class CSATestCase(unittest.TestCase):
    """CSA tests"""

    def test_CSA_OneToOne(self):
        """One-to-one connectivity"""

        nest.ResetKernel()

        n = 4 # number of neurons

        pop0 = nest.LayoutNetwork("iaf_neuron", [n])
        pop1 = nest.LayoutNetwork("iaf_neuron", [n])

        cs = csa.cset(csa.oneToOne)

        nest.CGConnect (pop0, pop1, cs)

        sources = nest.GetLeaves(pop0)[0]
        targets = nest.GetLeaves(pop1)[0]
        for i in xrange(n):
            conns = nest.GetStatus(nest.FindConnections([sources[i]]), 'target')
            self.assertEqual(len(conns), 1)
            self.assertEqual(conns[0], targets[i])
            
            conns = nest.GetStatus(nest.FindConnections([targets[i]]), 'target')
            self.assertEqual(len(conns), 0)


    def test_CSA_OneToOne_params(self):
        """One-to-one connectivity"""

        nest.ResetKernel()

        n = 4 # number of neurons

        pop0 = nest.LayoutNetwork("iaf_neuron", [n])
        pop1 = nest.LayoutNetwork("iaf_neuron", [n])

        cs = csa.cset(csa.oneToOne, 10000.0, 1.0)
            
        nest.CGConnect (pop0, pop1, cs, {"weight": 0, "delay": 1})

        sources = nest.GetLeaves(pop0)[0]
        targets = nest.GetLeaves(pop1)[0]
        for i in xrange(n):
            conns = nest.GetStatus(nest.FindConnections([sources[i]]), 'target')
            self.assertEqual(len(conns), 1)
            self.assertEqual(conns[0], targets[i])
            
            conns = nest.GetStatus(nest.FindConnections([targets[i]]), 'target')
            self.assertEqual(len(conns), 0)

    @_skipIf(not haveNumpy, 'Python numpy library not installed')
    def test_CSA_cgnext(self):
        """cgnext"""

        nest.ResetKernel()

        w = 10000.0
        d = 1.0
        cs = csa.cset(csa.oneToOne, w, d)

        kernel.pushsli(cs)
        kernel.runsli('dup')
        kernel.pushsli(numpy.array([0,1,2,3]))
        kernel.pushsli(numpy.array([0,1,2,3]))
        kernel.runsli('cgsetmask_cg_a_a')
        kernel.runsli('dup')
        kernel.runsli('cgstart')
        for i in xrange(4):
            kernel.runsli('dup')
            kernel.runsli('cgnext')
            self.assertEqual(kernel.popsli(), True)
            self.assertEqual(kernel.popsli(), d)
            self.assertEqual(kernel.popsli(), w)
            self.assertEqual(kernel.popsli(), i)
            self.assertEqual(kernel.popsli(), i)
        kernel.runsli('cgnext')
        self.assertEqual(kernel.popsli(), False)


def suite():

    suite = unittest.makeSuite(CSATestCase,'test')
    return suite


if __name__ == "__main__":

    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
