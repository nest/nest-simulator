# -*- coding: utf-8 -*-
#
# test_poisson_ps_min_interval.py
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
Name: testsuite::test_poisson_ps_min_interval - checks that intervals are independent of tic size

Synopsis: (test_poisson_ps_min_interval) run -> minimum interal close to 0

Description:
 The inter spike interval (ISI) distribution of a Poisson process is
 exponential. The test verifies that the minimum interval is arbitrarily
 small and not constrained by the ms_per_tic property of the simlation kernel.
Remarks:
 Even a correct implementation generates with low probability spike trains where the
 minimal interval is larger than ms_per_tic. The seed set in the default configuration
 of NEST avoids this problem. Therefore, failure of this script indicates that the
 configuration is not portable.
FirstVersion: February 2009
Author: Diesmann
"""


def test_poisson_ps_min_interval():
    nest.ResetKernel()
    nest.resolution = 0.1
    sr = nest.Create("spike_recorder")
    pg = nest.Create("poisson_generator_ps")
    pg.rate = 12892.25

    nest.Connect(pg, sr)

    nest.Simulate(10000.0)

    isi = np.ediff1d(sr.events["times"])

    assert isi.min() < nest.ms_per_tic / 10.0
