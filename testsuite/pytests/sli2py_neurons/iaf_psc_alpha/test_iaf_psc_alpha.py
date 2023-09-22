# -*- coding: utf-8 -*-
#
# test_iaf_psc_alpha.py
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

import dataclasses
import math

import nest
import numpy as np
import pytest
import testsimulation
import testutil
from scipy.special import lambertw

# Notes:
# * copy docs
# * add docs & examples
# * add comments
# * restructure
# * test min delay stuff tests sim indep of mindelay, move out?
# * `test_iaf_ps_dc_accuracy` tests kernel precision, move out?


@dataclasses.dataclass
class IAFPSCAlphaSimulation(testsimulation.Simulation):
    def setup(self):
        self.neuron = nest.Create("iaf_psc_alpha")
        vm = self.voltmeter = nest.Create("voltmeter")
        vm.interval = self.resolution
        sr = self.spike_recorder = nest.Create("spike_recorder")
        nest.Connect(vm, self.neuron, syn_spec={"weight": 1.0, "delay": self.delay})
        nest.Connect(self.neuron, sr, syn_spec={"weight": 1.0, "delay": self.delay})

    @property
    def spikes(self):
        return np.column_stack(
            (
                self.spike_recorder.events["senders"],
                self.spike_recorder.events["times"],
            )
        )


@dataclasses.dataclass
class MinDelaySimulation(IAFPSCAlphaSimulation):
    amplitude: float = 1000.0
    min_delay: float = 0.0

    def setup(self):
        dc = self.dc_generator = nest.Create("dc_generator")
        dc.amplitude = self.amplitude

        super().setup()

        nest.Connect(
            dc,
            self.neuron,
            syn_spec={"weight": 1.0, "delay": self.delay},
        )


@testutil.use_simulation(IAFPSCAlphaSimulation)
class TestIAFPSCAlpha:
    def test_iaf_psc_alpha(self, simulation):
        dc = simulation.dc_generator = nest.Create("dc_generator")
        dc.amplitude = 1000

        simulation.setup()

        nest.Connect(
            dc,
            simulation.neuron,
            syn_spec={"weight": 1.0, "delay": simulation.resolution},
        )

        results = simulation.simulate()

        actual, expected = testutil.get_comparable_timesamples(results, expected_default)
        assert actual == expected

    @pytest.mark.parametrize("duration", [20.0])
    def test_iaf_psc_alpha_fudge(self, simulation):
        simulation.setup()

        tau_m = 20
        tau_syn = 0.5
        C_m = 250.0
        a = tau_m / tau_syn
        b = 1.0 / tau_syn - 1.0 / tau_m
        t_max = 1.0 / b * (-lambertw(-math.exp(-1.0 / a) / a, k=-1) - 1.0 / a).real
        V_max = (
            math.exp(1)
            / (tau_syn * C_m * b)
            * ((math.exp(-t_max / tau_m) - math.exp(-t_max / tau_syn)) / b - t_max * math.exp(-t_max / tau_syn))
        )
        simulation.neuron.set(tau_m=tau_m, tau_syn_ex=tau_syn, tau_syn_in=tau_syn, C_m=C_m)
        sg = nest.Create(
            "spike_generator",
            params={"precise_times": False, "spike_times": [simulation.resolution]},
        )
        nest.Connect(
            sg,
            simulation.neuron,
            syn_spec={"weight": float(1.0 / V_max), "delay": simulation.resolution},
        )

        results = simulation.simulate()

        actual_t_max = results[np.argmax(results[:, 1]), 0]
        assert actual_t_max == pytest.approx(t_max + 0.2, abs=0.05)

    def test_iaf_psc_alpha_i0(self, simulation):
        simulation.setup()

        simulation.neuron.I_e = 1000

        results = simulation.simulate()

        actual, expected = testutil.get_comparable_timesamples(results, expected_i0)
        assert actual == expected
        assert simulation.spikes == pytest.approx(expected_i0_t)

    @pytest.mark.parametrize("resolution", [0.1, 0.2, 0.5, 1.0])
    def test_iaf_psc_alpha_i0_refractory(self, simulation):
        simulation.setup()

        simulation.neuron.I_e = 1450

        results = simulation.simulate()

        actual, expected = testutil.get_comparable_timesamples(results, expected_i0_refr)
        assert actual == expected


