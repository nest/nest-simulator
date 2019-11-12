# -*- coding: utf-8 -*-
#
# test_visualization.py
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
Tests for visualization functions.
"""

import os
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

try:
    import pydot
    HAVE_PYDOT = True
except ImportError:
    HAVE_PYDOT = False

try:
    import pandas
    HAVE_PANDAS = True
except ImportError:
    HAVE_PANDAS = False


class VisualizationTestCase(unittest.TestCase):
    def nest_tmpdir(self):
        """Returns temp dir path from environment, current dir otherwise."""
        if 'NEST_DATA_PATH' in os.environ:
            return os.environ['NEST_DATA_PATH']
        else:
            return '.'

    @unittest.skipIf(not HAVE_PYDOT, 'pydot not found')
    def test_plot_network(self):
        """Test plot_network"""
        import nest.visualization as nvis
        nest.ResetKernel()
        sources = nest.Create('iaf_psc_alpha', 10)
        targets = nest.Create('iaf_psc_alpha', 10)
        nest.Connect(sources, targets)

        filename = os.path.join(self.nest_tmpdir(), 'network_plot.png')
        nvis.plot_network(sources + targets, filename)
        self.assertTrue(os.path.isfile(filename), 'Plot was not created or not saved')
        os.remove(filename)

    def voltage_trace_verify(self, device):
        self.assertIsNotNone(plt._pylab_helpers.Gcf.get_active(), 'No active figure')
        ax = plt.gca()
        vm = device.get('events', 'V_m')
        for ref_vm, line in zip((vm[::2], vm[1::2]), ax.lines):
            x_data, y_data = line.get_data()
            # Check that times are correct
            self.assertEqual(list(x_data), list(np.unique(device.get('events', 'times'))))
            # Check that voltmeter data corresponds to the lines in the plot
            self.assertTrue(all(np.isclose(ref_vm, y_data)))
        plt.close(ax.get_figure())

    @unittest.skipIf(not PLOTTING_POSSIBLE, 'Plotting impossible because matplotlib or display missing')
    def test_voltage_trace_from_device(self):
        """Test voltage_trace from device"""
        import nest.voltage_trace as nvtrace
        nest.ResetKernel()
        nodes = nest.Create('iaf_psc_alpha', 2)
        pg = nest.Create('poisson_generator', 1, {'rate': 1000.})
        device = nest.Create('voltmeter')
        nest.Connect(pg, nodes)
        nest.Connect(device, nodes)
        nest.Simulate(100)

        # Test with data from device
        nest.voltage_trace.from_device(device)
        self.voltage_trace_verify(device)

        # Test with fata from file
        vm = device.get('events')
        data = np.zeros([len(vm['senders']), 3])
        data[:, 0] = vm['senders']
        data[:, 1] = vm['times']
        data[:, 2] = vm['V_m']
        filename = os.path.join(self.nest_tmpdir(), 'voltage_trace.txt')
        np.savetxt(filename, data)
        nest.voltage_trace.from_file(filename)
        self.voltage_trace_verify(device)
        os.remove(filename)

    def spike_detector_data_setup(self):
        nest.ResetKernel()
        pg = nest.Create('poisson_generator', {'rate': 1000.})
        sd = nest.Create('spike_detector')
        nest.Connect(pg, sd)
        nest.Simulate(100)
        return sd

    def spike_detector_raster_verify(self, sd_ref):
        self.assertIsNotNone(plt._pylab_helpers.Gcf.get_active(), 'No active figure')
        fig = plt.gcf()
        axs = fig.get_axes()
        x_data, y_data = axs[0].lines[0].get_data()
        plt.close(fig)
        # Have to use isclose() because of round-off errors
        self.assertTrue(all(np.isclose(x_data, sd_ref)))

    @unittest.skipIf(not PLOTTING_POSSIBLE, 'Plotting impossible because matplotlib or display missing')
    def test_raster_plot(self):
        """Test raster_plot"""
        import nest.raster_plot as nraster

        sd = self.spike_detector_data_setup()
        spikes = sd.get('events')
        sd_ref = spikes['times']

        # Test from_device
        nest.raster_plot.from_device(sd)
        self.spike_detector_raster_verify(sd_ref)

        # Test from_data
        data = np.zeros([len(spikes['senders']), 2])
        data[:, 0] = spikes['senders']
        data[:, 1] = spikes['times']
        nest.raster_plot.from_data(data)
        self.spike_detector_raster_verify(sd_ref)

        # Test from_file
        filename = os.path.join(self.nest_tmpdir(), 'spike_data.txt')
        np.savetxt(filename, data)
        nest.raster_plot.from_file(filename)
        self.spike_detector_raster_verify(sd_ref)

        # Test from_file_numpy
        nest.raster_plot.from_file_numpy([filename])
        self.spike_detector_raster_verify(sd_ref)

        if HAVE_PANDAS:
            # Test from_file_pandas
            nest.raster_plot.from_file_pandas([filename])
            self.spike_detector_raster_verify(sd_ref)

        # Test extract_events
        all_extracted = nest.raster_plot.extract_events(data)
        times_30_to_40_extracted = nest.raster_plot.extract_events(data, time=[30., 40.], sel=[1])
        source_2_extracted = nest.raster_plot.extract_events(data, sel=[2])
        self.assertTrue(np.array_equal(all_extracted, data))
        self.assertTrue(np.all(times_30_to_40_extracted[:, 1] >= 30.))
        self.assertTrue(np.all(times_30_to_40_extracted[:, 1] < 40.))
        self.assertEqual(len(source_2_extracted), 0)

        # Cleanup temporary datafile
        os.remove(filename)


def suite():
    suite = unittest.makeSuite(VisualizationTestCase, 'test')
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
