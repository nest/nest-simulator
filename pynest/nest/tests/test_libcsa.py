# -*- coding: utf-8 -*-
#
# test_libcsa.py
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
CSA tests
"""

import unittest
import nest

from . import compatibility

try:
    import libcsa
    HAVE_LIBCSA = True
except ImportError:
    HAVE_LIBCSA = False

try:
    import numpy
    HAVE_NUMPY = True
except ImportError:
    HAVE_NUMPY = False

nest.sli_run("statusdict/have_libneurosim ::")
HAVE_LIBNEUROSIM = nest.sli_pop()


@nest.check_stack
@unittest.skipIf(not HAVE_LIBCSA, 'Python libcsa package is not available')
@unittest.skipIf(not HAVE_LIBNEUROSIM, 'PyNEST was built without the libneurosim library')
class libcsaTestCase(unittest.TestCase):
    """libcsa tests"""


    def test_libcsa_OneToOne_subnet_1d(self):
        """One-to-one connectivity with 1-dim subnets"""

        nest.ResetKernel()

        n = 4 # number of neurons

        pop0 = nest.LayoutNetwork("iaf_neuron", [n])
        pop1 = nest.LayoutNetwork("iaf_neuron", [n])

        cg = libcsa.oneToOne

        nest.CGConnect (pop0, pop1, cg)

        sources = nest.GetLeaves(pop0)[0]
        targets = nest.GetLeaves(pop1)[0]
        for i in range(n):
            conns = nest.GetStatus(nest.FindConnections([sources[i]]), 'target')
            self.assertEqual(len(conns), 1)
            self.assertEqual(conns[0], targets[i])
            
            conns = nest.GetStatus(nest.FindConnections([targets[i]]), 'target')
            self.assertEqual(len(conns), 0)


    def test_libcsa_OneToOne_subnet_nd(self):
        """One-to-one connectivity with n-dim subnets"""

        nest.ResetKernel()

        n = 2 # number of neurons per dimension

        pop0 = nest.LayoutNetwork("iaf_neuron", [n, n])
        pop1 = nest.LayoutNetwork("iaf_neuron", [n, n])

        cg = libcsa.oneToOne

        self.assertRaisesRegex(nest.NESTError, "BadProperty", nest.CGConnect, pop0, pop1, cg)

    def test_libcsa_OneToOne_idrange(self):
        """One-to-one connectivity with id ranges"""

        nest.ResetKernel()

        n = 4 # number of neurons

        sources = nest.Create("iaf_neuron", n)
        targets = nest.Create("iaf_neuron", n)

        cg = libcsa.oneToOne

        nest.CGConnect (sources, targets, cg)

        for i in range(n):
            conns = nest.GetStatus(nest.FindConnections([sources[i]]), 'target')
            self.assertEqual(len(conns), 1)
            self.assertEqual(conns[0], targets[i])
            
            conns = nest.GetStatus(nest.FindConnections([targets[i]]), 'target')
            self.assertEqual(len(conns), 0)


    def test_libcsa_OneToOne_params(self):
        """One-to-one connectivity"""

        nest.ResetKernel()

        n = 4 # number of neurons

        pop0 = nest.LayoutNetwork("iaf_neuron", [n])
        pop1 = nest.LayoutNetwork("iaf_neuron", [n])

        cs = libcsa.cset(libcsa.oneToOne, 10000.0, 1.0)
            
        nest.CGConnect (pop0, pop1, cs, {"weight": 0, "delay": 1})

        sources = nest.GetLeaves(pop0)[0]
        targets = nest.GetLeaves(pop1)[0]
        for i in range(n):
            conns = nest.GetStatus(nest.FindConnections([sources[i]]), 'target')
            self.assertEqual(len(conns), 1)
            self.assertEqual(conns[0], targets[i])
            
            conns = nest.GetStatus(nest.FindConnections([targets[i]]), 'target')
            self.assertEqual(len(conns), 0)

    @unittest.skipIf(not HAVE_NUMPY, 'NumPy package is not available')
    def test_libcsa_cgnext(self):
        """cgnext"""

        nest.ResetKernel()

        w = 10000.0
        d = 1.0
        cs = libcsa.cset(libcsa.oneToOne, w, d)

        nest.sli_push(cs)
        nest.sli_run('dup')
        nest.sli_push(numpy.array([0,1,2,3]))
        nest.sli_push(numpy.array([0,1,2,3]))
        nest.sli_run('cgsetmask')
        nest.sli_run('dup')
        nest.sli_run('cgstart')
        for i in range(4):
            nest.sli_run('dup')
            nest.sli_run('cgnext')
            self.assertEqual(nest.sli_pop(), True)
            self.assertEqual(nest.sli_pop(), d)
            self.assertEqual(nest.sli_pop(), w)
            self.assertEqual(nest.sli_pop(), i)
            self.assertEqual(nest.sli_pop(), i)
        nest.sli_run('cgnext')
        self.assertEqual(nest.sli_pop(), False)


def suite():

    suite = unittest.makeSuite(libcsaTestCase,'test')
    return suite


if __name__ == "__main__":

    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
