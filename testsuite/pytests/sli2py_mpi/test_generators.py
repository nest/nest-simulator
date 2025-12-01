# -*- coding: utf-8 -*-
#
# test_generators.py
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


@pytest.mark.skipif_missing_threads
@pytest.mark.skipif_incompatible_mpi
@pytest.mark.parametrize(
    "gen_model, params",
    [
        ["gamma_sup_generator", {"rate": 1000}],
        ["mip_generator", {"rate": 2000, "p_copy": 0.5}],
        ["noise_generator", {"mean": 1000, "std": 500, "dt": 1}],
        ["poisson_generator", {"rate": 1000}],
        ["poisson_generator_ps", {"rate": 1000}],
        ["pp_psc_delta", {"I_e": 1000}],  # not strictly a generator, but behaves like one here
        ["ppd_sup_generator", {"rate": 1000}],
        ["pulsepacket_generator", {"pulse_times": [5], "activity": 5, "sdev": 2}],
        [
            "sinusoidal_gamma_generator",
            {"rate": 1000, "amplitude": 1000, "frequency": 100, "order": 3, "individual_spike_trains": False},
        ],
        [
            "sinusoidal_gamma_generator",
            {"rate": 1000, "amplitude": 1000, "frequency": 100, "order": 3, "individual_spike_trains": True},
        ],
        [
            "sinusoidal_poisson_generator",
            {"rate": 1000, "amplitude": 1000, "frequency": 100, "individual_spike_trains": False},
        ],
        [
            "sinusoidal_poisson_generator",
            {"rate": 1000, "amplitude": 1000, "frequency": 100, "individual_spike_trains": True},
        ],
    ],
)
@MPITestAssertEqual([1, 2, 4])
def test_generators(gen_model, params):
    """
    Creates a generator and sends spikes to spike recorder.
    Assert invariant results for fixed VP number. This is a partial response to ticket #551.
    May be adapted to other generators.

    The test compares data written by spike_recorder to SPIKE_LABEL.
    """

    import nest

    TOTAL_VPS = 4
    NEED_IAF = ["noise_generator"]

    nest.set(total_num_virtual_procs=TOTAL_VPS, overwrite_files=True)

    gen = nest.Create(gen_model, params=params)
    pnet = nest.Create("parrot_neuron" if gen_model not in NEED_IAF else "iaf_psc_alpha", TOTAL_VPS)
    sr = nest.Create(
        "spike_recorder",
        params={
            "label": SPIKE_LABEL.format(nest.num_processes),  # noqa: F821
            "record_to": "ascii",
            "time_in_steps": True,
        },
    )

    nest.Connect(gen, pnet)
    nest.Connect(pnet, sr)

    nest.Simulate(50)
    assert nest.local_spike_counter > 0

    # Uncomment next line to provoke test failure
    # nest.Simulate(20 if nest.num_processes == 1 else 40)
