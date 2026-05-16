# -*- coding: utf-8 -*-
#
# test_rate_connections.py
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
Test that that rate connections work properly.

This set of tests ensures that legal rate connections (``rate_connection_instantaneous``,
``rate_connection_delayed``, ``diffusion_connection``) can be created and that an
exception is thrown if one tries to create illegal rate connections.

Furthermore it is checked that the delay cannot be set for ``rate_connection_instantaneous``
connections and that delay and weight cannot be set for ``diffusion_connection`` connections.
"""

import nest
import pytest

# The following tests need the siegert_neuron model, so they should
# only run if we have GSL
pytestmark = pytest.mark.skipif_missing_gsl


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


@pytest.mark.parametrize("synapse_model", ["rate_connection_instantaneous", "rate_connection_delayed"])
@pytest.mark.parametrize(
    "supported_nrn_model",
    [
        "lin_rate_ipn",
        "lin_rate_opn",
        "threshold_lin_rate_ipn",
        "threshold_lin_rate_opn",
        "tanh_rate_ipn",
        "tanh_rate_opn",
    ],
)
def test_legal_rate_connections(synapse_model, supported_nrn_model):
    """
    Test that legal rate connections can be created.

    This test ensures that ``rate_connection_instantaneous`` and
    ``rate_connection_delayed`` connections can be established for supported
    neuron models.
    """

    supported_nrn = nest.Create(supported_nrn_model)
    nest.Connect(
        supported_nrn, supported_nrn, conn_spec={"rule": "one_to_one"}, syn_spec={"synapse_model": synapse_model}
    )


@pytest.mark.parametrize("synapse_model", ["rate_connection_instantaneous", "rate_connection_delayed"])
@pytest.mark.parametrize(
    "supported_nrn_model",
    [
        "lin_rate_ipn",
        "lin_rate_opn",
        "threshold_lin_rate_ipn",
        "threshold_lin_rate_opn",
        "tanh_rate_ipn",
        "tanh_rate_opn",
    ],
)
@pytest.mark.parametrize("unsupported_nrn_model", ["hh_psc_alpha_gap", "siegert_neuron", "iaf_psc_alpha"])
def test_illegal_rate_connections(synapse_model, supported_nrn_model, unsupported_nrn_model):
    """
    Test that illegal rate connections raise an exception.

    This test ensures that ``rate_connection_instantaneous`` and
    ``rate_connection_delayed`` connections between supported and unsupported
    neuron models raise an exception.
    """

    supported_nrn = nest.Create(supported_nrn_model)
    unsupported_nrn = nest.Create(unsupported_nrn_model)
    with pytest.raises(nest.NESTErrors.IllegalConnection):
        nest.Connect(
            supported_nrn, unsupported_nrn, conn_spec={"rule": "one_to_one"}, syn_spec={"synapse_model": synapse_model}
        )


@pytest.mark.parametrize(
    "supported_nrn_model",
    [
        "lin_rate_ipn",
        "lin_rate_opn",
        "threshold_lin_rate_ipn",
        "threshold_lin_rate_opn",
        "tanh_rate_ipn",
        "tanh_rate_opn",
    ],
)
def test_rate_connection_instantaneous_set_delay_disallowed(supported_nrn_model):
    """
    Test that delay cannot be set for ``rate_connection_instantaneous`` connections.
    """

    supported_nrn = nest.Create(supported_nrn_model)

    with pytest.raises(nest.NESTErrors.BadProperty):
        nest.Connect(
            supported_nrn,
            supported_nrn,
            conn_spec={"rule": "all_to_all"},
            syn_spec={"synapse_model": "rate_connection_instantaneous", "delay": 2.0},
        )


def test_legal_diffusion_connection_connections():
    """
    Test that legal ``diffusion_connection`` connections can be created.
    """

    supported_nrn = nest.Create("siegert_neuron")
    nest.Connect(
        supported_nrn,
        supported_nrn,
        conn_spec={"rule": "one_to_one"},
        syn_spec={"synapse_model": "diffusion_connection"},
    )


@pytest.mark.parametrize(
    "unsupported_nrn_model",
    [
        "hh_psc_alpha_gap",
        "iaf_psc_alpha",
        "lin_rate_ipn",
        "lin_rate_opn",
        "threshold_lin_rate_ipn",
        "threshold_lin_rate_opn",
        "tanh_rate_ipn",
        "tanh_rate_opn",
    ],
)
def test_illegal_diffusion_connection_connections(unsupported_nrn_model):
    """
    Test that illegal ``diffusion_connection`` connections raise an exception.
    """

    supported_nrn = nest.Create("siegert_neuron")
    unsupported_nrn = nest.Create(unsupported_nrn_model)

    with pytest.raises(nest.NESTErrors.IllegalConnection):
        nest.Connect(
            supported_nrn,
            unsupported_nrn,
            conn_spec={"rule": "one_to_one"},
            syn_spec={"synapse_model": "diffusion_connection"},
        )


@pytest.mark.parametrize("syn_param", ["weight", "delay"])
def test_diffusion_connection_set_weight_and_delay_disallowed(syn_param):
    """
    Test that weight and delay cannot be set for ``diffusion_connection`` connections.
    """

    supported_nrn = nest.Create("siegert_neuron")

    with pytest.raises(nest.NESTErrors.BadProperty):
        nest.Connect(
            supported_nrn,
            supported_nrn,
            conn_spec={"rule": "all_to_all"},
            syn_spec={"synapse_model": "diffusion_connection", syn_param: 2.0},
        )


def test_diffusion_connection_set_diffusion_factor_and_drift_factor_allowed():
    """
    Test that diffusion and drift factor can be set for ``diffusion_connection`` connections.
    """

    supported_nrn = nest.Create("siegert_neuron")
    nest.Connect(
        supported_nrn,
        supported_nrn,
        conn_spec={"rule": "all_to_all"},
        syn_spec={"synapse_model": "diffusion_connection", "diffusion_factor": 2.0, "drift_factor": 2.0},
    )
