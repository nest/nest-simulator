# -*- coding: utf-8 -*-
#
# test_ticket_673.py
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
import pytest

"""
Regression test for Ticket #673.

Test ported from SLI regression test.
Ensure precise neuron models handle refractory times equal to the simulation resolution.

Author: Susanne Kunkel, 2013-01-23
"""


PRECISE_MODELS = ("iaf_psc_alpha_ps", "iaf_psc_delta_ps", "iaf_psc_exp_ps")


def _second_spike_time(model: str, t_ref: float) -> float:
    nest.ResetKernel()
    nest.SetKernelStatus({"resolution": 0.1})

    neuron = nest.Create(model, params={"I_e": 1000.0, "t_ref": t_ref})
    spike_recorder = nest.Create("spike_recorder")
    nest.Connect(neuron, spike_recorder)

    nest.Simulate(20.0)

    events = nest.GetStatus(spike_recorder, "events")[0]
    times = events["times"]

    assert len(times) > 1, f"Model {model} did not fire at least twice."

    return times[1]


@pytest.mark.parametrize("model", PRECISE_MODELS)
def test_ticket_673_precise_models_shift_second_spike(model):
    """
    Ensure that reducing the refractory time by 0.9 ms advances the second spike by the same amount.
    """

    spike_time_default = _second_spike_time(model, t_ref=1.0)
    spike_time_shorter = _second_spike_time(model, t_ref=0.1)

    assert spike_time_default - spike_time_shorter == pytest.approx(0.9, abs=1e-12)
