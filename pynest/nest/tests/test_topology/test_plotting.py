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
"""

import unittest
import nest
import numpy as np
import matplotlib as mpl

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
        nest.ResetKernel()
        l = nest.Create('iaf_psc_alpha',
                        positions=nest.spatial.grid(shape=[3, 3],
                                                    extent=[2., 2.],
                                                    edge_wrap=True))
        nest.PlotLayer(l)

        plotted_datapoints = plt.gca().collections[-1].get_offsets().data
        reference_datapoints = nest.GetPosition(l)
        self.assertTrue(np.allclose(plotted_datapoints, reference_datapoints))

    def test_PlotTargets(self):
        """Test plotting targets."""
        cdict = {'rule': 'pairwise_bernoulli', 'p': 1.,
                 'mask': {'grid': {'shape': [2, 2]}}}
        sdict = {'synapse_model': 'stdp_synapse'}
        nest.ResetKernel()
        l = nest.Create('iaf_psc_alpha',
                        positions=nest.spatial.grid(shape=[3, 3],
                                                    extent=[2., 2.],
                                                    edge_wrap=True))

        # connect l -> l
        nest.Connect(l, l, cdict, sdict)

        ctr = nest.FindCenterElement(l)
        fig = nest.PlotTargets(ctr, l)
        fig.gca().set_title('Plain call')

        plotted_datapoints = plt.gca().collections[0].get_offsets().data
        eps = 0.01
        pos = np.array(nest.GetPosition(l))
        pos_xmask = pos[np.where(pos[:, 0] > -eps)]
        reference_datapoints = pos_xmask[np.where(pos_xmask[:, 1] < eps)][::-1]
        self.assertTrue(np.array_equal(np.sort(plotted_datapoints, axis=0), np.sort(reference_datapoints, axis=0)))

    def test_PlotKernel(self):
        """Test plotting kernels."""
        nest.ResetKernel()
        l = nest.Create('iaf_psc_alpha',
                        positions=nest.spatial.grid(shape=[3, 3],
                                                    extent=[2., 2.],
                                                    edge_wrap=True))
        f = plt.figure()
        a1 = f.add_subplot(221)
        ctr = nest.FindCenterElement(l)
        nest.PlotKernel(a1, ctr, {'circular': {'radius': 1.}},
                        {'gaussian': {'sigma': 0.2}})

        # This test has a more fuzzy testing criteria: Instead of checking
        # values against a reference it checks that each of the axes
        # contains some of the expected plotting elements.

        num_circle_elements_a1 = sum([type(p) == mpl.patches.Circle for p in a1.patches])
        self.assertGreater(num_circle_elements_a1, 2)

        a2 = f.add_subplot(222)
        nest.PlotKernel(a2, ctr, {'doughnut': {'inner_radius': 0.5,
                                               'outer_radius': 0.75}})

        num_circle_elements_a2 = sum([type(p) == mpl.patches.Circle for p in a2.patches])
        self.assertGreater(num_circle_elements_a2, 2)

        a3 = f.add_subplot(223)
        nest.PlotKernel(a3, ctr, {'rectangular':
                                  {'lower_left': [-.5, -.5],
                                   'upper_right': [0.5, 0.5]}})

        num_circle_elements_a3 = sum([type(p) == mpl.patches.Rectangle for p in a3.patches])
        self.assertGreater(num_circle_elements_a3, 2)


def suite():
    suite = unittest.makeSuite(PlottingTestCase, 'test')
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())

    import matplotlib.pyplot as plt

    plt.show()
