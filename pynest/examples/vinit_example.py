# -*- coding: utf-8 -*-
#
# vinit_example.py
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
Plot several runs of the iaf_cond_exp_sfa_rr neuron with no input and
various initial values for the membrane potential.
"""

import nest 
import numpy
import pylab

for vinit in numpy.arange(-100, -50, 10, float):

    nest.ResetKernel()

    cbn = nest.Create('iaf_cond_exp_sfa_rr')

    # set the initial membrane potential
    nest.SetStatus(cbn, 'V_m', vinit)

    voltmeter = nest.Create('voltmeter')
    nest.SetStatus(voltmeter, {'withtime': True})
    nest.Connect(voltmeter, cbn)

    nest.Simulate(75.0)

    t = nest.GetStatus(voltmeter,"events")[0]["times"]
    v = nest.GetStatus(voltmeter,"events")[0]["V_m"]

    pylab.plot(t, v, label="initial V_m=%.2f mV" % vinit)

pylab.legend(loc=4)
pylab.xlabel("time (ms)")
pylab.ylabel("V_m (mV)")
