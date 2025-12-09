# -*- coding: utf-8 -*-
#
# test_gap_junction.py
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
Ensures that NEST throws an exception if one tries to create illegal gap-junction connections.
Furthermore it is checked that the delay cannot be set for gap-junction connections.
"""

import nest
import pytest


@pytest.fixture(autouse=True)
def setup():
    """
    Reset kernel to clear reset delay and resolution parameters. Setup base network.
    """

    nest.ResetKernel()

    pytest.wfr_comm = 2.0
    nest.SetKernelStatus({"wfr_comm_interval": pytest.wfr_comm})

    pytest.neuron_no_gap = nest.Create("iaf_psc_alpha")
    pytest.neuron_gap = nest.Create("hh_psc_alpha_gap")


@pytest.mark.parametrize(
    "syn_spec", [{"synapse_model": "gap_junction"}, {"synapse_model": "gap_junction", "weight": 2.0}]
)
def test_neuron_gap_connect_one_to_one(syn_spec):
    """
    Test if gap junction neurons can be connected by a gap junction synapse
    with a symmetric one_to_one connection rule.
    """
    conn_spec = {"rule": "one_to_one", "make_symmetric": True}
    nest.Connect(pytest.neuron_gap, pytest.neuron_gap, conn_spec=conn_spec, syn_spec=syn_spec)


def test_neuron_gap_connect_all_to_all():
    """
    Test if gap junction neurons can be connected by a gap junction synapse
    with an all_to_all connection rule.
    """
    conn_spec = {"rule": "all_to_all"}
    syn_spec = {"synapse_model": "gap_junction"}
    nest.Connect(pytest.neuron_gap, pytest.neuron_gap, conn_spec=conn_spec, syn_spec=syn_spec)


@pytest.mark.parametrize("conn_spec", [{"rule": "one_to_one", "make_symmetric": True}, {"rule": "all_to_all"}])
def test_neuron_gap_connect_with_delay_fails(conn_spec):
    """
    Test if setting a delay when connecting gap junction neurons via a gap junction synapse
    leads to an error.
    """
    syn_spec = {"synapse_model": "gap_junction", "delay": 2.0}
    with pytest.raises(nest.kernel.NESTError, match="Delay specified for a connection type which doesn't use delays."):
        nest.Connect(pytest.neuron_gap, pytest.neuron_gap, conn_spec=conn_spec, syn_spec=syn_spec)


def test_neuron_nogap_nogap_connect_fails():
    """
    Test if connecting no-gap junction neurons via a gap junction synapse leads to an error.
    """
    conn_spec = {"rule": "one_to_one", "make_symmetric": True}
    syn_spec = {"synapse_model": "gap_junction"}
    with pytest.raises(nest.kernel.NESTError, match="The source node does not support gap junction output."):
        nest.Connect(pytest.neuron_no_gap, pytest.neuron_no_gap, conn_spec=conn_spec, syn_spec=syn_spec)


def test_neuron_nogap_gap_connect_fails():
    """
    Test if connecting a no-gap junction neuron to a gap junction neuron via
    a gap junction synapse leads to an error.
    """
    conn_spec = {"rule": "one_to_one", "make_symmetric": True}
    syn_spec = {"synapse_model": "gap_junction"}
    with pytest.raises(
        nest.kernel.NESTError, match="The target node or synapse model does not support gap junction input."
    ):
        nest.Connect(pytest.neuron_no_gap, pytest.neuron_gap, conn_spec=conn_spec, syn_spec=syn_spec)


def test_neuron_gap_connect_not_symmetric_fails():
    """
    Test if connecting gap junction neurons via a gap junction synapse via
    a nonsymmetric connection rule leads to an error.
    """
    conn_spec = {"rule": "one_to_one", "make_symmetric": False}
    syn_spec = {"synapse_model": "gap_junction", "weight": 2.0}
    with pytest.raises(
        nest.kernel.NESTError,
        match="Connections with this synapse model can only be created as "
        + 'one-to-one connections with "make_symmetric" set to true *',
    ):
        nest.Connect(pytest.neuron_gap, pytest.neuron_gap, conn_spec=conn_spec, syn_spec=syn_spec)


@pytest.mark.parametrize("delay", [3.0, 1.0])
def test_min_delay(delay):
    """
    Test if gap junction connections contribute to the delay extrema via wfr_comm_interval.
    """
    nest.Connect(pytest.neuron_no_gap, pytest.neuron_no_gap, syn_spec={"weight": 3.0, "delay": delay})

    old_min, old_max = nest.GetKernelStatus(["min_delay", "max_delay"])

    conn_spec = {"rule": "one_to_one", "make_symmetric": True}
    syn_spec = {"synapse_model": "gap_junction"}
    nest.Connect(pytest.neuron_gap, pytest.neuron_gap, conn_spec=conn_spec, syn_spec=syn_spec)

    new_min, new_max = nest.GetKernelStatus(["min_delay", "max_delay"])

    if pytest.wfr_comm > old_min:
        assert new_max == pytest.wfr_comm and new_min == old_min
    elif pytest.wfr_comm < old_min:
        assert new_min == pytest.wfr_comm and new_max == old_max
