#!/usr/bin/env python3

# This is the real nest program, which requires NESTIO + ARBOR-NESTIO

from sys import argv
argv.append('--quiet')
import sys

print("Getting comm")
from mpi4py import MPI
comm = MPI.COMM_WORLD.Split(0)  # is nest

print("Getting nest")
import nest


STATUS_DICT = nest.ll_api.sli_func("statusdict")
if (not STATUS_DICT["have_recordingbackend_arbor"]):
    print("Recording backend Arbor available. Exit testscript!")
    sys.exit(1)

nest.set_communicator(comm)
nest.SetKernelStatus({'recording_backends': {'arbor': {}}})

print("Building network")
pg = nest.Create('poisson_generator', params={'rate': 10.0})

# We cannot directly record from poisson_generator due to implementation
# details. Create a parrot and connect the recorder to that
parrots = nest.Create('parrot_neuron', 100)
nest.Connect(pg, parrots)

sd2 = nest.Create('spike_recorder', params={"record_to": "arbor"})
nest.Connect(parrots, sd2)

status = nest.GetKernelStatus()
print('min_delay: ', status['min_delay'], ", max_delay: ", status['max_delay'])
print("Simulate")
sys.stdout.flush()

nest.Simulate(100.0)
print("Done")
