# -*- coding: utf-8 -*-
#
# test_dumping.py
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
Tests for topology hl_api dumping functions.


NOTE: These tests only test whether the code runs, it does not check
      whether the results produced are correct.
"""

import unittest
import nest

import sys

import os
import os.path


class PlottingTestCase(unittest.TestCase):
    def nest_tmpdir(self):
        """Returns temp dir path from environment, current dir otherwise."""
        if 'NEST_DATA_PATH' in os.environ:
            return os.environ['NEST_DATA_PATH']
        else:
            return '.'

    def test_DumpNodes(self):
        """Test dumping nodes."""
        nest.ResetKernel()
        l = nest.Create('iaf_psc_alpha',
                        positions=nest.spatial.grid(rows=3, columns=3,
                                                    extent=[2., 2.],
                                                    edge_wrap=True))
        nest.DumpLayerNodes(l, os.path.join(self.nest_tmpdir(),
                                            'test_DumpNodes.out.lyr'))
        self.assertTrue(True)

    def test_DumpConns(self):
        """Test dumping connections."""
        cdict = {'connection_type': 'divergent',
                 'mask': {'circular': {'radius': 1.}}}
        nest.ResetKernel()
        l = nest.Create('iaf_psc_alpha',
                        positions=nest.spatial.grid(rows=3, columns=3,
                                                    extent=[2., 2.],
                                                    edge_wrap=True))
        nest.ConnectLayers(l, l, cdict)

        nest.DumpLayerConnections(l, l, 'static_synapse',
                                  os.path.join(self.nest_tmpdir(),
                                               'test_DumpConns.out.cnn'))
        self.assertTrue(True)


def suite():
    suite = unittest.makeSuite(PlottingTestCase, 'test')
    return suite


if __name__ == "__main__":

    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())

    try:
        import matplotlib.pyplot as plt

        plt.show()
    except ImportError:
        pass
