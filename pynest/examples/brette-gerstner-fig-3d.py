# -*- coding: utf-8 -*-
#
# brette-gerstner-fig-3d.py
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

# Test for the adapting exponential integrate and fire model according to
# Brette and Gerstner (2005) J. Neurophysiology.
# This script reproduces figure 3.d of the paper.
# Note that Brette&Gerstner give the value for b in nA.
# To be consistent with the other parameters in the equations, b must be
# converted to pA (pico Ampere).

import nest
import nest.voltage_trace
import pylab

nest.ResetKernel()

res=0.1
nest.SetKernelStatus({"resolution": res})
neuron=nest.Create("aeif_cond_exp")
nest.SetStatus(neuron, {"V_peak":20., "E_L":-60.0, "a":80.0, "b":80.5, "tau_w": 720.0})
dc=nest.Create("dc_generator")

nest.SetStatus(dc,[{"amplitude":-800.0, "start":0.0, "stop":400.0}])

nest.ConvergentConnect(dc,neuron)

voltmeter= nest.Create("voltmeter")
nest.SetStatus(voltmeter, {"withgid": True, "withtime": True, 'interval':0.1})

nest.Connect(voltmeter,neuron)

nest.Simulate(1000.0)

nest.voltage_trace.from_device(voltmeter)
pylab.axis([0,1000,-85,0])
