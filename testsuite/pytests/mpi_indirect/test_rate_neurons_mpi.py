# -*- coding: utf-8 -*-
#
# test_rate_neurons_mpi.py
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

import pytest
from mpi_test_wrapper import MPITestAssertEqual


@pytest.mark.skipif_incompatible_mpi
@pytest.mark.skipif_missing_threads
@pytest.mark.skipif_missing_gsl
@pytest.mark.parametrize(
    "neuron_model, params1, params2, synspec",
    [
        [
            "lin_rate_ipn",
            {"mu": 0, "sigma": 0, "rate": 20},
            {"mu": 0, "sigma": 0},
            {"synapse_model": "rate_connection_instantaneous", "weight": 5},
        ],
        [
            "siegert_neuron",
            {"rate": 20},
            {},
            {"synapse_model": "diffusion_connection", "diffusion_factor": 2, "drift_factor": 4},
        ],
    ],
)
@MPITestAssertEqual([1, 2], debug=False)
def test_rate_neurons_mpi(neuron_model, params1, params2, synspec):
    """
    Test that rate neurons are simulated correctly in parallel.

    The test is performed on the multimeter data recorded to MULTI_LABEL during the simulation.
    """

    import nest

    total_vps = 4
    h = 0.1

    nest.SetKernelStatus(
        {
            "total_num_virtual_procs": total_vps,
            "resolution": h,
            "use_wfr": True,
            "wfr_tol": 0.0001,
            "wfr_interpolation_order": 3,
            "wfr_max_iterations": 10,
            "wfr_comm_interval": 1.0,
        }
    )

    neuron1 = nest.Create(neuron_model, params=params1)
    neuron2 = nest.Create(neuron_model, params=params2)
    mm = nest.Create(
        "multimeter",
        params={
            "record_from": ["rate"],
            "interval": 1,
            "record_to": "ascii",
            "precision": 8,
            "time_in_steps": True,
            "label": MULTI_LABEL.format(nest.num_processes),  # noqa: F821
        },
    )

    nest.Connect(mm, neuron1 + neuron2)

    nest.Connect(neuron1, neuron2, syn_spec=synspec)

    nest.Simulate(11)
