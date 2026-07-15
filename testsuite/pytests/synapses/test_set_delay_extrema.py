# -*- coding: utf-8 -*-
#
# test_set_delay_extrema.py
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


@pytest.fixture()
def prepare():
    nest.ResetKernel()


@pytest.fixture()
def prepare_network():
    h = 0.1
    nest.set(resolution=h, min_delay=h, max_delay=2.0)

    n1 = nest.Create("iaf_psc_alpha", params={"I_e": 1450.0})
    n2 = nest.Create("iaf_psc_alpha")

    nest.Connect(n1, n2, syn_spec={"weight": 100.0, "delay": 0.5})  # small delay
    nest.Connect(n2, n1, syn_spec={"weight": 100.0, "delay": 1.0})  # large delay

    nest.Simulate(1.0)


def test_setting_delay(prepare_network):
    min_delay, max_delay = nest.GetKernelStatus(["min_delay", "max_delay"])

    assert min_delay == 0.1
    assert max_delay == 2
