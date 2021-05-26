# -*- coding: utf-8 -*-
#
# gif_cond_exp_multisynapse.py
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
gif_cond_exp_multisynapse
-------------------------

"""

import nest
import numpy as np

neuron = nest.Create('gif_cond_exp_multisynapse',
                     params={'E_rev': [0.0, -85.0],
                             'tau_syn': [4.0, 8.0]})

spike = nest.Create('spike_generator', params={'spike_times':
                                               np.array([10.0])})

delays = [1., 30.]
w = [1., 5.]
for syn in range(2):
    nest.Connect(spike, neuron, syn_spec={'synapse_model': 'static_synapse',
                                          'receptor_type': 1 + syn,
                                          'weight': w[syn],
                                          'delay': delays[syn]})
nest.Simulate(100.)
