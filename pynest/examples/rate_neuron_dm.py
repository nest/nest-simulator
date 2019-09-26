# -*- coding: utf-8 -*-
#
# rate_neuron_dm.py
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

"""Rate neuron decision making
------------------------------------

A binary decision is implemented in the form of two rate neurons
engaging in mutual inhibition.

Evidence for each decision is reflected by the mean input
experienced by the respective neuron.
The activity of each neuron is recorded using multimeter devices.

It can be observed how noise as well as the difference in evidence
affects which neuron exhibits larger activity and hence which
decision will be made.
"""

import nest
import pylab
import numpy

#####################################################################################
# First,the function ``build_network`` is defined to build the network and
# return the handles of two decision units and the ``Multimeter``


def build_network(sigma, dt):
    nest.ResetKernel()
    nest.SetKernelStatus({'resolution': dt, 'use_wfr': False})
    Params = {'lambda': 0.1, 'sigma': sigma, 'tau': 1., 'rectify_output': True}
    D1 = nest.Create('lin_rate_ipn', params=Params)
    D2 = nest.Create('lin_rate_ipn', params=Params)

    nest.Connect(D1, D2, 'all_to_all', {
        'synapse_model': 'rate_connection_instantaneous', 'weight': -0.2})
    nest.Connect(D2, D1, 'all_to_all', {
        'synapse_model': 'rate_connection_instantaneous', 'weight': -0.2})

    mm = nest.Create('multimeter')
    nest.SetStatus(mm, {'interval': dt, 'record_from': ['rate']})
    nest.Connect(mm, D1, syn_spec={'delay': dt})
    nest.Connect(mm, D2, syn_spec={'delay': dt})

    return D1, D2, mm


#####################################################################################
# The function ``build_network`` takes the noise parameter sigma
# and the time resolution as arguments.
# First, the Kernel is reset and the ``use_wfr`` (waveform-relaxation) is set to
# false while the resolution is set to the specified value `dt`.
# Two rate neurons with linear activation functions are created and the
# handle is stored in the variables `D1` and `D2`. The output of both decision
# units is rectified at zero.
# The two decisions units are coupled via mutual inhibition.
# Next the multimeter is created and the handle stored in mm and the option
# ``record_from`` is set. The multimeter is then connected to the two units
# in order to 'observe' them.  The ``Connect`` function takes the handles as input.

##################################################################################
# The decision making process is simulated for three different levels of noise
# and three differences in evidence for a given decision. The activity of both
# decision units is plotted for each scenario.

fig_size = [14, 8]
fig_rows = 3
fig_cols = 3
fig_plots = fig_rows * fig_cols
face = 'white'
edge = 'white'

ax = [None] * fig_plots
fig = pylab.figure(facecolor=face, edgecolor=edge, figsize=fig_size)

dt = 1e-3
sigma = [0.0, 0.1, 0.2]
dE = [0.0, 0.004, 0.008]
T = numpy.linspace(0, 200, 200 / dt - 1)
for i in range(9):

    c = i % 3
    r = int(i / 3)
    D1, D2, mm = build_network(sigma[r], dt)

###########################################################################
# First using build_network the network is build and the handles of
# the decision units and the multimeter are stored in `D1`, `D2` and `mm`

    nest.Simulate(100.0)
    nest.SetStatus(D1, {'mu': 1. + dE[c]})
    nest.SetStatus(D2, {'mu': 1. - dE[c]})
    nest.Simulate(100.0)

########################################################################
# The network is simulated using ``Simulate``, which takes the desired
# simulation time in milliseconds and advances the network state by
# this amount of time. After an initial period in the absence of evidence
# for either decision, evidence is given by changing the state of each

    senders = data[0]['events']['senders']
    voltages = data[0]['events']['rate']

########################################################################
# The activity values ('voltages') are read out by the multimeter

    ax[i] = fig.add_subplot(fig_rows, fig_cols, i + 1)
    ax[i].plot(T, voltages[numpy.where(senders == D1)],
               'b', linewidth=2, label="D1")
    ax[i].plot(T, voltages[numpy.where(senders == D2)],
               'r', linewidth=2, label="D2")
    ax[i].set_ylim([-.5, 12.])
    ax[i].get_xaxis().set_ticks([])
    ax[i].get_yaxis().set_ticks([])
    if c == 0:
        ax[i].set_ylabel("activity ($\sigma=%.1f$) " % (sigma[r]))
        ax[i].get_yaxis().set_ticks([0, 3, 6, 9, 12])

    if r == 0:
        ax[i].set_title("$\Delta E=%.3f$ " % (dE[c]))
        if c == 2:
            pylab.legend(loc=0)
    if r == 2:
        ax[i].get_xaxis().set_ticks([0, 50, 100, 150, 200])
        ax[i].set_xlabel('time (ms)')

########################################################################
# The activity of the two units is plotted in each scenario.
#
# In the absence of noise, the network will not make a decision if evidence
# for both choices is equal. With noise, this symmetry can be broken and a
# decision wil be taken despite identical evidence.
#
# As evidence for `D1` relative to `D2` increases, it becomes more likely that
# the corresponding decision will be taken. For small differences in the
# evidence for the two decisions, noise can lead to the 'wrong' decision.


pylab.show()
