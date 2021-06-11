# This script runs NEST and transmits spikes via the Arbor backend.
# Spikes are also stored to file for testing.
# It requires NESTIO + ARBOR-NESTIO.

from sys import argv
argv.append('--quiet')
import sys                  # noqa nopep8 (order of imports matter)

print("Getting comm")
from mpi4py import MPI      # noqa nopep8 (order of imports matter)
comm = MPI.COMM_WORLD.Split(0)  # is nest

print("Getting nest")
import nest                 # noqa nopep8 (order of imports matter)


STATUS_DICT = nest.ll_api.sli_func("statusdict")
if (not STATUS_DICT["have_recordingbackend_arbor"]):
    print("Recording backend Arbor unavailable. Exit testscript!")
    sys.exit(1)

nest.set_communicator(comm)
nest.SetKernelStatus({'recording_backends': {'arbor': {}}})

print("Building network")
pg = nest.Create('poisson_generator', params={'rate': 10.0})

# We cannot directly record from poisson_generator due to implementation
# details. Create a parrot and connect the recorder to that
parrots = nest.Create('parrot_neuron', 100)
nest.Connect(pg, parrots)

sd1 = nest.Create('spike_recorder', params={"record_to": "ascii"})
sd2 = nest.Create('spike_recorder', params={"record_to": "arbor"})
nest.Connect(parrots, sd1 + sd2)

status = nest.GetKernelStatus()
print('min_delay: ', status['min_delay'], ", max_delay: ", status['max_delay'])
print("Simulate")
sys.stdout.flush()

nest.Simulate(100.0)
print("Done")
