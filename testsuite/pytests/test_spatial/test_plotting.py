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

import nest
import numpy as np
import pytest

try:
    import matplotlib

    matplotlib.use("Agg")  # backend without window
    import matplotlib.pyplot as plt

    tmp_fig = plt.figure()  # make sure we can open a figure
    plt.close(tmp_fig)
    PLOTTING_POSSIBLE = True
except ImportError:
    PLOTTING_POSSIBLE = False


# skip all test if plotting impossible
pytestmark = pytest.mark.skipif(
    not PLOTTING_POSSIBLE, reason="Plotting impossible because matplotlib or display missing"
)


class TestLayerPlot:
    @pytest.fixture(autouse=True)
    def _prep(self):
        nest.ResetKernel()
        self.layer = nest.Create(
            "iaf_psc_alpha", positions=nest.spatial.grid(shape=[3, 3], extent=[2.0, 2.0], edge_wrap=True)
        )

    def test_PlotLayer(self):
        """Test plotting layer."""

        nest.PlotLayer(self.layer)

        plotted_datapoints = np.array(plt.gca().collections[-1].get_offsets().data)
        reference_datapoints = np.array(nest.GetPosition(self.layer))
        assert plotted_datapoints == pytest.approx(reference_datapoints)

    def test_PlotTargets(self):
        """Test plotting targets."""

        delta = 0.05
        mask = {"rectangular": {"lower_left": [-delta, -2 / 3 - delta], "upper_right": [2 / 3 + delta, delta]}}

        nest.ResetKernel()
        layer = nest.Create(
            "iaf_psc_alpha", positions=nest.spatial.grid(shape=[3, 3], extent=[2.0, 2.0], edge_wrap=True)
        )

        # connect layer -> layer
        projection = nest.PairwiseBernoulli(layer, layer, p=1.0, mask=mask, syn_spec=nest.synapsemodels.stdp())
        nest.Connect(projection)
        nest.BuildNetwork()

        ctr = nest.FindCenterElement(self.layer)
        fig = nest.PlotTargets(ctr, self.layer)
        fig.gca().set_title("Plain call")

        plotted_datapoints = plt.gca().collections[0].get_offsets().data
        eps = 0.01
        pos = np.array(nest.GetPosition(self.layer))
        pos_xmask = pos[np.where(pos[:, 0] > -eps)]
        reference_datapoints = pos_xmask[np.where(pos_xmask[:, 1] < eps)][::-1]
        assert np.sort(plotted_datapoints, axis=0) == pytest.approx(np.sort(reference_datapoints, axis=0))

        fig = nest.PlotTargets(ctr, self.layer, mask=mask)
        ax = fig.gca()
        ax.set_title("Call with mask")
        assert len(ax.patches) >= 1

    def test_PlotSources(self):
        """Test plotting sources"""

        delta = 0.05
        mask = {"rectangular": {"lower_left": [-delta, -2 / 3 - delta], "upper_right": [2 / 3 + delta, delta]}}
        cdict = {"rule": "pairwise_bernoulli", "p": 1.0, "mask": mask}
        sdict = {"synapse_model": "stdp_synapse"}

        # connect layer -> layer
        nest.Connect(self.layer, self.layer, cdict, sdict)

        ctr = nest.FindCenterElement(self.layer)
        fig = nest.PlotSources(self.layer, ctr)
        fig.gca().set_title("Plain call")

        plotted_datapoints = plt.gca().collections[0].get_offsets().data
        eps = 0.01
        pos = np.array(nest.GetPosition(self.layer))
        pos_xmask = pos[np.where(pos[:, 0] < eps)]
        reference_datapoints = pos_xmask[np.where(pos_xmask[:, 1] > -eps)][::-1]
        assert np.sort(plotted_datapoints, axis=0) == pytest.approx(np.sort(reference_datapoints, axis=0))

        fig = nest.PlotSources(self.layer, ctr, mask=mask)
        ax = fig.gca()
        ax.set_title("Call with mask")
        assert len(ax.patches) >= 1


class TestKernelProbabilityPlot:
    @pytest.fixture(autouse=True)
    def _prep(self):
        nest.ResetKernel()
        self.plot_shape = [10, 10]
        self.plot_edges = [-0.5, 0.5, -0.5, 0.5]
        self.layer = nest.Create("iaf_psc_alpha", positions=nest.spatial.grid([10, 10], edge_wrap=False))
        self.source = self.layer[25]

    @staticmethod
    def _probability_calculation(distance):
        return 1 - 1.5 * distance

    def test_plot_probability_kernel(self):
        """Plot parameter probability"""

        source_x, source_y = nest.GetPosition(self.source)

        # Calculate reference values
        ref_probability = np.zeros(self.plot_shape[::-1])
        for i, x in enumerate(np.linspace(self.plot_edges[0], self.plot_edges[1], self.plot_shape[0])):
            positions = np.array(
                [[x, y] for y in np.linspace(self.plot_edges[2], self.plot_edges[3], self.plot_shape[1])]
            )
            ref_distances = np.sqrt((positions[:, 0] - source_x) ** 2 + (positions[:, 1] - source_y) ** 2)
            values = self._probability_calculation(ref_distances)
            ref_probability[:, i] = np.maximum(np.minimum(np.array(values), 1.0), 0.0)

        # Create the parameter
        parameter = self._probability_calculation(nest.spatial.distance)

        fig, ax = plt.subplots()
        nest.PlotProbabilityParameter(self.source, parameter, ax=ax, shape=self.plot_shape, edges=self.plot_edges)

        assert len(ax.images) >= 1
        img = ax.images[0]
        img_data = img.get_array().data
        assert img_data == pytest.approx(ref_probability)

    def test_plot_probability_kernel_with_mask(self):
        """Plot parameter probability with mask"""

        parameter = self._probability_calculation(1 - 1.5 * nest.spatial.distance)

        masks = [
            {"circular": {"radius": 0.4}},
            {"doughnut": {"inner_radius": 0.2, "outer_radius": 0.45}},
            {"rectangular": {"lower_left": [-0.3, -0.3], "upper_right": [0.3, 0.3]}},
            {"elliptical": {"major_axis": 0.8, "minor_axis": 0.4}},
        ]
        fig, axs = plt.subplots(2, 2)

        for mask, ax in zip(masks, axs.flatten()):
            nest.PlotProbabilityParameter(
                self.source, parameter, mask=mask, ax=ax, shape=self.plot_shape, edges=self.plot_edges
            )

            assert len(ax.images) >= 1
            assert len(ax.patches) >= 1
