# -*- coding: utf-8 -*-
#
# one_neuron_with_sine_wave.py
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

import nest
import nest.voltage_trace

nest.ResetKernel()

neuron = nest.Create('iaf_neuron')

sine = nest.Create('ac_generator', 1, 
                   {'amplitude': 100.0,
                    'frequency': 2.0})

noise = nest.Create('poisson_generator', 2,
                    [{'rate': 70000.0}, 
                     {'rate': 20000.0}])

voltmeter = nest.Create('voltmeter',1,
                        {'withgid': True})

nest.Connect(sine, neuron)
nest.Connect(voltmeter, neuron)
nest.Connect(noise[:1], neuron, syn_spec={'weight': 1.0, 'delay': 1.0})
nest.Connect(noise[1:], neuron, syn_spec={'weight': -1.0, 'delay': 1.0})

nest.Simulate(1000.0)

nest.voltage_trace.from_device(voltmeter)

import matplotlib.pyplot as plt
plt.savefig('../figures/voltage_trace.eps')
