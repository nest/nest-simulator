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
Tests for basic spatial plotting functions.
"""

import unittest
import nest
import numpy as np

try:
    import matplotlib.pyplot as plt

    tmp_fig = plt.figure()  # make sure we can open a window; DISPLAY may not be set
    plt.close(tmp_fig)
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
        delta = 0.05
        mask = {'rectangular': {'lower_left': [-delta, -2/3 - delta], 'upper_right': [2/3 + delta, delta]}}
        cdict = {'rule': 'pairwise_bernoulli', 'p': 1.,
                 'mask': mask}
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

        fig = nest.PlotTargets(ctr, l, mask=mask)
        ax = fig.gca()
        ax.set_title('Call with mask')
        self.assertGreaterEqual(len(ax.patches), 1)

    def test_plot_probability_kernel(self):
        """Plot parameter probability"""
        nest.ResetKernel()
        plot_shape = [10, 10]
        plot_edges = [-0.5, 0.5, -0.5, 0.5]

        def probability_calculation(distance):
            return 1 - 1.5*distance

        l = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid([10, 10], edge_wrap=False))
        source = l[25]
        source_pos = np.array(nest.GetPosition(source))
        source_x, source_y = source_pos

        # Calculate reference values
        ref_probability = np.zeros(plot_shape[::-1])
        for i, x in enumerate(np.linspace(plot_edges[0], plot_edges[1], plot_shape[0])):
            positions = np.array([[x, y] for y in np.linspace(plot_edges[2], plot_edges[3], plot_shape[1])])
            ref_distances = np.sqrt((positions[:, 0] - source_x)**2 + (positions[:, 1] - source_y)**2)
            values = probability_calculation(ref_distances)
            ref_probability[:, i] = np.maximum(np.minimum(np.array(values), 1.0), 0.0)

        # Create the parameter
        parameter = probability_calculation(nest.spatial.distance)

        fig, ax = plt.subplots()
        nest.PlotProbabilityParameter(source, parameter, ax=ax, shape=plot_shape, edges=plot_edges)

        self.assertEqual(len(ax.images), 1)
        img = ax.images[0]
        img_data = img.get_array().data
        self.assertTrue(np.array_equal(img_data, ref_probability))

    def test_plot_probability_kernel_with_mask(self):
        """Plot parameter probability with mask"""
        nest.ResetKernel()
        plot_shape = [10, 10]
        plot_edges = [-0.5, 0.5, -0.5, 0.5]

        l = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid([10, 10], edge_wrap=False))
        parameter = 1 - 1.5*nest.spatial.distance

        source = l[25]
        masks = [{'circular': {'radius': 0.4}},
                 {'doughnut': {'inner_radius': 0.2, 'outer_radius': 0.45}},
                 {'rectangular': {'lower_left': [-.3, -.3], 'upper_right': [0.3, 0.3]}},
                 {'elliptical': {'major_axis': 0.8, 'minor_axis': 0.4}}]
        fig, axs = plt.subplots(2, 2)
        for mask, ax in zip(masks, axs.flatten()):
            nest.PlotProbabilityParameter(source, parameter, mask=mask, ax=ax, shape=plot_shape, edges=plot_edges)
            self.assertEqual(len(ax.images), 1)
            self.assertGreaterEqual(len(ax.patches), 1)


def suite():
    suite = unittest.makeSuite(PlottingTestCase, 'test')
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())

    plt.show()
