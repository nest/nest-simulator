# -*- coding: utf-8 -*-
#
# test_sic_connection.py
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
Test functionality of the SIC connection
"""

import nest
import numpy as np
import pytest

pytestmark = pytest.mark.skipif_missing_gsl


SUPPORTED_SOURCES = [
    "astrocyte_lr_1994",
]
SUPPORTED_TARGETS = [
    "aeif_cond_alpha_astro",
]
TEST_MODELS = SUPPORTED_SOURCES + SUPPORTED_TARGETS + ["iaf_psc_exp"]


@pytest.mark.parametrize("source_model", TEST_MODELS)
@pytest.mark.parametrize("target_model", TEST_MODELS)
def test_ConnectNeuronsWithSICConnection(source_model, target_model):
    """Ensures that the restriction to supported neuron models works."""

    source = nest.Create(source_model)
    target = nest.Create(target_model)

    if source_model in SUPPORTED_SOURCES and target_model in SUPPORTED_TARGETS:
        # Connection should work
        nest.Connect(source, target, syn_spec={"synapse_model": "sic_connection"})
    else:
        # Connection should fail
        with pytest.raises(nest.kernel.NESTError):
            nest.Connect(source, target, syn_spec={"synapse_model": "sic_connection"})


def test_SynapseFunctionWithAeifModel():
    """Ensure that SICEvent is properly processed"""

    nest.ResetKernel()
    resol = nest.resolution

    # Create neurons and devices
    astrocyte = nest.Create("astrocyte_lr_1994", {"Ca_astro": 0.2})  # a calcium value which produces SIC
    neuron = nest.Create("aeif_cond_alpha_astro")

    mm_neuron = nest.Create("multimeter", params={"record_from": ["I_SIC"], "interval": resol})
    mm_astro = nest.Create("multimeter", params={"record_from": ["Ca_astro"], "interval": resol})

    nest.Connect(astrocyte, neuron, syn_spec={"synapse_model": "sic_connection"})
    nest.Connect(mm_neuron, neuron)
    nest.Connect(mm_astro, astrocyte)

    # Simulation
    nest.Simulate(1000.0)

    # Evaluation
    # The expected SIC values are calculated based on the astrocyte dynamics
    # implemented in astrocyte_lr_1994.cpp.
    actual_sic_values = mm_neuron.events["I_SIC"]
    Ca = mm_astro.events["Ca_astro"]
    f_v = np.vectorize(lambda x: np.log(x * 1000.0 - 196.69) if x * 1000.0 - 196.69 > 1.0 else 0.0)
    expected_sic_values = f_v(Ca)

    # The sic_connection has a default delay (1 ms), thus the values after
    # the number of steps of delay are compared with the expected values.
    sic_delay = nest.GetDefaults("sic_connection")["delay"]
    n_step_delay = int(sic_delay / resol)

    assert actual_sic_values[n_step_delay:] == pytest.approx(expected_sic_values[:-n_step_delay])
