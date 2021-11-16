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

import nest
import numpy as np
import os
import pytest

try:
    import matplotlib.pyplot as plt

    tmp_fig = plt.figure()  # make sure we can open a window; DISPLAY may not be set
    plt.close(tmp_fig)
    PLOTTING_POSSIBLE = True
except ImportError:
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


class TestVisualization:
    def nest_tmpdir(self):
        """Returns temp dir path from environment, current dir otherwise."""
        if 'NEST_DATA_PATH' in os.environ:
            return os.environ['NEST_DATA_PATH']
        else:
            return '.'

    @pytest.fixture(autouse=True)
    def setUp(self):
        self.filenames = []
        yield
        # fixture teardown code below
        for filename in self.filenames:
            # Cleanup temporary datafiles
            os.remove(filename)

    @pytest.mark.skipif(not HAVE_PYDOT, reason='pydot not found')
    def test_plot_network(self):
        """Test plot_network"""
        import nest.visualization as nvis
        nest.ResetKernel()
        sources = nest.Create('iaf_psc_alpha', 10)
        targets = nest.Create('iaf_psc_alpha', 10)
        nest.Connect(sources, targets)

        filename = os.path.join(self.nest_tmpdir(), 'network_plot.png')
        self.filenames.append(filename)
        nvis.plot_network(sources + targets, filename)
        assert os.path.isfile(filename), 'Plot was not created or not saved'

    def voltage_trace_verify(self, device):
        assert plt._pylab_helpers.Gcf.get_active() is not None, 'No active figure'
        ax = plt.gca()
        vm = device.get('events', 'V_m')
        for ref_vm, line in zip((vm[::2], vm[1::2]), ax.lines):
            x_data, y_data = line.get_data()
            # Check that times are correct
            assert list(x_data) == list(np.unique(device.get('events', 'times')))
            # Check that voltmeter data corresponds to the lines in the plot
            assert all(np.isclose(ref_vm, y_data))
        plt.close(ax.get_figure())

    @pytest.mark.skipif(not PLOTTING_POSSIBLE, reason='Plotting impossible because matplotlib or display missing')
    def test_voltage_trace_from_device(self):
        """Test voltage_trace from device"""
        import nest.voltage_trace
        nest.ResetKernel()
        nodes = nest.Create('iaf_psc_alpha', 2)
        pg = nest.Create('poisson_generator', 1, {'rate': 1000.})
        device = nest.Create('voltmeter')
        nest.Connect(pg, nodes)
        nest.Connect(device, nodes)
        nest.Simulate(100)

        # Test with data from device
        plt.close("all")
        nest.voltage_trace.from_device(device)
        self.voltage_trace_verify(device)

        # Test with data from file
        vm = device.get('events')
        data = np.zeros([len(vm['senders']), 3])
        data[:, 0] = vm['senders']
        data[:, 1] = vm['times']
        data[:, 2] = vm['V_m']
        filename = os.path.join(self.nest_tmpdir(), 'voltage_trace.txt')
        self.filenames.append(filename)
        np.savetxt(filename, data)

        plt.close("all")
        nest.voltage_trace.from_file(filename)
        self.voltage_trace_verify(device)

    def spike_recorder_data_setup(self, to_file=False):
        nest.ResetKernel()
        pg = nest.Create('poisson_generator', {'rate': 1000.})
        sr = nest.Create('spike_recorder')
        if to_file:
            parrot = nest.Create('parrot_neuron')
            sr_to_file = nest.Create('spike_recorder')
            sr_to_file.record_to = 'ascii'
            nest.Connect(pg, parrot)
            nest.Connect(parrot, sr)
            nest.Connect(parrot, sr_to_file)
            nest.Simulate(100)
            return sr, sr_to_file
        else:
            nest.Simulate(100)
            return sr

    def spike_recorder_raster_verify(self, sr_ref):
        assert plt._pylab_helpers.Gcf.get_active() is not None, 'No active figure'
        fig = plt.gcf()
        axs = fig.get_axes()
        x_data, y_data = axs[0].lines[0].get_data()
        plt.close(fig)
        # Have to use isclose() because of round-off errors
        assert x_data.shape == sr_ref.shape
        assert all(np.isclose(x_data, sr_ref))

    @pytest.mark.skipif(not PLOTTING_POSSIBLE, reason='Plotting impossible because matplotlib or display missing')
    def test_raster_plot(self):
        """Test raster_plot"""
        import nest.raster_plot

        sr, sr_to_file = self.spike_recorder_data_setup(to_file=True)
        spikes = sr.get('events')
        sr_ref = spikes['times']

        # Test from_device
        nest.raster_plot.from_device(sr)
        self.spike_recorder_raster_verify(sr_ref)

        # Test from_data
        data = np.zeros([len(spikes['senders']), 2])
        data[:, 0] = spikes['senders']
        data[:, 1] = spikes['times']
        nest.raster_plot.from_data(data)
        self.spike_recorder_raster_verify(sr_ref)

        # Test from_file
        filename = sr_to_file.filenames[0]
        self.filenames.append(filename)
        nest.raster_plot.from_file(filename)
        self.spike_recorder_raster_verify(sr_ref)

        # Test from_file_numpy
        nest.raster_plot.from_file_numpy([filename])
        self.spike_recorder_raster_verify(sr_ref)

        if HAVE_PANDAS:
            # Test from_file_pandas
            nest.raster_plot.from_file_pandas([filename])
            self.spike_recorder_raster_verify(sr_ref)

        # Test extract_events
        all_extracted = nest.raster_plot.extract_events(data)
        times_30_to_40_extracted = nest.raster_plot.extract_events(data, time=[30., 40.], sel=[3])
        source_2_extracted = nest.raster_plot.extract_events(data, sel=[2])
        assert np.array_equal(all_extracted, data)
        assert np.all(times_30_to_40_extracted[:, 1] >= 30.)
        assert np.all(times_30_to_40_extracted[:, 1] < 40.)
        assert len(source_2_extracted) == 0
