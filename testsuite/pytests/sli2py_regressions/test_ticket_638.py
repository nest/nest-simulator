# -*- coding: utf-8 -*-
#
# test_ticket_638.py
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
Regression test for Ticket #638.

Test ported from SLI regression test.
Ensure precise neuron models spike with default refractory time and reject zero refractory periods.

Author: Hans Ekkehard Plesser, 2012-12-13
"""


PRECISE_MODELS = ("iaf_psc_alpha_ps", "iaf_psc_delta_ps", "iaf_psc_exp_ps")


def _run_simulation(model: str, t_ref: float) -> int:
    nest.ResetKernel()

    neuron = nest.Create(model, params={"I_e": 1000.0, "t_ref": t_ref})
    spike_recorder = nest.Create("spike_recorder")
    nest.Connect(neuron, spike_recorder)
    nest.Simulate(100.0)

    n_events = spike_recorder.get("n_events")
    return n_events


def test_ticket_638_precise_models_spike_with_default_refractory():
    """
    Ensure precise neuron models generate multiple spikes with default refractory periods.
    """

    spike_counts = [_run_simulation(model, t_ref=2.0) for model in PRECISE_MODELS]

    assert all(count > 1 for count in spike_counts)


@pytest.mark.parametrize("model", PRECISE_MODELS)
def test_ticket_638_precise_models_reject_zero_refractory(model):
    """
    Ensure precise neuron models reject zero refractory time.
    """

    nest.ResetKernel()

    with pytest.raises(nest.NESTErrors.BadProperty, match="Refractory time must be at least one time step"):
        nest.Create(model, params={"t_ref": 0.0})