@pytest.mark.parametrize("min_delay", [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 1.0, 2.0])
@pytest.mark.parametrize("delay, duration", [(2.0, 10.5)])
@testutil.use_simulation(MinDelaySimulation)
class TestMinDelayUsingIAFPSCAlpha:
    def test_iaf_psc_alpha_mindelay_create(self, simulation, min_delay):
        simulation.setup()

        # Connect 2 throwaway neurons with `min_delay` to force `min_delay`
        nest.Connect(*nest.Create("iaf_psc_alpha", 2), syn_spec={"delay": min_delay, "weight": 1.0})

        results = simulation.simulate()

        actual, expected = testutil.get_comparable_timesamples(results, expected_mindelay)
        assert actual == expected

    def test_iaf_psc_alpha_mindelay_set(self, simulation, min_delay, delay):
        nest.set(min_delay=min_delay, max_delay=delay)
        nest.SetDefaults("static_synapse", {"delay": delay})

        simulation.setup()

        results = simulation.simulate()

        actual, expected = testutil.get_comparable_timesamples(results, expected_mindelay)
        assert actual == expected

    def test_iaf_psc_alpha_mindelay_simblocks(self, simulation, min_delay, delay):
        nest.set(min_delay=min_delay, max_delay=delay)
        nest.SetDefaults("static_synapse", {"delay": delay})

        simulation.setup()

        for _ in range(22):
            nest.Simulate(0.5)
        # duration=0 so that `simulation.simulate` is noop but
        # still extracts results for us.
        simulation.duration = 0
        results = simulation.simulate()

        actual, expected = testutil.get_comparable_timesamples(results, expected_mindelay)
        assert actual == expected


def test_kernel_precision():
    nest.ResetKernel()
    nest.set(tics_per_ms=2**14, resolution=2**0)
    assert math.frexp(nest.ms_per_tic) == (0.5, -13)


@dataclasses.dataclass
class DCAccuracySimulation(testsimulation.Simulation):
    # Don't autoset the resolution in the fixture, we do it in setup.
    set_resolution = False
    model: str = "iaf_psc_alpha"
    params: dict = dataclasses.field(default_factory=dict)

    def setup(self):
        nest.ResetKernel()
        nest.set(tics_per_ms=2**14, resolution=self.resolution)
        self.neuron = nest.Create(self.model, params=self.params)


@testutil.use_simulation(DCAccuracySimulation)
@pytest.mark.parametrize(
    "model",
    [
        "iaf_psc_alpha_ps",
        "iaf_psc_delta_ps",
        "iaf_psc_exp_ps",
        "iaf_psc_exp_ps_lossless",
    ],
)
@pytest.mark.parametrize("resolution", [2**i for i in range(0, -14, -1)])
class TestIAFPSDCAccuracy:
    @pytest.mark.parametrize(
        "params",
        [
            {
                "E_L": 0.0,  # resting potential in mV
                "V_m": 0.0,  # initial membrane potential in mV
                "V_th": 2000.0,  # spike threshold in mV
                "I_e": 1000.0,  # DC current in pA
                "tau_m": 10.0,  # membrane time constant in ms
                "C_m": 250.0,  # membrane capacity in pF
            }
        ],
    )
    @pytest.mark.parametrize("duration, tolerance", [(5, 1e-13), (500.0, 1e-9)])
    def test_iaf_ps_dc_accuracy(self, simulation, duration, tolerance, params):
        simulation.run()
        # Analytical solution
        V = params["I_e"] * params["tau_m"] / params["C_m"] * (1.0 - math.exp(-duration / params["tau_m"]))
        # Check that membrane potential is within tolerance of analytical solution.
        assert math.fabs(simulation.neuron.V_m - V) < tolerance

    @pytest.mark.parametrize(
        "params",
        [
            {
                "E_L": 0.0,  # resting potential in mV
                "V_m": 0.0,  # initial membrane potential in mV
                "V_th": 15.0,  # spike threshold in mV
                "I_e": 1000.0,  # DC current in pA
                "tau_m": 10.0,  # membrane time constant in ms
                "C_m": 250.0,  # membrane capacity in pF
            }
        ],
    )
    @pytest.mark.parametrize("duration, tolerance", [(5, 1e-13)])
    def test_iaf_ps_dc_t_accuracy(self, simulation, params, tolerance):
        simulation.run()
        t = -params["tau_m"] * math.log(1.0 - (params["C_m"] * params["V_th"]) / (params["tau_m"] * params["I_e"]))
        assert math.fabs(simulation.neuron.t_spike - t) < tolerance


