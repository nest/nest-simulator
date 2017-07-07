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

'''
  hh_phaseplane makes a numerical phase-plane analysis of the Hodgkin-Huxley
  neuron (iaf_psc_alpha). Dynamics is investigated in the V-n space (see remark
  below). A constant DC can be specified  and its influence on the nullclines
  can be studied.

  REMARK
  To make the two-dimensional analysis possible, the (four-dimensional)
  Hodgkin-Huxley formalism needs to be artificially reduced to two dimensions,
  in this case by 'clamping' the two other variables, m an h, to
  constant values (m_eq and h_eq).
'''

import nest
from matplotlib import pyplot as plt


amplitude = 100.  # Set externally applied current amplitude in pA
dt = 0.1  # simulation step length [ms]

nest.ResetKernel()
nest.set_verbosity('M_ERROR')

nest.SetKernelStatus({'resolution': dt})
neuron = nest.Create('hh_psc_alpha')

# Numerically obtain equilibrium state
nest.Simulate(1000)

m_eq = nest.GetStatus(neuron)[0]['Act_m']
h_eq = nest.GetStatus(neuron)[0]['Act_h']

nest.SetStatus(neuron, {'I_e': amplitude})  # Apply external current

# Scan state space
print('Scanning phase space')

V_new_vec = []
n_new_vec = []
# x will contain the phase-plane data as a vector field
x = []
count = 0
for V in range(-100, 42, 2):
    n_V = []
    n_n = []
    for n in range(10, 81):
        # Set V_m and n
        nest.SetStatus(neuron, {'V_m': V*1.0, 'Inact_n': n/100.0,
                                'Act_m': m_eq, 'Act_h': h_eq})
        # Find state
        V_m = nest.GetStatus(neuron)[0]['V_m']
        Inact_n = nest.GetStatus(neuron)[0]['Inact_n']

        # Simulate a short while
        nest.Simulate(dt)

        # Find difference between new state and old state
        V_m_new = nest.GetStatus(neuron)[0]['V_m'] - V*1.0
        Inact_n_new = nest.GetStatus(neuron)[0]['Inact_n'] - n/100.0

        # Store in vector for later analysis
        n_V.append(abs(V_m_new))
        n_n.append(abs(Inact_n_new))
        x.append([V_m, Inact_n, V_m_new, Inact_n_new])

        if count % 10 == 0:
            # Write updated state next to old state
            print('')
            print('Vm:  \t', V_m)
            print('new Vm:\t', V_m_new)
            print('Inact_n:', Inact_n)
            print('new Inact_n:', Inact_n_new)

        count += 1
    # Store in vector for later analysis
    V_new_vec.append(n_V)
    n_new_vec.append(n_n)

# Set state for AP generation
nest.SetStatus(neuron, {'V_m': -34., 'Inact_n': 0.2,
                        'Act_m': m_eq, 'Act_h': h_eq})

print('')
print('AP-trajectory')
# ap will contain the trace of a single action potential as one possible
# numerical solution in the vector field
ap = []
for i in range(1, 1001):
    # Find state
    V_m = nest.GetStatus(neuron)[0]['V_m']
    Inact_n = nest.GetStatus(neuron)[0]['Inact_n']

    if i % 10 == 0:
        # Write new state next to old state
        print('Vm: \t', V_m)
        print('Inact_n:', Inact_n)
    ap.append([V_m, Inact_n])

    # Simulate again
    nest.SetStatus(neuron, {'Act_m': m_eq, 'Act_h': h_eq})
    nest.Simulate(dt)

# Make analysis
print('')
print('Plot analysis')

V_matrix = [list(x) for x in zip(*V_new_vec)]
n_matrix = [list(x) for x in zip(*n_new_vec)]
n_vec = [x/100. for x in range(10, 81)]
V_vec = [x*1. for x in range(-100, 42, 2)]

nullcline_V = []
nullcline_n = []

print('Searching nullclines')
for i in range(0, len(V_vec)):
    index = V_matrix[:][i].index(min(V_matrix[:][i]))
    if index != 0 and index != len(n_vec):
        nullcline_V.append([V_vec[i], n_vec[index]])

    index = n_matrix[:][i].index(min(n_matrix[:][i]))
    if index != 0 and index != len(n_vec):
        nullcline_n.append([V_vec[i], n_vec[index]])

print('Plotting vector field')
factor = 0.1
for i in range(0, count, 3):
    plt.plot([x[i][0], x[i][0] + factor*x[i][2]],
             [x[i][1], x[i][1] + factor*x[i][3]], color=[0.6, 0.6, 0.6])

plt.plot(nullcline_V[:][0], nullcline_V[:][1], linewidth=2.0)
plt.plot(nullcline_n[:][0], nullcline_n[:][1], linewidth=2.0)

plt.xlim([V_vec[0], V_vec[-1]])
plt.ylim([n_vec[0], n_vec[-1]])

plt.plot(ap[:][0], ap[:][1], color='black', linewidth=1.0)

plt.xlabel('Membrane potential V [mV]')
plt.ylabel('Inactivation variable n')
plt.title('Phase space of the Hodgkin-Huxley Neuron')

plt.show()
