# -*- coding: utf-8 -*-
#
# hh_phaseplane.py
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

"""Numerical phase-plane analysis of the Hodgkin-Huxley neuron
----------------------------------------------------------------

hh_phaseplane makes a numerical phase-plane analysis of the Hodgkin-Huxley
neuron (``hh_psc_alpha``). Dynamics is investigated in the V-n space (see remark
below). A constant DC can be specified  and its influence on the nullclines
can be studied.

Remark
~~~~~~~~

To make the two-dimensional analysis possible, the (four-dimensional)
Hodgkin-Huxley formalism needs to be artificially reduced to two dimensions,
in this case by 'clamping' the two other variables, `m` and `h`, to
constant values (`m_eq` and `h_eq`).

"""

import nest
import numpy as np
from matplotlib import pyplot as plt


amplitude = 100.  # Set externally applied current amplitude in pA
dt = 0.1  # simulation step length [ms]

v_min = -100.  # Min membrane potential
v_max = 42.  # Max membrane potential
n_min = 0.1  # Min inactivation variable
n_max = 0.81  # Max inactivation variable
delta_v = 2.  # Membrane potential step length
delta_n = 0.01  # Inactivation variable step length

V_vec = np.arange(v_min, v_max, delta_v)
n_vec = np.arange(n_min, n_max, delta_n)

num_v_steps = len(V_vec)
num_n_steps = len(n_vec)

nest.ResetKernel()
nest.set_verbosity('M_ERROR')

nest.SetKernelStatus({'resolution': dt})
neuron = nest.Create('hh_psc_alpha')

# Numerically obtain equilibrium state
nest.Simulate(1000)

m_eq = nest.GetStatus(neuron)[0]['Act_m']
h_eq = nest.GetStatus(neuron)[0]['Inact_h']

nest.SetStatus(neuron, {'I_e': amplitude})  # Apply external current

# Scan state space
print('Scanning phase space')

V_matrix = np.zeros([num_n_steps, num_v_steps])
n_matrix = np.zeros([num_n_steps, num_v_steps])

# pp_data will contain the phase-plane data as a vector field
pp_data = np.zeros([num_n_steps * num_v_steps, 4])

count = 0
for i, V in enumerate(V_vec):
    for j, n in enumerate(n_vec):
        # Set V_m and n
        nest.SetStatus(neuron, {'V_m': V, 'Act_n': n,
                                'Act_m': m_eq, 'Inact_h': h_eq})
        # Find state
        V_m = nest.GetStatus(neuron)[0]['V_m']
        Act_n = nest.GetStatus(neuron)[0]['Act_n']

        # Simulate a short while
        nest.Simulate(dt)

        # Find difference between new state and old state
        V_m_new = nest.GetStatus(neuron)[0]['V_m'] - V
        Act_n_new = nest.GetStatus(neuron)[0]['Act_n'] - n

        # Store in vector for later analysis
        V_matrix[j, i] = abs(V_m_new)
        n_matrix[j, i] = abs(Act_n_new)
        pp_data[count] = np.array([V_m, Act_n, V_m_new, Act_n_new])

        if count % 10 == 0:
            # Write updated state next to old state
            print('')
            print('Vm:  \t', V_m)
            print('new Vm:\t', V_m_new)
            print('Act_n:', Act_n)
            print('new Act_n:', Act_n_new)

        count += 1

# Set state for AP generation
nest.SetStatus(neuron, {'V_m': -34., 'Act_n': 0.2,
                        'Act_m': m_eq, 'Inact_h': h_eq})

print('')
print('AP-trajectory')
# ap will contain the trace of a single action potential as one possible
# numerical solution in the vector field
ap = np.zeros([1000, 2])
for i in range(1, 1001):
    # Find state
    V_m = nest.GetStatus(neuron)[0]['V_m']
    Act_n = nest.GetStatus(neuron)[0]['Act_n']

    if i % 10 == 0:
        # Write new state next to old state
        print('Vm: \t', V_m)
        print('Act_n:', Act_n)
    ap[i - 1] = np.array([V_m, Act_n])

    # Simulate again
    nest.SetStatus(neuron, {'Act_m': m_eq, 'Inact_h': h_eq})
    nest.Simulate(dt)

# Make analysis
print('')
print('Plot analysis')

nullcline_V = []
nullcline_n = []

print('Searching nullclines')
for i in range(0, len(V_vec)):
    index = np.nanargmin(V_matrix[:][i])
    if index != 0 and index != len(n_vec):
        nullcline_V.append([V_vec[i], n_vec[index]])

    index = np.nanargmin(n_matrix[:][i])
    if index != 0 and index != len(n_vec):
        nullcline_n.append([V_vec[i], n_vec[index]])

print('Plotting vector field')
factor = 0.1
for i in range(0, np.shape(pp_data)[0], 3):
    plt.plot([pp_data[i][0], pp_data[i][0] + factor * pp_data[i][2]],
             [pp_data[i][1], pp_data[i][1] + factor * pp_data[i][3]],
             color=[0.6, 0.6, 0.6])

plt.plot(nullcline_V[:][0], nullcline_V[:][1], linewidth=2.0)
plt.plot(nullcline_n[:][0], nullcline_n[:][1], linewidth=2.0)

plt.xlim([V_vec[0], V_vec[-1]])
plt.ylim([n_vec[0], n_vec[-1]])

plt.plot(ap[:][0], ap[:][1], color='black', linewidth=1.0)

plt.xlabel('Membrane potential V [mV]')
plt.ylabel('Inactivation variable n')
plt.title('Phase space of the Hodgkin-Huxley Neuron')

plt.show()
