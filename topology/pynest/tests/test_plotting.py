# -*- coding: utf-8 -*-
#
# test_plotting.py
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
Tests for basic topology hl_api functions.

NOTE: These tests only test whether the code runs, it does not check
      whether the results produced are correct.
"""

import unittest
import nest
import nest.topology as topo

try:
    import matplotlib.pyplot as plt

    plt.figure()  # make sure we can open a window; DISPLAY may not be set
    PLOTTING_POSSIBLE = True
except:
    PLOTTING_POSSIBLE = False


@unittest.skipIf(not PLOTTING_POSSIBLE,
                 'Plotting impossible because matplotlib or display missing')
class PlottingTestCase(unittest.TestCase):
    def test_PlotLayer(self):
        """Test plotting layer."""
        ldict = {'elements': 'iaf_psc_alpha', 'rows': 3, 'columns': 3,
                 'extent': [2., 2.], 'edge_wrap': True}
        nest.ResetKernel()
        l = topo.CreateLayer(ldict)
        topo.PlotLayer(l)

        self.assertTrue(True)

    def test_PlotTargets(self):
        """Test plotting targets."""
        ldict = {'elements': 'iaf_psc_alpha',
                 'rows': 3,
                 'columns': 3,
                 'extent': [2., 2.], 'edge_wrap': True}
        cdict = {'connection_type': 'divergent',
                 'synapse_model': 'stdp_synapse',
                 'mask': {'grid': {'rows': 2, 'columns': 2}}}
        nest.ResetKernel()
        l = topo.CreateLayer(ldict)

        # connect l -> l
        topo.ConnectLayers(l, l, cdict)

        ctr = topo.FindCenterElement(l)
        fig = topo.PlotTargets(l[ctr-1:ctr], l)
        fig.gca().set_title('Plain call')

        self.assertTrue(True)

    def test_PlotKernel(self):
        """Test plotting kernels."""
        ldict = {'elements': 'iaf_psc_alpha', 'rows': 3, 'columns': 3,
                 'extent': [2., 2.], 'edge_wrap': True}
        nest.ResetKernel()
        l = topo.CreateLayer(ldict)
        f = plt.figure()
        a1 = f.add_subplot(221)
        ctr = topo.FindCenterElement(l)
        topo.PlotKernel(a1, l[ctr-1], {'circular': {'radius': 1.}},
                        {'gaussian': {'sigma': 0.2}})

        a2 = f.add_subplot(222)
        topo.PlotKernel(a2, l[ctr-1], {'doughnut': {'inner_radius': 0.5,
                                                    'outer_radius': 0.75}})

        a3 = f.add_subplot(223)
        topo.PlotKernel(a3, l[ctr-1], {'rectangular':
                                       {'lower_left': [-.5, -.5],
                                        'upper_right': [0.5, 0.5]}})

        self.assertTrue(True)


def suite():
    suite = unittest.makeSuite(PlottingTestCase, 'test')
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())

    import matplotlib.pyplot as plt

    plt.show()
