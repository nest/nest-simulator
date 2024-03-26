# -*- coding: utf-8 -*-
#
# test_correlospinmatrix_detector.py
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


def test_correlospinmatrix_detector():
    """Checks that the correlospinmatrix_detector computes
    the expected raw correlation matrix.
    """

    nest.ResetKernel()

    # 3-tensor of correlation functions
    expected_corr = np.array(
        [
            [
                [0, 0, 0, 0, 0, 10, 20, 30, 40, 50, 60, 50, 40, 30, 20, 10, 0, 0, 0, 0, 0],
                [0, 10, 20, 30, 40, 50, 50, 40, 30, 20, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
            ],
            [
                [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 20, 30, 40, 50, 50, 40, 30, 20, 10, 0],
                [0, 0, 0, 0, 0, 0, 10, 20, 30, 40, 50, 40, 30, 20, 10, 0, 0, 0, 0, 0, 0],
                [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
            ],
            [
                [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
            ],
        ]
    )

    sg1 = nest.Create("spike_generator")
    sg2 = nest.Create("spike_generator")
    sg3 = nest.Create("spike_generator")

    csd = nest.Create("correlospinmatrix_detector")

    csd.set(N_channels=3, tau_max=10.0, delta_tau=1.0)

    sg1.set(spike_times=[10.0, 10.0, 16.0])  # binary pulse starting at 10. ending at 16.
    sg2.set(spike_times=[15.0, 15.0, 20.0])  # binary pulse starting at 15. ending at 20.

    # one final event needed so that last down transition will be detected
    sg3.set(spike_times=[25.0])

    nest.Connect(sg1, csd, syn_spec={"receptor_type": 0})
    nest.Connect(sg2, csd, syn_spec={"receptor_type": 1})
    nest.Connect(sg3, csd, syn_spec={"receptor_type": 2})

    nest.Simulate(100.0)

    np.testing.assert_equal(np.array(csd.count_covariance), expected_corr)