expected_default = np.array(
    [
        [0.1, -70],
        [0.2, -70],
        [0.3, -69.602],
        [0.4, -69.2079],
        [0.5, -68.8178],
        [0.6, -68.4316],
        [0.7, -68.0492],
        [0.8, -67.6706],
        [0.9, -67.2958],
        [1.0, -66.9247],
        [4.5, -56.0204],
        [4.6, -55.7615],
        [4.7, -55.5051],
        [4.8, -55.2513],
        [4.9, -55.0001],
        [5.0, -70],
        [5.1, -70],
        [5.2, -70],
        [5.3, -70],
        [5.4, -70],
        [5.5, -70],
        [5.6, -70],
        [5.7, -70],
        [5.8, -70],
        [5.9, -70],
        [6.0, -70],
        [6.1, -70],
        [6.2, -70],
        [6.3, -70],
        [6.4, -70],
        [6.5, -70],
        [6.6, -70],
        [6.7, -70],
        [6.8, -70],
        [6.9, -70],
        [7.0, -70],
        [7.1, -69.602],
        [7.2, -69.2079],
        [7.3, -68.8178],
        [7.4, -68.4316],
        [7.5, -68.0492],
        [7.6, -67.6706],
        [7.7, -67.2958],
        [7.8, -66.9247],
        [7.9, -66.5572],
    ]
)

expected_i0 = np.array(
    [
        [0.1, -69.602],
        [0.2, -69.2079],
        [0.3, -68.8178],
        [0.4, -68.4316],
        [0.5, -68.0492],
        [4.3, -56.0204],
        [4.4, -55.7615],
        [4.5, -55.5051],
        [4.6, -55.2513],
        [4.7, -55.0001],
        [4.8, -70],
        [4.9, -70],
        [5.0, -70],
    ]
)

expected_i0_t = np.array(
    [
        [1, 4.8],
    ]
)

expected_i0_refr = np.array(
    [
        [0.1, -69.4229],
        [0.2, -68.8515],
        [0.3, -68.2858],
        [0.4, -67.7258],
        [0.5, -67.1713],
        [0.6, -66.6223],
        [0.7, -66.0788],
        [0.8, -65.5407],
        [0.9, -65.008],
        [1.0, -64.4806],
        [1.1, -63.9584],
        [1.2, -63.4414],
        [1.3, -62.9295],
        [1.4, -62.4228],
        [1.5, -61.9211],
        [1.6, -61.4243],
        [1.7, -60.9326],
        [1.8, -60.4457],
        [1.9, -59.9636],
        [2.0, -59.4864],
        [2.1, -59.0139],
        [2.2, -58.5461],
        [2.3, -58.0829],
        [2.4, -57.6244],
        [2.5, -57.1704],
        [2.6, -56.721],
        [2.7, -56.276],
        [2.8, -55.8355],
        [2.9, -55.3993],
        [3.0, -70],
        [3.1, -70],
        [3.2, -70],
        [3.3, -70],
        [3.4, -70],
        [3.5, -70],
        [3.6, -70],
        [3.7, -70],
        [3.8, -70],
        [3.9, -70],
        [4.0, -70],
        [4.1, -70],
        [4.2, -70],
        [4.3, -70],
        [4.4, -70],
        [4.5, -70],
        [4.6, -70],
        [4.7, -70],
        [4.8, -70],
        [4.9, -70],
        [5.0, -70],
        [
            5.1,
            -69.4229,
        ],
        [5.2, -68.8515],
        [5.3, -68.2858],
        [5.4, -67.7258],
        [5.5, -67.1713],
        [5.6, -66.6223],
        [5.7, -66.0788],
        [5.8, -65.5407],
        [5.9, -65.008],
        [6.0, -64.4806],
        [6.1, -63.9584],
        [6.2, -63.4414],
        [6.3, -62.9295],
        [6.4, -62.4228],
        [6.5, -61.9211],
        [6.6, -61.4243],
        [6.7, -60.9326],
        [6.8, -60.4457],
        [6.9, -59.9636],
    ]
)

expected_mindelay = np.array(
    [
        [1.000000e00, -7.000000e01],
        [2.000000e00, -7.000000e01],
        [3.000000e00, -6.655725e01],
        [4.000000e00, -6.307837e01],
        [5.000000e00, -5.993054e01],
        [6.000000e00, -5.708227e01],
        [7.000000e00, -7.000000e01],
        [8.000000e00, -7.000000e01],
        [9.000000e00, -6.960199e01],
        [1.000000e01, -6.583337e01],
    ]
)
