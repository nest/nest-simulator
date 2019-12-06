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

"""
Microcircuit stimulus parameters
--------------------------------

Stimulus parameters for the microcircuit.

Hendrik Rothe, Hannah Bos, Sacha van Albada; May 2016
"""

import numpy as np
from network_params import net_dict

stim_dict = {
    # Turn thalamic input on or off (True or False).
    'thalamic_input': False,
    # Turn DC input on or off (True or False).
    'dc_input': False,
    # Number of thalamic neurons.
    'n_thal': 902,
    # Mean amplitude of the thalamic postsynaptic potential (in mV).
    'PSP_th': 0.15,
    # Standard deviation of the postsynaptic potential (in relative units).
    'PSP_sd': 0.1,
    # Start of the thalamic input (in ms).
    'th_start': 700.0,
    # Duration of the thalamic input (in ms).
    'th_duration': 10.0,
    # Rate of the thalamic input (in Hz).
    'th_rate': 120.0,
    # Start of the DC generator (in ms).
    'dc_start': 0.0,
    # Duration of the DC generator (in ms).
    'dc_dur': 1000.0,
    # Connection probabilities of the thalamus to the different populations.
    # Order as in 'populations' in 'network_params.py'
    'conn_probs_th':
        np.array([0.0, 0.0, 0.0983, 0.0619, 0.0, 0.0, 0.0512, 0.0196]),
    # Mean delay of the thalamic input (in ms).
    'delay_th':
        np.asarray([1.5 for i in list(range(len(net_dict['populations'])))]),
    # Standard deviation of the thalamic delay (in ms).
    'delay_th_sd':
        np.asarray([0.75 for i in list(range(len(net_dict['populations'])))]),
    # Amplitude of the DC generator (in pA).
    'dc_amp': np.ones(len(net_dict['populations'])) * 0.3,
    }
