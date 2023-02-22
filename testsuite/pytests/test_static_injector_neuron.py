# -*- coding: utf-8 -*-
#
# test_static_injector_neuron.py
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
from pprint import pprint

nest.ResetKernel()

#nest.set(resolution=0.1, ms_per_tic=0.001, tics_per_ms=1000.0)
nest.set(resolution=0.1, tics_per_ms=1000.0)

sg = nest.Create("spike_generator",
                 params={"spike_times": [1.0, 1.9999, 3.0001]})


pprint(nest.GetStatus(sg))

inj_nrn = nest.Create("static_injector_neuron",
                      params={"spike_times": [1.0, 1.9999, 3.0001]})

pprint(nest.GetStatus(inj_nrn))
