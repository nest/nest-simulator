# -*- coding: utf-8 -*-
#
# test_axonal_delay_user_interface.py
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


def test_total_delay_user_interface_connect_success():
    """
    Make sure that one can only set the total delay and not just the axonal or dendritic delay for synapses which only
    support a total transmission delay in the Connect call.
    """
    nest.ResetKernel()

    neuron = nest.Create("iaf_psc_alpha")

    conn = nest.Connect(
        neuron, neuron, syn_spec={"synapse_model": "stdp_pl_synapse_hom", "delay": 1.0}, return_synapsecollection=True
    )
    assert nest.GetStatus(conn)[0]["delay"] == 1.0


def test_total_delay_user_interface_set_status_success():
    """
    Make sure that one can only set the total delay and not just the axonal or dendritic delay for synapses which only
    support a total transmission delay when using SetStatus.
    """
    nest.ResetKernel()

    neuron = nest.Create("iaf_psc_alpha")

    conn = nest.Connect(
        neuron, neuron, syn_spec={"synapse_model": "stdp_pl_synapse_hom"}, return_synapsecollection=True
    )
    nest.SetStatus(conn, {"delay": 1.0})
    assert nest.GetStatus(conn)[0]["delay"] == 1.0


def test_total_delay_user_interface_set_defaults_success():
    """
    Make sure that one can only set the total delay and not just the axonal or dendritic delay for synapses which only
    support a total transmission delay when using SetDefaults.
    """
    nest.ResetKernel()

    nest.SetDefaults("stdp_pl_synapse_hom", {"delay": 1.0})
    assert nest.GetDefaults("stdp_pl_synapse_hom")["delay"] == 1.0


@pytest.mark.parametrize("ax_delay, dend_delay", [[1.0, 0.0], [0.0, 1.0], [1.0, 1.0]])
def test_split_delay_user_interface_connect_success(ax_delay, dend_delay):
    """
    Make sure that one can only set the axonal or dendritic delay for synapses which support split axonal and dendritic
    delays and setting the total transmission delay via the "delay" parameter name must fail because of ambiguity.
    """
    nest.ResetKernel()

    neuron = nest.Create("iaf_psc_alpha")

    conn = nest.Connect(
        neuron,
        neuron,
        syn_spec={"synapse_model": "stdp_pl_synapse_hom_ax_delay", "axonal_delay": 1.0},
        return_synapsecollection=True,
    )
    assert nest.GetStatus(conn)[0]["axonal_delay"] == 1.0
    conn = nest.Connect(
        neuron,
        neuron,
        syn_spec={"synapse_model": "stdp_pl_synapse_hom_ax_delay", "dendritic_delay": 1.0},
        return_synapsecollection=True,
    )
    assert nest.GetStatus(conn)[0]["dendritic_delay"] == 1.0


@pytest.mark.parametrize("ax_delay, dend_delay", [[1.0, 0.0], [0.0, 1.0], [1.0, 1.0]])
def test_split_delay_user_interface_set_status_success(ax_delay, dend_delay):
    """
    Make sure that one can only set the axonal or dendritic delay for synapses which support split axonal and dendritic
    delays and setting the total transmission delay via the "delay" parameter name must fail because of ambiguity.
    """
    nest.ResetKernel()

    neuron = nest.Create("iaf_psc_alpha")

    conn = nest.Connect(
        neuron, neuron, syn_spec={"synapse_model": "stdp_pl_synapse_hom_ax_delay"}, return_synapsecollection=True
    )
    nest.SetStatus(conn, {"axonal_delay": 1.0})
    nest.SetStatus(conn, {"dendritic_delay": 1.0})
    assert nest.GetStatus(conn)[0]["axonal_delay"] == 1.0
    assert nest.GetStatus(conn)[0]["dendritic_delay"] == 1.0


@pytest.mark.parametrize("ax_delay, dend_delay", [[1.0, 0.0], [0.0, 1.0], [1.0, 1.0]])
def test_split_delay_user_interface_set_defaults_success(ax_delay, dend_delay):
    """
    Make sure that one can set the axonal or dendritic delay for synapses which support split axonal and dendritic
    delays and setting the total transmission delay via the "delay" parameter name must fail because of ambiguity.
    """
    nest.ResetKernel()

    nest.SetDefaults("stdp_pl_synapse_hom_ax_delay", {"axonal_delay": 1.0})
    nest.SetDefaults("stdp_pl_synapse_hom_ax_delay", {"dendritic_delay": 1.0})
    assert nest.GetDefaults("stdp_pl_synapse_hom_ax_delay")["axonal_delay"] == 1.0
    assert nest.GetDefaults("stdp_pl_synapse_hom_ax_delay")["dendritic_delay"] == 1.0


