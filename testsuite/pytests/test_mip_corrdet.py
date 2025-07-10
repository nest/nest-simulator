# -*- coding: utf-8 -*-
#
# test_mip_corrdet.py
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
Tests correlation_detector connected to two parrot_neurons
receiving input from a mip_generator. Compares resulting correlation
to known value.
"""


import nest
import numpy.testing as nptest
import pytest


def test_correlation_detector_mip():
    # Cross check generated with cross_check_test_mip_corrdet.py
    expected_hist = [
        2335,
        2317,
        2364,
        2370,
        2376,
        2336,
        2308,
        2325,
        2292,
        2393,
        4806,
        2378,
        2373,
        2356,
        2357,
        2400,
        2420,
        2325,
        2367,
        2338,
        2293,
    ]

    nest.ResetKernel()

    nest.set(rng_seed=12345)

    mg = nest.Create("mip_generator")
    mg.set(rate=100.0, p_copy=0.5)
    cd = nest.Create("correlation_detector")
    cd.set(tau_max=100.0, delta_tau=10.0)

    pn1 = nest.Create("parrot_neuron")
    pn2 = nest.Create("parrot_neuron")

    nest.Connect(mg, pn1)
    nest.Connect(mg, pn2)

    syn_spec = {"weight": 1.0, "receptor_type": 0}
    nest.Connect(pn1, cd, syn_spec=syn_spec)

    syn_spec = {"weight": 1.0, "receptor_type": 1}
    nest.Connect(pn2, cd, syn_spec=syn_spec)

    # One extra time step so that after conversion to deliver-events-first logic
    # final spikes are delivered as in NEST 3 up to NEST 3.5.
    nest.Simulate(100000.0 + nest.resolution)

    hist = cd.get("histogram")
    nptest.assert_array_equal(hist, expected_hist)
