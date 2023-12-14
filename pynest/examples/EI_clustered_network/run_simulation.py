# -*- coding: utf-8 -*-
#
# run_simulation.py
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

"""PyNEST EI-clustered network: Run Simulation
----------------------------------------

This is an example script for running the EI-clustered model with two stimulations
and generating a raster plot.
"""


import numpy as np
import default_parameters as default
import network
import general_helper
import matplotlib.pyplot as plt

if __name__ == '__main__':
    # set the parameters for the simulation
    params = {'n_jobs': 12,  #number of threads used for parallelization in NEST
              'N_E': 4000,  #number of excitatory neurons
              'N_I': 1000,   #number of inhibitory neurons
              'E_L': 0.,    #leak potential
              'V_th_E': 20.,#threshold potential for excitatory neurons
              'V_th_I': 20.,#threshold potential for inhibitory neurons
              'V_r': 0.,    #reset potential
              'C_m': 1.,    #membrane capacity
              'tau_E': 20., #membrane time constant for excitatory neurons
              'tau_I': 10., #membrane time constant for inhibitory neurons
              't_ref': 5.,  #refractory period
              'ps': np.array([[0.2, 0.5], [0.5, 0.5]]), #connection probabilities
              'ge': 1.2,    #excitatory synaptic weight factor
              'I_th_E': 1.25, #Background current for excitatory neurons in multiples of I_rheobase
              'I_th_I': 0.78, #Background current for inhibitory neurons in multiples of I_rheobase
              'dt': 0.1,    #simulation time step
              'neuron_type': 'iaf_psc_exp', #neuron model used in NEST iaf and gif are available
              'simtime': 10000, #simulation time in ms
              'warmup': 1000,  #warmup time in ms (time before recording starts)
              'Q': 20, #number of clusters
              'clustering': 'weight', #type of clustering used 'weight' or
              'stim_clusters': [2,3,4], #list of clusters which are stimulated
              'stim_starts': [2000,6000], #list of stimulation start times
              'stim_ends': [3500,7500], #list of stimulation end times
              'stim_amp': 0.15, #amplitude of the stimulation current in pA
              #'probabilities'
              }

    Rj= 0.82  # ration excitatory to inhibitory clustering
    Rep = 6.  # excitatory clustering strength

    # Creates object which creates the EI clustered network in NEST
    EI_Network = network.ClusteredNetwork.Clustered_Network_from_Rj_Rep(Rj, Rep, default, params)

    # Runs the simulation and returns the spiketimes
    # get simulation initializes the network in NEST and runs the simulation
    # it returns a dict with the average rates, the spiketimes and the used parameters
    Result = EI_Network.get_simulation()
    ax = general_helper.raster_plot(Result['spiketimes'], tlim=(0, params['simtime']),colorgroups=[
        ('k', 0, params['N_E']), ("darkred", params['N_E'], params['N_E'] + params['N_I'])])
    plt.savefig('Rasterplot.png')
    print('Firing rate of excitatory neurons: ' + str(Result['e_rate']) + ' Spikes/s')
    print('Firing rate of inhibitory neurons: ' + str(Result['i_rate']) + ' Spikes/s')

