#!/usr/bin/env python

import music

import sys
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
