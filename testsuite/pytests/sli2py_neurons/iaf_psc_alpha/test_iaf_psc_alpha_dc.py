# -*- coding: utf-8 -*-
#
# test_iaf_psc_alpha_dc.py
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

import nest
import numpy as np
import pytest
import testsimulation
import testutil


@dataclasses.dataclass
class IAFPSCAlphaDCSimulation(testsimulation.Simulation):
    amplitude: float = 1000.0
    origin: float = 0.0
    arrival: float = 3.0
    dc_delay: float = 1.0
    dc_visible: float = 3.0
    dc_duration: float = 2.0

    def __post_init__(self):
        self.dc_on = self.dc_visible - self.dc_delay
        self.dc_off = self.dc_on + self.dc_duration

    def setup(self):
        super().setup()
        n1 = self.neuron = nest.Create("iaf_psc_alpha")
        dc = self.dc_generator = nest.Create("dc_generator")
        dc.amplitude = self.amplitude
        vm = self.voltmeter = nest.Create("voltmeter")
        vm.interval = self.resolution
        nest.Connect(vm, n1)


@pytest.mark.parametrize("weight", [1.0])
@testutil.use_simulation(IAFPSCAlphaDCSimulation)
class TestIAFPSCAlphaDC:
    @pytest.mark.parametrize("delay", [0.1])
    def test_dc(self, simulation):
        simulation.setup()

        dc_gen_spec = {"delay": simulation.delay, "weight": simulation.weight}
        nest.Connect(simulation.dc_generator, simulation.neuron, syn_spec=dc_gen_spec)

        results = simulation.simulate()

        actual, expected = testutil.get_comparable_timesamples(results, expect_default)
        assert actual == expected

    @pytest.mark.parametrize("resolution,delay", [(0.1, 0.1), (0.2, 0.2), (0.5, 0.5), (1.0, 1.0)])
    def test_dc_aligned(self, simulation):
        simulation.setup()

        simulation.dc_generator.set(
            amplitude=simulation.amplitude,
            origin=simulation.origin,
            start=simulation.arrival - simulation.resolution,
        )
        nest.Connect(
            simulation.dc_generator,
            simulation.neuron,
            syn_spec={"delay": simulation.delay},
        )

        results = simulation.simulate()

        actual, expected = testutil.get_comparable_timesamples(results, expect_aligned)
        assert actual == expected

    @pytest.mark.parametrize("resolution,delay", [(0.1, 0.1), (0.2, 0.2), (0.5, 0.5), (1.0, 1.0)])
    def test_dc_aligned_auto(self, simulation):
        simulation.setup()

        simulation.dc_generator.set(
            amplitude=simulation.amplitude,
            origin=simulation.origin,
            start=simulation.dc_on,
        )
        dc_gen_spec = {"delay": simulation.dc_delay, "weight": simulation.weight}
        nest.Connect(simulation.dc_generator, simulation.neuron, syn_spec=dc_gen_spec)

        results = simulation.simulate()
        actual, expected = testutil.get_comparable_timesamples(results, expect_aligned)
        assert actual == expected

    @pytest.mark.parametrize("resolution,delay", [(0.1, 0.1), (0.2, 0.2), (0.5, 0.5), (1.0, 1.0)])
    @pytest.mark.parametrize("duration", [10.0])
    def test_dc_aligned_stop(self, simulation):
        simulation.setup()

        simulation.dc_generator.set(
            amplitude=simulation.amplitude,
            origin=simulation.origin,
            start=simulation.dc_on,
            stop=simulation.dc_off,
        )
        dc_gen_spec = {"delay": simulation.dc_delay, "weight": simulation.weight}
        nest.Connect(simulation.dc_generator, simulation.neuron, syn_spec=dc_gen_spec)

        results = simulation.simulate()
        actual, expected = testutil.get_comparable_timesamples(results, expect_stop)
        assert actual == expected


expect_default = np.array(
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
        [1.1, -66.5572],
        [1.2, -66.1935],
        [1.3, -65.8334],
        [1.4, -65.4768],
        [1.5, -65.1238],
        [1.6, -64.7743],
    ]
)


expect_aligned = np.array(
    [
        [2.5, -70],
        [2.6, -70],
        [2.7, -70],
        [2.8, -70],
        [2.9, -70],
        [3.0, -70],
        [3.1, -69.602],
        [3.2, -69.2079],
        [3.3, -68.8178],
        [3.4, -68.4316],
        [3.5, -68.0492],
        [3.6, -67.6706],
        [3.7, -67.2958],
        [3.8, -66.9247],
        [3.9, -66.5572],
        [4.0, -66.1935],
        [4.1, -65.8334],
        [4.2, -65.4768],
    ]
)


expect_stop = np.array(
    [
        [2.5, -70],
        [2.6, -70],
        [2.7, -70],
        [2.8, -70],
        [2.9, -70],
        [3.0, -70],
        [3.1, -69.602],
        [3.2, -69.2079],
        [3.3, -68.8178],
        [3.4, -68.4316],
        [3.5, -68.0492],
        [3.6, -67.6706],
        [3.7, -67.2958],
        [3.8, -66.9247],
        [3.9, -66.5572],
        [4.0, -66.1935],
        [4.1, -65.8334],
        [4.2, -65.4768],
        [4.3, -65.1238],
        [4.4, -64.7743],
        [4.5, -64.4283],
        [4.6, -64.0858],
        [4.7, -63.7466],
        [4.8, -63.4108],
        [4.9, -63.0784],
        [5.0, -62.7492],
        [5.1, -62.8214],
        [5.2, -62.8928],
        [5.3, -62.9635],
        [5.4, -63.0335],
        [5.5, -63.1029],
        [5.6, -63.1715],
        [5.7, -63.2394],
        [5.8, -63.3067],
        [5.9, -63.3733],
        [6.0, -63.4392],
    ]
)
