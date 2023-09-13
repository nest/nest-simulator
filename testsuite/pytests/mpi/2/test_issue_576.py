# -*- coding: utf-8 -*-
#
# test_issue_576.py
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


import unittest

import nest

HAVE_GSL = nest.build_info["have_gsl"]


@unittest.skipIf(nest.NumProcesses() < 2, "Requires >= 2 MPI process")
@unittest.skipIf(not HAVE_GSL, "GSL is not available")
def test_simulation_completes():
    """
    Ensure that simulation of structural plasticity completes with MPI.

    This is a regression test for issue #576 concerning crashing simulations
    when running MPI-parallel.
    """

    nest.ResetKernel()
    nest.resolution = 0.1

    nest.EnableStructuralPlasticity()
    nest.structural_plasticity_update_interval = 1000.0

    growth_curve = {
        "growth_curve": "gaussian",
        "growth_rate": 0.0001,  # Beta (elements/ms)
        "continuous": False,
        "eta": 0.1,
        "eps": 0.7,
    }
    structural_p_elements_E = {"Den_ex": growth_curve, "Den_in": growth_curve, "Axon_ex": growth_curve}
    neuronDict = {
        "V_m": -60.0,
        "t_ref": 5.0,
        "V_reset": -60.0,
        "V_th": -50.0,
        "C_m": 200.0,
        "E_L": -60.0,
        "g_L": 10.0,
        "E_ex": 0.0,
        "E_in": -80.0,
        "tau_syn_ex": 5.0,
        "tau_syn_in": 10.0,
        "I_e": 220.0,
    }

    nest.SetDefaults("iaf_cond_exp", neuronDict)
    neuronsE = nest.Create("iaf_cond_exp", 1, {"synaptic_elements": structural_p_elements_E})  # noqa: F841

    synDictE = {
        "synapse_model": "static_synapse",
        "weight": 3.0,
        "pre_synaptic_element": "Axon_ex",
        "post_synaptic_element": "Den_ex",
    }
    nest.structural_plasticity_synapses = {"synapseEE": synDictE}

    nest.Simulate(200 * 1000)