def test_total_delay_user_interface_connect_failure():
    """
    Make sure that one can only set the total delay and not just the axonal or dendritic delay for synapses which only
    support a total transmission delay in the Connect call.
    """
    nest.ResetKernel()

    neuron = nest.Create("iaf_psc_alpha")

    with pytest.raises(nest.kernel.NESTError):
        nest.Connect(neuron, neuron, syn_spec={"synapse_model": "stdp_pl_synapse_hom", "axonal_delay": 1.0})
    with pytest.raises(nest.kernel.NESTError):
        nest.Connect(neuron, neuron, syn_spec={"synapse_model": "stdp_pl_synapse_hom", "dendritic_delay": 1.0})


def test_total_delay_user_interface_set_status_failure():
    """
    Make sure that one can only set the total delay and not just the axonal or dendritic delay for synapses which only
    support a total transmission delay when using SetStatus.
    """
    nest.ResetKernel()

    neuron = nest.Create("iaf_psc_alpha")

    conn = nest.Connect(
        neuron, neuron, syn_spec={"synapse_model": "stdp_pl_synapse_hom"}, return_synapsecollection=True
    )
    with pytest.raises(nest.kernel.NESTError):
        nest.SetStatus(conn, {"dendritic_delay": 1.0})
    with pytest.raises(nest.kernel.NESTError):
        nest.SetStatus(conn, {"axonal_delay": 1.0})


def test_total_delay_user_interface_set_defaults_failure():
    """
    Make sure that one can only set the total delay and not just the axonal or dendritic delay for synapses which only
    support a total transmission delay when using SetDefaults.
    """
    nest.ResetKernel()

    with pytest.raises(nest.kernel.NESTError):
        nest.SetDefaults("stdp_pl_synapse_hom", {"axonal_delay": 1.0})
    with pytest.raises(nest.kernel.NESTError):
        nest.SetDefaults("stdp_pl_synapse_hom", {"dendritic_delay": 1.0})


def test_split_delay_user_interface_connect_failure():
    """
    Make sure that setting the total transmission delay via the "delay" parameter name in Connect fails because of
    ambiguity.
    """
    nest.ResetKernel()

    neuron = nest.Create("iaf_psc_alpha")

    with pytest.raises(nest.kernel.NESTError):
        nest.Connect(neuron, neuron, syn_spec={"synapse_model": "stdp_pl_synapse_hom_ax_delay", "delay": 1.0})


def test_split_delay_user_interface_set_status_failure():
    """
    Make sure that setting the total transmission delay via the "delay" parameter name in SetStatus fails because of
    ambiguity.
    """
    nest.ResetKernel()

    neuron = nest.Create("iaf_psc_alpha")

    conn = nest.Connect(
        neuron, neuron, syn_spec={"synapse_model": "stdp_pl_synapse_hom_ax_delay"}, return_synapsecollection=True
    )
    with pytest.raises(nest.kernel.NESTError):
        nest.SetStatus(conn, {"delay": 1.0})


def test_split_delay_user_interface_set_defaults_failure():
    """
    Make sure that setting the total transmission delay via the "delay" parameter name in SetDefaults fails because of
    ambiguity.
    """
    nest.ResetKernel()

    with pytest.raises(nest.kernel.NESTError):
        nest.SetDefaults("stdp_pl_synapse_hom_ax_delay", {"delay": 1.0})


def test_split_delay_user_interface_connect_zero_delay_failure():
    """
    Make sure that either the dendritic or axonal delay is non-zero.
    """
    nest.ResetKernel()

    neuron = nest.Create("iaf_psc_alpha")

    with pytest.raises(nest.kernel.NESTError):
        nest.Connect(
            neuron,
            neuron,
            syn_spec={"synapse_model": "stdp_pl_synapse_hom_ax_delay", "dendritic_delay": 0.0, "axonal_delay": 0.0},
        )


def test_split_delay_user_interface_set_status_zero_delay_failure():
    """
    Make sure that either the dendritic or axonal delay is non-zero.
    """
    nest.ResetKernel()

    neuron = nest.Create("iaf_psc_alpha")

    conn = nest.Connect(
        neuron, neuron, syn_spec={"synapse_model": "stdp_pl_synapse_hom_ax_delay"}, return_synapsecollection=True
    )
    with pytest.raises(nest.kernel.NESTError):
        nest.SetStatus(conn, {"dendritic_delay": 0.0, "axonal_delay": 0.0})


def test_split_delay_user_interface_set_defaults_zero_delay_failure():
    """
    Make sure that either the dendritic or axonal delay is non-zero.
    """
    nest.ResetKernel()

    with pytest.raises(nest.kernel.NESTError):
        nest.SetDefaults("stdp_pl_synapse_hom_ax_delay", {"dendritic_delay": 0.0, "axonal_delay": 0.0})
