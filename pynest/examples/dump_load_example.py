# -*- coding: utf-8 -*-
#
# dump_load_example.py
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
Store and re-load a network simulation
--------------------------------------

This example shows how to store (dump) user-defined aspects of a
network to file and how to re-load it again for further simulation.
This may be used, e.g., to train weights in a network up to a certain
point, store those weights and later perform diverse experiments on
the same network using the stored weights.

.. admonition:: Only user-defined aspects are stored

   NEST does not support support storing the complete state of a simulation
   in a way that would allow one to continue a simulation as if one had
   made an new `Simulate()` call on an existing network. Such complete
   checkpointing would be very difficult to implement.

   NEST's explicit approach to storing and loading network state makes
   clear to all which aspects of a network are carried from one simulation
   to another and thus contributes to good scientific practice.

   Storing and loading is currently not supported for MPI-parallel simulations.

"""
###############################################################################
# Import necessary modules.

import nest
import pickle
###############################################################################
# These modules are only needed for illustrative plotting.

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

###############################################################################
# Implement network as class.
#
# Implementing the network as a class makes network properties available to
# the initial network builder, the dumper and the loader, thus reducing the
# amount of data that needs to be stored.

class EINetwork:
    """
    A simple balanced random network with plastic excitatory synapses.

    This simple Brunel-style balanced random network has an excitatory
    and inhibitory population, both driven by external excitatory poisson
    input. Excitatory connections are plastic (STDP). Spike activity of
    the excitatory population is recorded.

    The model is provided as a non-trivial example for dumping and loading.
    """

    def __init__(self):
        self.nI = 500
        self.nE = 4 * self.nI
        self.n = self.nE + self.nI

        self.JE = 15.0
        self.JI = -4 * self.JE
        self.indeg_e = 200
        self.indeg_i = 50

        self.neuron_model = 'iaf_psc_alpha'

        # Create synapse models so we can extract specific connection information
        nest.CopyModel('stdp_synapse_hom', 'e_syn', {'Wmax': 2 * self.JE})
        nest.CopyModel('static_synapse', 'i_syn')

        self.nrn_params = {'V_m': nest.random.normal(-65., 5.)}
        self.poisson_rate = 4000.

    def build(self):
        """
        Construct network from scratch, including instrumentation.
        """

        self.e_neurons = nest.Create(self.neuron_model, n=self.nE, params=self.nrn_params)
        self.i_neurons = nest.Create(self.neuron_model, n=self.nI)
        self.neurons = self.e_neurons + self.i_neurons

        self.pg = nest.Create('poisson_generator', {'rate': self.poisson_rate})
        self.sr = nest.Create('spike_recorder')

        nest.Connect(self.e_neurons, self.neurons,
                     {'rule': 'fixed_indegree', 'indegree': self.indeg_e},
                     {'synapse_model': 'e_syn', 'weight': self.JE})
        nest.Connect(self.i_neurons, self.neurons,
                     {'rule': 'fixed_indegree', 'indegree': self.indeg_i},
                     {'synapse_model': 'i_syn', 'weight': self.JI})
        nest.Connect(self.pg, self.neurons, 'all_to_all', {'weight': self.JE})
        nest.Connect(self.e_neurons, self.sr)

    def dump(self, dump_filename):
        """
        Store neuron membrane potential and synaptic weights to given file.
        """

        assert nest.NumProcesses() == 1, "Cannot dump MPI parallel"
        ###############################################################################
        # Build dictionary with relevant network information:
        #   - membrane potential for all neurons in each population
        #   - source, target and weight of all connections
        # Dictionary entries are Pandas Dataframes.
        #
        # Strictly speaking, we would not need to store the weight of the inhibitory
        # synapses since they are fixed, but we do so out of symmetry and to make it
        # easier to add plasticity for inihibitory connections later.

        network = {}
        network['n_vp'] = nest.GetKernelStatus('total_num_virtual_procs')
        network['e_nrns'] = self.neurons.get('V_m', output='pandas')
        network['i_nrns'] = self.neurons.get('V_m', output='pandas')

        network['e_syns'] = nest.GetConnections(synapse_model='e_syn').get(
                                     ('source', 'target', 'weight'), output='pandas')
        network['i_syns'] = nest.GetConnections(synapse_model='i_syn').get(
                                     ('source', 'target', 'weight'), output='pandas')

        with open(dump_filename, 'wb') as f:
            pickle.dump(network, f, pickle.HIGHEST_PROTOCOL)

    def load(self, dump_filename):
        """
        Load network data from file and combine with base information to rebuild network.
        """

        assert nest.NumProcesses() == 1, "Cannot load MPI parallel"

        with open(dump_filename, 'rb') as f:
            network = pickle.load(f)

        assert (network['n_vp'] == nest.GetKernelStatus('total_num_virtual_procs'),
                'N_VP must match')

        ###############################################################################
        # Reconstruct neurons
        # Since NEST does not understand Pandas Series, we must pass the values as
        # NumPy arrays
        self.e_neurons = nest.Create(self.neuron_model, n=self.nE,
                                     params={'V_m': network['e_nrns'].V_m.values})
        self.i_neurons = nest.Create(self.neuron_model, n=self.nI,
                                     params={'V_m': network['i_nrns'].V_m.values})
        self.neurons = self.e_neurons + self.i_neurons

        ###############################################################################
        # Reconstruct instrumentation
        self.pg = nest.Create('poisson_generator', {'rate': self.poisson_rate})
        self.sr = nest.Create('spike_recorder')

        ###############################################################################
        # Reconstruct connectivity
        nest.Connect(network['e_syns'].source.values, network['e_syns'].target.values,
                     'one_to_one',
                     {'synapse_model': 'e_syn', 'weight': network['e_syns'].weight.values})

        nest.Connect(network['i_syns'].source.values, network['i_syns'].target.values,
                     'one_to_one',
                     {'synapse_model': 'i_syn', 'weight': network['i_syns'].weight.values})

        ###############################################################################
        # Reconnect instruments
        nest.Connect(self.pg, self.neurons, 'all_to_all', {'weight': self.JE})
        nest.Connect(self.e_neurons, self.sr)


class DemoPlot:
    """
    Create demonstration figure for effect of storing and loading a network.

    The figure shows raster plots for five different runs, a PSTH for the
    initial 1 s simulation and PSTHs for all 1 s continuations, and weight 
    historgrams.
    """

    def __init__(self):
        self._colors = [c['color'] for c in plt.rcParams['axes.prop_cycle']]
        self._next_line = 0

        self.fig = plt.figure(figsize=(12, 7), constrained_layout=True)

        gs = self.fig.add_gridspec(nrows=5, ncols=2)
        self.rasters = [self.fig.add_subplot(gs[n, 0]) for n in range(5)]
        self.psths = self.fig.add_subplot(gs[:3, 1])
        self.weights = self.fig.add_subplot(gs[3:, 1])

    def add_to_plot(self, sr, n_max=100, time_shift=0, t_max=1000, lbl=''):

        spks = pd.DataFrame.from_dict(sr.get('events'))
        spks = spks.loc[(spks.senders < n_max) & (spks.times > time_shift)]
        spks.times -= time_shift

        self.rasters[self._next_line].plot(spks.times, spks.senders, '.',
                                           color=self._colors[self._next_line])
        self.rasters[self._next_line].set_xlim(0, t_max)
        self.rasters[self._next_line].set_title(lbl, fontsize='small')

        sbins = np.arange(0, t_max, 10.)
        self.psths.hist(spks.times, bins=sbins, histtype='step',
                        color=self._colors[self._next_line], alpha=0.7, lw=3)
        self.psths.set_xlim(0, t_max)

        wbins = np.arange(14., 16., 0.05)
        w = nest.GetConnections(synapse_model='e_syn').weight
        self.weights.hist(w, bins=wbins, histtype='step', label=lbl,
                          color=self._colors[self._next_line],
                          alpha=0.7, lw=3)

        self._next_line += 1


if __name__ == '__main__':

    T_sim = 1000

    dplot = DemoPlot()

    ###############################################################################
    # Create network from scratch and simulate 1s.
    nest.SetKernelStatus({'local_num_threads': 4})
    ein = EINetwork()

    ein.build()
    nest.Simulate(T_sim)
    dplot.add_to_plot(ein.sr, lbl='Initial simuation')

    ###############################################################################
    # Write network state to file with state after 1s.
    ein.dump('ein_1000.pkl')

    ###############################################################################
    # Continue simulation by another 1s.
    nest.Simulate(T_sim)
    dplot.add_to_plot(ein.sr, lbl='Continued simuation', time_shift=T_sim)

    ###############################################################################
    # Clear kernel, reload network from file and simulate for 1s.
    nest.ResetKernel()
    nest.SetKernelStatus({'local_num_threads': 4})
    ein2 = EINetwork()
    ein2.load('ein_1000.pkl')
    nest.Simulate(T_sim)
    dplot.add_to_plot(ein2.sr, lbl='Reloaded simuation')

    ###############################################################################
    # Repeat previous step. This shall result in *exactly* the same results as
    # the previous run because we use the same random seed.
    nest.ResetKernel()
    nest.SetKernelStatus({'local_num_threads': 4})
    ein2 = EINetwork()
    ein2.load('ein_1000.pkl')
    nest.Simulate(T_sim)
    dplot.add_to_plot(ein2.sr, lbl='Reloaded simuation (same seed)')

    ###############################################################################
    # Clear, reload and simulate again, but now with different random seed.
    # Details in results shall differ from previous run.
    nest.ResetKernel()
    nest.SetKernelStatus({'local_num_threads': 4, 'rng_seed': 5345234})
    ein2 = EINetwork()
    ein2.load('ein_1000.pkl')
    nest.Simulate(T_sim)
    dplot.add_to_plot(ein2.sr, lbl='Reloaded simulation (different seed)')

    dplot.fig.savefig('dump_load_demo.png')
    plt.show()

    input('Press ENTER to close figure!')

    ###############################################################################
    # In the resulting figure, note the following
    #
    # - The raster plot in the top left and PSTH in the top right show the initial
    #   simulation. All other simulations are based on the state at the end of this
    #   simulation.
    # - The "Continued simulation" (orange) shows the result of simply calling
    #   Simulate() again an running for another 1s. In this case, all active PSCs
    #   and all spikes in transition are preserved, as is the spike history for
    #   STDP evaluation.
    # - The "Reloaded simulation"s (green, red) preserve only the membrane potntial
    #   V_m and the synaptic weights from the initial simulation. The effect of not
    #   preserving active PSCs and spikes in transition is clearly visible by the
    #   lack of activity at the beginning of the simulation. The two simulations show
    #   *identical* results because they run from the same starting point with the
    #   same random number seeds.
    # - The "Reloaded simulation (different seed)" (purple) starts from the same
    #   starting point as the green and red cases, but the different random seed
    #   causes different spike times.
    # - Weight distributions are shown to the bottom left. One clearly sees that
    #   weights have evolved from the distribution after 1s (blue), and that the
    #   weights of the continued (orange) and reloaded (green, red, purple) simulations
    #   differ slightly.
    # - In the PSTH and weight histograms, the green and red curves overlap fully
    #   forming a brown curve.
