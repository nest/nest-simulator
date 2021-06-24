# -*- coding: utf-8 -*-
#
# correlospinmatrix_detector_two_neuron.py
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
Correlospinmatrix detector example
----------------------------------

This scripts simulates two connected binary neurons, similar
as in [1]_. It measures and plots the auto- and cross covariance functions
of the individual neurons and between them, repsectively.

References
~~~~~~~~~~

.. [1] Ginzburg and Sompolinsky (1994). Theory of correlations in stochastic neural networks. 50(4) p. 3175. Fig. 1.

"""

import matplotlib.pyplot as plt
import nest
import numpy as np

m_x = 0.5
tau_m = 10.
h = 0.1
T = 1000000.
tau_max = 100.

csd = nest.Create("correlospinmatrix_detector")
csd.set(N_channels=2, tau_max=tau_max, Tstart=tau_max, delta_tau=h)

n1 = nest.Create("ginzburg_neuron")
n1.set(theta=0.0, tau_m=tau_m, c_1=0.0, c_2=2. * m_x, c_3=1.0)

n2 = nest.Create("mcculloch_pitts_neuron")
n2.set(theta=0.5, tau_m=tau_m)

nest.Connect(n1, n2, syn_spec={"weight": 1.0})

nest.Connect(n1, csd, syn_spec={"receptor_type": 0})
nest.Connect(n2, csd, syn_spec={"receptor_type": 1})

nest.Simulate(T)

count_covariance = csd.count_covariance

mean_activities = np.zeros(2, dtype=float)
for i in range(2):
    mean_activities[i] = count_covariance[i][i][int(tau_max / h)] * (h / T)

print('mean activities =', mean_activities)

covariance_matrix = np.zeros((2, 2, int(2 * tau_max / h) + 1), dtype=float)
for i in range(2):
    for j in range(2):
        covariance_matrix[i, j] = count_covariance[i][j] * (h / T) - mean_activities[i] * mean_activities[j]

ts = np.arange(-tau_max, tau_max + h, h)

plt.title("auto- and cross covariance functions")

plt.plot(ts, covariance_matrix[0, 1], 'r', label=r"$c_{12}$")
plt.plot(ts, covariance_matrix[1, 0], 'b', label=r"$c_{21}$")
plt.plot(ts, covariance_matrix[0, 0], 'g', label=r"$c_{11}$")
plt.plot(ts, covariance_matrix[1, 1], 'y', label=r"$c_{22}$")
plt.xlabel(r"time $t \; \mathrm{ms}$")
plt.ylabel(r"$c$")
plt.legend()

plt.show()
