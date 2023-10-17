# -*- coding: utf-8 -*-
#
# test_poisson_ps_intervals.py
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
import numpy as np
import pytest

"""
Name: testsuite::test_poisson_ps_intervals - checks coefficient of variation

Synopsis: (test_poisson_ps_intervals) run -> CV sufficiently close to 1

Description:
 The inter spike interval (ISI) distribution of a Poisson process is
 exponential. The coefficient of variation defined as the standard deviation
 of the intervals normalized by the mean interval is 1. The test checks whether
 the output of the poisson_generator_ps as recorded by the spike_recorder is
 sufficiently close to 1. Without the property /precise_times the error is larger
 than 0.1 .
Remarks:
 An improved version of this test should check whether the observed deviation from
 unity is within the error bounds given by the number of spikes.
 Even a correct implementation generates with low probability spike trains  which
 do not pass the test. The seed set in the default configuration of NEST avoids
 this problem. Therefore, failure of this script indicates that the configuration
 is not portable.
 The test in r8067 worked even with ticket #157 unresolved. Only the presence of a
 neuron (see test_spike_transmission_ps_iaf) exhibited the problem. Prior to the
 fix of ticket #164 this test produced inconsistent results without specifying
 /local_num_threads 1 for the kernel.

FirstVersion: February 2008
Author: Diesmann, Plesser
"""


def test_poisson_ps_intervals():
    nest.ResetKernel()
    nest.resolution = 0.1

    sr = nest.Create("spike_recorder")
    pg = nest.Create("poisson_generator_ps")
    pg.rate = 12892.25

    nest.Connect(pg, sr)

    nest.Simulate(10000.0)

    times = sr.events["times"]
    isi = np.ediff1d(times)
    cv = isi.std() / isi.mean()
    assert np.abs(1 - cv) < 0.001
