# -*- coding: utf-8 -*-
#
# test_ticket_310.py
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
0  # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with NEST.  If not, see <http://www.gnu.org/licenses/>.

import nest
import pytest

#  Models to skip to do given reasons
skip_list = [
    "correlospinmatrix_detector",  # not a neuron
    "eprop_iaf_bsshslm_2020",  # no ArchivingNode, thus no t_spike
    "eprop_iaf_adapt_bsshslm_2020",  # no ArchivingNode, thus no t_spike
    "eprop_readout_bsshslm_2020",  # no ArchivingNode, thus no t_spike
    "eprop_iaf",  # no ArchivingNode, thus no t_spike
    "eprop_iaf_adapt",  # no ArchivingNode, thus no t_spike
    "eprop_iaf_psc_delta",  # no ArchivingNode, thus no t_spike
    "eprop_iaf_psc_delta_adapt",  # no ArchivingNode, thus no t_spike
    "eprop_readout",  # no ArchivingNode, thus no t_spike
    "iaf_chxk_2008",  # non-standard spiking conditions
    "izhikevich",  # generating output spike not reliably suppressed even for subthreshold V_m
]


def has_Vm_and_Vth(model):
    params = nest.GetDefaults(model)
    return "V_m" in params and "V_th" in params


relevant_models = [model for model in nest.node_models if model not in skip_list and has_Vm_and_Vth(model)]


@pytest.mark.parametrize("model", relevant_models)
def test_ticket_310(model):
    """
    Regression test for Ticket #310.

    Ensure that all neuron models that have V_m and V_th permit
    V_m to be set to >= V_th, and that they emit a spike with
    time stamp == resolution in that case.

    Author: Hans Ekkehard Plesser, 2009-02-11
    """

    time_resolution = 2**-3  # Use power-of-two resolution to avoid round-off problems

    nest.ResetKernel()
    nest.resolution = time_resolution

    nrn = nest.Create(model)
    nrn.V_m = nrn.V_th + 15

    nest.Simulate(time_resolution)

    assert 0 < nrn.t_spike <= time_resolution  # for precise neurons, < is possible; -1 if never spiked
