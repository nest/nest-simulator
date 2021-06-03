Stimulation from simulations
============================

The architecture of Nest has been modified to include a backend for
stimulating devices. This modification is inspired by the backend for
recording devices. (:doc:`recording from simulations <recording_simulations>`)

With Nest 3.0, we change the terminology of input device to stimulating device.
Nest 3.0 supports one stimulating backend, `MPI communication`. This backend is
available only when the Nest is compiled with `MPI` support. This is useful in
case of co-simulation and allows a closed loop simulation if it's coupled with the
MPI recording backend.

Changes
^^^^^^^

Add the new parameter `stimulus_source`, for selecting backend in stimulating
device, if it's not the default one.
Add the parameter `label` for the MPI stimulating backend, allow to selection
the file for the MPI connection.

All details about the new infrastructure can be found in the guide on
:doc:`stimulating a network <../../stimulating_the_network.rst>`.

