# -*- coding: utf-8 -*-
#
# test_gif_pop_psc_exp.py
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
import numpy as np
import pytest


@pytest.mark.skipif_missing_gsl
class TestGifPopPscExp:
    r"""
    Compare mean and variance of population rate of the model with desired values

    This script simulate an inhibitory population with the population model. After each simulation, it calculates
    the mean and variance of population rate and compare them with desired values. The simulated values should be
    in a acceptable margin.
    """

    def test_gif_pop_psc_exp(self):
        err_mean = 1.0  # error margin for mean rate (Hz)
        err_var = 6.0  # error margin for variance of rate (Hz^2)
        res = 0.5  # simulation time step (ms)
        T = 10000.0  # simulation duration (ms)
        start_time = 1000.0  # time for starting calculating mean and variance (ms)

        start_step = int(start_time / res)
        remaining_step = int((T - 1) / res) - start_step

        pop_size = 500
        expected_rate = 22.0
        expected_var = 102.0

        nest.ResetKernel()
        nest.resolution = res

        node = nest.Create(
            "gif_pop_psc_exp",
            params={
                "N": pop_size,
                "V_reset": 0.0,
                "V_T_star": 10.0,
                "E_L": 0.0,
                "Delta_V": 2.0,
                "C_m": 250.0,
                "tau_m": 20.0,
                "t_ref": 4.0,
                "I_e": 500.0,
                "lambda_0": 10.0,
                "tau_syn_in": 2.0,
                "tau_sfa": [500.0],
                "q_sfa": [1.0],
            },
        )

        nest.Connect(node, node, syn_spec={"synapse_model": "static_synapse", "delay": 1.0, "weight": -6.25})

        vm = nest.Create("voltmeter", params={"record_from": ["n_events"], "interval": res})
        nest.Connect(vm, node)

        nest.Simulate(T)

        nspike = vm.events["n_events"][start_step:]

        mean_nspike = np.mean(nspike)
        mean_rate = mean_nspike / pop_size / res * 1000.0  # convert to mean rate

        var_nspike = np.var(nspike)
        var_nspike = var_nspike / pop_size / res * 1000.0
        var_rate = var_nspike / pop_size / res * 1000.0  # convert to var of rate

        assert abs(mean_rate - expected_rate) <= err_mean
        assert abs(var_rate - expected_var) <= err_var
