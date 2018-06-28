# -*- coding: utf-8 -*-
#
# plot_twoneurons.py
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
Two neurons
-----------

'''

import pylab

import nest
import nest.voltage_trace

weight = 20.0
delay = 1.0
stim = 1000.0

neuron1 = nest.Create("iaf_psc_alpha")
neuron2 = nest.Create("iaf_psc_alpha")
voltmeter = nest.Create("voltmeter")

nest.SetStatus(neuron1, {"I_e": stim})
nest.Connect(neuron1, neuron2, syn_spec={'weight': weight, 'delay': delay})
nest.Connect(voltmeter, neuron2)

nest.Simulate(100.0)

nest.voltage_trace.from_device(voltmeter)
nest.voltage_trace.show()
