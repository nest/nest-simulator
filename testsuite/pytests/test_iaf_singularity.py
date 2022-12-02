# -*- coding: utf-8 -*-
#
# test_iaf_singularity.py
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
from nest.ll_api import sr

"""
Test for correct handling of IAF neuron propagator singularity.
"""

import nest
import pytest
import numpy as np
import pandas as pd


@nest.ll_api.check_stack
class TestIAFSingularity:
    """
    Test that iaf neurons handle singularities for ``tau_syn = tau_m`` correctly.

    A single spike is injected into a neuron. Under all conditions, the resulting
    membrane potential trace must be smooth.
    """

    @pytest.mark.parametrize("model", ["iaf_psc_exp", "iaf_psc_alpha"])
    @pytest.mark.parametrize("h", [0.001, 0.1])
    @pytest.mark.parametrize("tau_m", [1, 10, 100])
    def test_smooth_response(self, model, h, tau_m):
        """
        Drive single neuron with single spike through excitatory and inhibitory synapse.
        Confirm that ``V_m`` is smooth with single maximum.

        For the sake of efficiency, we actually create multiple neurons with different
        differences between ``tau_m`` and ``tau_syn``.
        """

        delta_tau = np.hstack((-np.logspace(-10, -1, 5), [0], np.logspace(-10, -1, 5)))
        tau_syn = tau_m + delta_tau

        nest.ResetKernel()
        nest.resolution = h

        neurons = nest.Create(model, n=len(tau_syn),
                              params={"tau_m": tau_m,
                                      "tau_syn_ex": tau_syn,
                                      "tau_syn_in": tau_syn,
                                      "V_th": np.inf})

        spike_gen = nest.Create('spike_generator', params={'spike_times': [1.]})
        vm = nest.Create('voltmeter', params={'interval': h})

        nest.Connect(spike_gen, neurons, syn_spec={'weight':  1000., 'delay': 1.0})
        nest.Connect(spike_gen, neurons, syn_spec={'weight': -1000., 'delay': 2.0})
        nest.Connect(vm, neurons)

        nest.Simulate(10 * tau_m)

        d = pd.DataFrame.from_records(vm.events).set_index(['times', 'senders'])
        assert not any(d.V_m.isnull()), "Voltmeter returned NaN"

        V_range = d.V_m.max() - d.V_m.min()
        assert V_range > 1, "Too small changed in V_m for valid test"

        # Drop top row from diff since it is NaN by construction
        dV_by_sender = d.unstack().diff(axis=0).iloc[1:]

        # Finally the real test, factor 3 determine by visual checks
        assert all(np.abs(dV_by_sender.V_m.max()) < 3 * V_range * h)
