# -*- coding: utf-8 -*-
#
# test_poisson_generator_campbell_alpha.py
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
Tests that the mean membrane potential of 1000 iaf_psc_alpha neuron
receiving input from a poisson_generator reaches the expected value.
Expected value is given by
tau_m / C_m * nu * J / tau_syn_ex * int_0^inf t * exp(1-t/tau_syn_ex)
= tau_m / C_m * nu * J * tau_syn_ex * exp(1)
where nu is the rate of the poisson generator.
"""


import nest
import numpy as np
import pytest


def test_poisson_generator_alpha():
    nest.ResetKernel()
    nest.resolution = 0.1
    neuron_params = {"tau_m": 5.0, "tau_syn_ex": 0.5, "E_L": 0.0, "V_th": 999.0, "C_m": 1.0, "V_m": 0.0}

    nest.SetDefaults("iaf_psc_alpha", neuron_params)
    neuron = nest.Create("iaf_psc_alpha", 1000)

    tolerance = 0.05
    V_m_target = 20.0
    J = 0.01
    # solve for rate, convert to spks/s, drop C_m as it is 1.
    rate = 1000.0 * V_m_target / (J * neuron_params["tau_m"] * np.exp(1) * neuron_params["tau_syn_ex"])

    poisson = nest.Create("poisson_generator")
    poisson.rate = rate
    syn_spec = {"synapse_model": "static_synapse", "weight": J, "delay": 0.1}
    nest.Connect(poisson, neuron, "all_to_all", syn_spec)

    nest.Simulate(200.0)
    print(np.mean(neuron.V_m))
    assert np.abs(np.mean(neuron.V_m) - V_m_target) < tolerance
