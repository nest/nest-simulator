# -*- coding: utf-8 -*-
#
# test_set_tics.py
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
The base of the integer clock tics of NEST and the number of those tics
per computation time step are specified at configuration time with
options like --with-tics-per-ms='std::ldexp(1.0,14)' --with-tics-per-step='1024'.
However, these values can also be changed at run time. This is of advantage
if the correctness of a script or the validity of a result need to be checked
by running the same simulation at different computation step sizes. While it
is more comfortable in demonstrations and for beginners to operate with tics
to the base of 10, in production it is often better to use base 2 because of
the increased density of data points and exact representation in the double
data type. Therefore, these parameters can also be specified at run time
in the root object prior to the creation of any network element. Here, it is
often more convenient to specify the resolution (the computation time step) in
units of milliseconds instead of the number of tics per step.
This script tests whether the NEST kernel accepts a modification of the
parameters and whether the corresponding conversions are correct.
"""

import nest
import numpy as np
import pytest


@pytest.fixture(autouse=True)
def prepare():
    nest.ResetKernel()
    nest.set(tics_per_ms=2**14, resolution=2 ** (-4))
    nest.set(tics_per_ms=2**14, resolution=2**-4)


def test_correct_ticks_per_ms():
    actual = nest.GetKernelStatus("tics_per_ms")
    assert actual == 2**14


def test_correct_resolution():
    actual = nest.GetKernelStatus("resolution")
    assert actual == 2**-4


def test_correct_tics_per_step():
    actual = nest.GetKernelStatus("tics_per_step")
    assert actual == 2**10


def test_correct_ms_per_tic():
    actual = nest.GetKernelStatus("ms_per_tic")
    assert actual == 2**-14
