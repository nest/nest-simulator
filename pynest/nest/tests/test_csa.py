# -*- coding: utf-8 -*-
#
# test_csa.py
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
    import csa
    HAVE_CSA = True
except ImportError:
    HAVE_CSA = False

try:
    import numpy
    HAVE_NUMPY = True
except ImportError:
    HAVE_NUMPY = False

nest.sli_run("statusdict/have_libneurosim ::")
HAVE_LIBNEUROSIM = nest.sli_pop()


@nest.check_stack
@unittest.skipIf(not HAVE_CSA, 'Python CSA package is not available')
@unittest.skipIf(
    not HAVE_LIBNEUROSIM,
    'NEST was built without the libneurosim library'
)
class CSATestCase(unittest.TestCase):
    """CSA tests"""

    def test_CSA_OneToOne_tuples(self):
        """One-to-one connectivity with id tuples"""

        nest.ResetKernel()

        n_neurons = 4

        sources = nest.Create("iaf_psc_alpha", n_neurons)
        targets = nest.Create("iaf_psc_alpha", n_neurons)

        cg = csa.cset(csa.oneToOne)

        nest.CGConnect(sources, targets, cg)

        for i in range(n_neurons):
            conns = nest.GetStatus(nest.GetConnections([sources[i]]))
            self.assertEqual(len(conns), 1)
            self.assertEqual(conns[0]["target"], targets[i])

            conns = nest.GetStatus(nest.GetConnections([targets[i]]))
            self.assertEqual(len(conns), 0)

    @unittest.skipIf(not HAVE_NUMPY, 'NumPy package is not available')
    def test_CSA_OneToOne_intvectors(self):
        """One-to-one connectivity with id intvectors"""

        nest.ResetKernel()

        n_neurons = 4

        sources = nest.Create("iaf_psc_alpha", n_neurons)
        targets = nest.Create("iaf_psc_alpha", n_neurons)

        cg = csa.cset(csa.oneToOne)

        nest.CGConnect(numpy.array(sources), numpy.array(targets), cg)

        for i in range(n_neurons):
            conns = nest.GetStatus(nest.GetConnections([sources[i]]))
            self.assertEqual(len(conns), 1)
            self.assertEqual(conns[0]["target"], targets[i])

            conns = nest.GetStatus(nest.GetConnections([targets[i]]))
            self.assertEqual(len(conns), 0)

    def test_CSA_OneToOne_params(self):
        """One-to-one connectivity with paramters"""

        nest.ResetKernel()

        n_neurons = 4
        weight = 10000.0
        delay = 2.0

        sources = nest.Create("iaf_psc_alpha", n_neurons)
        targets = nest.Create("iaf_psc_alpha", n_neurons)

        cs = csa.cset(csa.oneToOne, weight, delay)

        nest.CGConnect(sources, targets, cs, {"weight": 0, "delay": 1})

        for i in range(n_neurons):
            conns = nest.GetStatus(nest.GetConnections([sources[i]]))
            self.assertEqual(len(conns), 1)
            self.assertEqual(conns[0]["target"], targets[i])
            self.assertEqual(conns[0]["weight"], weight)
            self.assertEqual(conns[0]["delay"], delay)

            conns = nest.GetStatus(nest.GetConnections([targets[i]]))
            self.assertEqual(len(conns), 0)

    def test_CSA_OneToOne_synmodel(self):
        """One-to-one connectivity with synmodel"""

        nest.ResetKernel()

        n_neurons = 4
        synmodel = "stdp_synapse"

        sources = nest.Create("iaf_psc_alpha", n_neurons)
        targets = nest.Create("iaf_psc_alpha", n_neurons)

        cs = csa.cset(csa.oneToOne)

        nest.CGConnect(sources, targets, cs, model=synmodel)

        for i in range(n_neurons):
            conns = nest.GetStatus(nest.GetConnections([sources[i]]))
            self.assertEqual(len(conns), 1)
            self.assertEqual(conns[0]["target"], targets[i])
            self.assertEqual(conns[0]["synapse_model"], synmodel)

            conns = nest.GetStatus(nest.GetConnections([targets[i]]))
            self.assertEqual(len(conns), 0)

    def test_CSA_error_handling(self):
        """Error handling of CGConnect"""

        nest.ResetKernel()

        cs = csa.cset(csa.oneToOne)
        nonnodes = [1, 2, 3]

        self.assertRaisesRegex(nest.NESTError, "UnknownNode",
                               nest.CGConnect, nonnodes, nonnodes, cs)

        n_neurons = 4

        sources = nest.Create("iaf_psc_alpha", n_neurons)
        targets = nest.Create("iaf_psc_alpha", n_neurons)

        self.assertRaisesRegex(nest.NESTError, "UnknownSynapseType",
                               nest.CGConnect, sources, targets, cs,
                               model="nonexistent_synapse")


def suite():

    suite = unittest.makeSuite(CSATestCase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
