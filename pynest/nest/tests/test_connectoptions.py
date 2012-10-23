#! /usr/bin/env python
#
# test_connectoptions.py
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
ConnectOptions
"""

import unittest
import nest

class ConnectOptionsTestCase(unittest.TestCase):
    """Call Random{Con,Di}vergentConnect with non-standard options and
       ensure that the option settings are properly restored before
       returning."""


    def test_ConnectOptions(self):
        """ConnectOptions"""

        nest.ResetKernel()

        copts = nest.sli_func('GetOptions', '/RandomConvergentConnect', litconv=True)
        dopts = nest.sli_func('GetOptions', '/RandomDivergentConnect',  litconv=True)

        ncopts = dict([(k, not v) for k,v in copts.iteritems() if k != 'DefaultOptions'])
        ndopts = dict([(k, not v) for k,v in dopts.iteritems() if k != 'DefaultOptions'])
        
        n = nest.Create('iaf_neuron', 3)

        nest.RandomConvergentConnect(n, n, 1, options=ncopts)
        nest.RandomDivergentConnect (n, n, 1, options=ndopts)

        self.assertEqual(copts,
                         nest.sli_func('GetOptions', '/RandomConvergentConnect', litconv=True))
        self.assertEqual(dopts,
                         nest.sli_func('GetOptions', '/RandomDivergentConnect',  litconv=True))


def suite():

    suite = unittest.makeSuite(ConnectOptionsTestCase,'test')
    return suite


if __name__ == "__main__":

    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
