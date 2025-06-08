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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with NEST.  If not, see <http://www.gnu.org/licenses/>.

import nest
import pytest


def test_ticket_310():
    """
    Regression test for Ticket #310.

    Ensure that all neuron models that have V_m and V_th permit
    V_m to be set to >= V_th, and that they emit a spike with
    time stamp == resolution in that case.

    Author: Hans Ekkehard Plesser, 2009-02-11
    """

    # Use power-of-two resolution to avoid round-off problems
    res = 2**-3

    skip_list = [
        "iaf_chxk_2008",  # non-standard spiking conditions
        "correlospinmatrix_detector",  # not a neuron
        "eprop_iaf_bsshslm_2020",  # no ArchivingNode, thus no t_spike
        "eprop_iaf_adapt_bsshslm_2020",  # no ArchivingNode, thus no t_spike
        "eprop_readout_bsshslm_2020",  # no ArchivingNode, thus no t_spike
        "eprop_iaf",  # no ArchivingNode, thus no t_spike
        "eprop_iaf_adapt",  # no ArchivingNode, thus no t_spike
        "eprop_iaf_psc_delta",  # no ArchivingNode, thus no t_spike
        "eprop_iaf_psc_delta_adapt",  # no ArchivingNode, thus no t_spike
        "eprop_readout",  # no ArchivingNode, thus no t_spike
    ]

    node_models = nest.node_models

    results = []

    for model in node_models:
        if model not in skip_list:
            nest.ResetKernel()
            nest.SetKernelStatus({"resolution": res})
            n = nest.Create(model)

            # Check if V_m and V_th exist in the model's status
            status = nest.GetStatus(n)[0]
            if "V_m" in status and "V_th" in status:
                nest.SetStatus(n, {"V_m": status["V_th"] + 15.0})
                nest.Simulate(res)
                t_spike = nest.GetStatus(n, "t_spike")[0]
                results.append(t_spike <= res)
            else:
                results.append(True)
        else:
            results.append(True)

    # Check if all entries are true
    assert all(results), "Test failed for one or more models"
