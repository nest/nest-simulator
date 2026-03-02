#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# test_cont_proxy_receiver.py
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

mciproxy = nest.Create("music_cont_in_proxy", params={"port_name": "voltage_in"})

results = np.empty(shape=(20, 2), dtype=float)
with nest.RunManager():
    for i in range(20):
        nest.Run(1)
        results[i, :] = mciproxy.data

reference_results = np.array(
    [
        [-57.0719, -60.0000],
        [-57.4215, -55.8623],
        [-57.7953, -57.3695],
        [-58.1948, -58.9805],
        [-58.6218, -60.0000],
        [-59.0783, -60.0000],
        [-59.5663, -55.4294],
        [-60.0879, -56.9067],
        [-60.6455, -58.4859],
        [-61.2415, -60.0000],
        [-61.8786, -60.0000],
        [-62.5596, -55.1193],
        [-63.2876, -56.5752],
        [-64.0658, -58.1315],
        [-64.8976, -59.7951],
        [-65.7867, -61.5734],
        [-66.7371, -63.4743],
        [-67.7531, -65.5062],
        [-68.8391, -67.6783],
        [0.00000, 0.00000],
    ]
)

# reference results are in reverse order (copied from SLI)
np.testing.assert_allclose(reference_results[-1::-1, :], results, atol=1e-4)
