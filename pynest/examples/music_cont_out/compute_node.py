#!/usr/bin/env python
# -*- coding: utf-8 -*-

import music
import numpy as np
from itertools import takewhile, dropwhile

music_setup = music.Setup()

stoptime = music_setup.config("stoptime")

# In PyMusic, the timestep is given in seconds, in NEST however, we are
# dealing with milliseconds
timestep = music_setup.config("timestep")
cont_portwidth = int(music_setup.config("cont_portwidth"))

# MPI related
mpi_comm = music_setup.comm
mpi_rank = mpi_comm.Get_rank()

# event_out = music_setup.publishEventOutput("event_out")
# event_out.map(music.Index.GLOBAL,
#               base=0,
#               size=event_out.width(),
#               maxBuffered=1)

cont_in = music_setup.publishContInput("cont_in")
cont_in_data = np.zeros(cont_portwidth, dtype=np.double)
cont_in.map(cont_in_data, delay=0., interpolate=False)

runtime = music_setup.runtime(timestep)
maxtime = stoptime + timestep

data_view = cont_in_data.view().reshape((20,2))
data_view = cont_in_data.view(dtype=[('V_m', np.double), ('g_ex', np.double)])

# Comment this out for printing
start = dropwhile(lambda t: t < timestep, runtime)
times = takewhile(lambda t: t < maxtime, start)

for time in times:
    print("t={time}\treceiver {receiver}: received {received}\n"
          .format(time=time, receiver=mpi_rank, received=data_view))

