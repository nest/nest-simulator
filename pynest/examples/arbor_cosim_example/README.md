# Arbor co-simulation example
One directional sending of spikes using the NESTIO backend. Receiver is
assumed to be Arbor. Correctly employing static routing.

## Requirements
+ mpi4Pi
+ NumPy
+ arbor installation

## Files
arbor_proxy.py Fake pure python3 arbor.
nest_sender.py example script sending spikes to the other side 

## Instructions
source the nest environment vars

mpirun -np 2 ./arbor_proxy.py : -np 2 ./nest_sender.py

w.klijn@fz-juelich.de