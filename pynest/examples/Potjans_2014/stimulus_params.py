# -*- coding: utf-8 -*-
#
# stimulus_params.py
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

""" PyNEST Microcircuit: Stimulus Parameters
-----------------------------------------------

A dictionary with parameters for an optional external transient stimulation.
Thalamic input and DC input can be switched on individually.

"""

import numpy as np
from network_params import net_dict

stim_dict = {
    # optional thalamic input
    # turn thalamic input on or off (True or False)
    'thalamic_input': False,
    # start of the thalamic input (in ms)
    'th_start': 700.0,
    # duration of the thalamic input (in ms)
    'th_duration': 10.0,
    # rate of the thalamic input (in Hz)
    'th_rate': 120.0,
    # number of thalamic neurons
    'num_th_neurons': 902,
    # connection probabilities of the thalamus to the different populations
    # (order as in 'populations' in 'network_params.py')
    'conn_probs_th':
        np.array([0.0, 0.0, 0.0983, 0.0619, 0.0, 0.0, 0.0512, 0.0196]),
    # mean amplitude of the thalamic postsynaptic potential (in mV)
    # standard deviation will be tarken from 'network_parameters.py'
    'PSP_th': 0.15,
     # mean delay of the thalamic input (in ms)
    'delay_th':
        np.asarray([1.5 for i in list(range(len(net_dict['populations'])))]),
    # standard deviation of the thalamic delay (in ms)
    'delay_th_sd':
        np.asarray([0.75 for i in list(range(len(net_dict['populations'])))]),
  
    # optional DC input
    # turn DC input on or off (True or False)
    'dc_input': False,
    # start of the DC generator (in ms)
    'dc_start': 0.0,
    # duration of the DC generator (in ms)
    'dc_dur': 1000.0,
    # amplitude of the DC generator (in pA)
    # amplitude will be multiplied with 'K_ext' from 'network_parameters.py'
    'dc_amp': np.ones(len(net_dict['populations'])) * 0.3
    }
