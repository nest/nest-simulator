#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# receiver_script.py
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
Music example receiver script
------------------------------

Please note that MUSIC and the recording backend for Arbor are mutually exclusive
and cannot be enabled at the same time.

"""

import sys
import music
import numpy
from itertools import takewhile, dropwhile

setup = music.Setup()
stoptime = setup.config("stoptime")
timestep = setup.config("timestep")

comm = setup.comm
rank = comm.Get_rank()

pin = setup.publishContInput("in")
data = numpy.array([0.0, 0.0], dtype=numpy.double)
pin.map(data, interpolate=False)

runtime = setup.runtime(timestep)
mintime = timestep
maxtime = stoptime+timestep
start = dropwhile(lambda t: t < mintime, runtime)
times = takewhile(lambda t: t < maxtime, start)
for time in times:
    val = data
    sys.stdout.write(
        "t={}\treceiver {}: received {}\n".
        format(time, rank, val))
