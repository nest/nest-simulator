# -*- coding: utf-8 -*-
#
# gif_population.py
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

'''
Population of GIF neuron model with oscillatory behavior
--------------------------------------------------------

This script simulates a population of generalized integrate-and-fire (GIF)
neuron model receives noise from a group of Poisson neurons.

Due to spike-frequency adaptation, the GIF neurons tend to show oscillatory
behavior on the time scale comparable with the time constant of adaptation
elements (stc and sfa).

Using spike detector, population dynamics (raster plot and average firing rate)
are shown.
'''

'''
Import all necessary modules for simulation and plotting .
'''

import nest
import nest.raster_plot
import matplotlib.pyplot as plt

'''
Assigning the simulation parameters to variables.
'''

dt = 0.1
simtime = 2000.0

'''
Definition of neural parameters for the GIF model. These parameters are
extracted by fitting the model to experimental data: Mensi, S., Naud, R.,
Pozzorini, C., Avermann, M., Petersen, C.C. and Gerstner, W., 2012. Parameter
extraction and classification of three cortical neuron types reveals two
distinct adaptation mechanisms. Journal of neurophysiology, 107(6),
pp.1756-1775.
'''

neuron_params = {"C_m": 83.1,
                 "g_L": 3.7,
                 "E_L": -67.0,
                 "Delta_V": 1.4,
                 "V_T_star": -39.6,
                 "t_ref": 4.0,
                 "V_reset": -36.7,
                 "lambda_0": 1.0,
                 "q_stc": [56.7, -6.9],
                 "tau_stc": [57.8, 218.2],
                 "q_sfa": [11.7, 1.8],
                 "tau_sfa": [53.8, 640.0],
                 "tau_syn_ex": 10.0,
                 }

'''
Definition of the parameters for the population of GIF neurons.
'''

N_ex = 100
p_ex = 0.3
w_ex = 30.0  # pA

'''
Definition of the parameters for the Poisson group and its connection with GIF
neurons population.
'''

N_noise = 50
rate_noise = 10.0  # Hz
w_noise = 20.0  # pA

'''
Configuration of the simulation kernel with the previously defined time
resolution.
'''

nest.SetKernelStatus({"resolution": dt})

'''
Building a population of GIF neurons, a group of Poisson neurons and a
spike detector device for capturing spike times of the population.
'''

population = nest.Create("gif_psc_exp", N_ex, params=neuron_params)

noise = nest.Create("poisson_generator", N_noise, params={'rate': rate_noise})

spikes_det = nest.Create("spike_detector")


'''
Building connections inside the population of GIF neurons population, between
Poisson group and the population, and also connecting spike detector to the
population.
'''

nest.Connect(
    population, population, {'rule': 'pairwise_bernoulli', 'p': p_ex},
    syn_spec={"weight": w_ex}
    )

nest.Connect(noise, population, 'all_to_all', syn_spec={"weight": w_noise})

nest.Connect(population, spikes_det)

'''
Simulation of the network.
'''

nest.Simulate(simtime)

'''
Plotting the results of simulation including raster plot and histogram of
population activity.
'''

nest.raster_plot.from_device(spikes_det, hist=True)
plt.title('Population dynamics')
plt.show()
