# -*- coding: utf-8 -*-
#
# test_gif_exp.py
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
@pytest.mark.parametrize(
    "model", ["gif_psc_exp", "gif_cond_exp", "gif_psc_exp_multisynapse", "gif_cond_exp_multisynapse"]
)
class TestGifModels:
    r"""
    Test of gif_cond_exp_multisynapse and gif_psc_exp with external DC current and spike generators

    Inject external DC current and also three spikes to the neuron and measure its
    spike times. The spike times should match the expected values. The expected
    spike times are computed by the Python code used for the publication
    (Code/GIF.py in http://wiki.epfl.ch/giftoolbox/documents/GIF_Toolbox.zip).
    In order to use the Python code, the adaptation parameters (q_stc/sfa) should
    be converted using the formula described in the model documentation.
    """

    def run_simulation(self, model, params=None):
        nest.ResetKernel()
        nest.resolution = 0.1
        nest.rng_seed = 1

        if not params:
            params = {}

        if "multisynapse" in model:
            params["tau_syn"] = [8.0, 4.0]
            if "cond" in model:
                params["E_rev"] = [0.0, 0.0]

        neuron = nest.Create(model, params=params)

        dc_gen = nest.Create("dc_generator", params={"amplitude": 170.0})
        nest.Connect(dc_gen, neuron)

        sg = nest.Create("spike_generator", params={"spike_times": [10.0, 20.0, 30.0]})
        if "multisynapse" in model:
            sg2 = nest.Create("spike_generator", params={"spike_times": [15.0, 25.0, 35.0]})
            nest.Connect(sg, neuron, syn_spec={"receptor_type": 1})
            nest.Connect(sg2, neuron, syn_spec={"receptor_type": 2})
        else:
            nest.Connect(sg, neuron)

        sr = nest.Create("spike_recorder", params={"time_in_steps": True})
        nest.Connect(neuron, sr)

        nest.Simulate(150.0)

        return sr.get("events")["times"]

    def test_gif_exp_wrong_params(self, model):
        """Test for wrong parameters (negative lambda)"""
        params = {"lambda_0": -10.0}
        with pytest.raises(nest.kernel.NESTError):
            self.run_simulation(model, params)

    def test_gif_exp_wrong_params2(self, model):
        """Test for wrong parameters (unequal size of arrays)"""
        params = {"tau_sfa": 120.0, "q_sfa": [10.0, 25.0]}
        with pytest.raises(nest.kernel.NESTError):
            self.run_simulation(model, params)

    def test_gif_exp_default_params(self, model):
        """Test default parameters"""
        times = self.run_simulation(model)
        if "gif_cond_exp" in model:
            if "multisynapse" in model:
                np.testing.assert_allclose(times, [274, 571, 988, 1400])
            else:
                np.testing.assert_allclose(times, [398, 740, 1101, 1500])
        else:
            assert "gif_psc_exp" in model
            np.testing.assert_allclose(times, [462, 850, 1288])

    def test_gif_exp_defined_params(self, model):
        """Test defined parameters"""
        params = {
            "C_m": 40.0,
            "Delta_V": 0.2,
            "tau_sfa": [120.0, 10.0],
            "q_sfa": [10.0, 25.0],
            "tau_stc": [10.0, 20.0],
            "q_stc": [20.0, -5.0],
        }

        if "multisynapse" not in model:
            params["tau_syn_ex"] = 8.0

        times = self.run_simulation(model, params)

        if "gif_cond_exp" in model:
            if "multisynapse" in model:
                np.testing.assert_allclose(times, [165, 438])
            else:
                np.testing.assert_allclose(times, [171, 676])
        else:
            assert "gif_psc_exp" in model
            np.testing.assert_allclose(times, [207, 740])
